//---------------------------------------------------------------------------

#include <vcl.h>
#include <memory>
#include <map>
#include <vector>
#include <winsock2.h>
#include <IniFiles.hpp>
#include "Defines.h"
#include "Log4Me.h"
#include "StrUtil.h"

#pragma comment(lib,"ws2_32.lib")

using namespace std;

#pragma hdrstop
#pragma argsused

#define HOOK_SIZE 6

static map<SOCKET, int> gNonBlockingSocket;
static int gConnectPort = 0;

//自定义APIHOOK结构
struct  HOOKSTRUCT
{
    FARPROC funcaddr;  //RECV函数所在地址
	BYTE olddata[HOOK_SIZE];      //RECV函数开头的8个字节原始数据
	BYTE newdata[HOOK_SIZE];    //我们要写入的数据，汇编代码在下面，利用了EAX存储要跳转的地址，并且FF E0这个操作是绝对地址的
};

static HINSTANCE gHInstDLL = 0;
static HANDLE gHostWindowHandle = 0;
static String gFindWindowClassName;
static TMemIniFile *gLoadIni;

BOOL CALLBACK EnumFindViewerWindowsProc(HWND hwnd, LPARAM lParam)
{
	TCHAR pclassName[100];
	GetClassName(hwnd, pclassName, sizeof(pclassName) / sizeof(TCHAR));
	String className = pclassName;
	GetLog()->Info("(%s, %s)", className, gFindWindowClassName);
	if(className == gFindWindowClassName)
	{
        gHostWindowHandle = hwnd;
        return false;
	}
    return true;
}

void InitDLL(HINSTANCE hinstDLL)
{
	if(gHInstDLL != 0)
		return;

	GetLog()->Info("InitDLL");
	gHInstDLL = hinstDLL;

	TCHAR pfileName[255];
	GetModuleFileName(gHInstDLL, pfileName, sizeof(pfileName) / sizeof(TCHAR));
	String fileName = pfileName;
	String path = ExtractFilePath(fileName) + "config.ini";
	if(!FileExists(path))
	{
		GetLog()->Error("Config Not Exists?%s", path);
		return;
	}
	gLoadIni = new TMemIniFile(path);

	gFindWindowClassName = gLoadIni->ReadString("SET", "ViewerDefineClassName", "");

	EnumWindows((WNDENUMPROC)EnumFindViewerWindowsProc, NULL);

	if(gHostWindowHandle == 0)
	{
		ShowMessage("Can't Find Host Window?");
		GetLog()->Error("Can't Find Host Window?");
		return;
	}

	gConnectPort = gLoadIni->ReadString("SET", "ViewerPort", "").ToIntDef(0);
	GetLog()->Info("gConnectPort=%d", gConnectPort);
}

void SendMsg(int par, String msg)
{
	COPYDATASTRUCT copyData;
	copyData.cbData = msg.Length() * sizeof(TCHAR);
	copyData.lpData = msg.c_str();
    copyData.dwData = par;

	HANDLE pWindow = gHostWindowHandle;
	if(pWindow)
    {
        ::SendMessage(pWindow,WM_COPYDATA, NULL, (LPARAM)&copyData);
	}
	else
	{
		GetLog()->Info(msg);
	}
}

void SendHint(String msg, ...)
{
	String info;
	va_list vaArgs;
	va_start(vaArgs, msg);
	info.vprintf(msg.c_str(), vaArgs);
	va_end(vaArgs);

	SendMsg(MSG_HINT, L">" + info);
}

void SendConnect(String msg, ...)
{
	String info;
	va_list vaArgs;
	va_start(vaArgs, msg);
	info.vprintf(msg.c_str(), vaArgs);
	va_end(vaArgs);

	SendMsg(MSG_CONNECT, info);
}

HOOKSTRUCT gRecvHookData; //ws2_32.dll HOOK结构
HOOKSTRUCT gConnectHookData; //ws2_32.dll HOOK结构
HOOKSTRUCT gSendHookData; //ws2_32.dll HOOK结构
HOOKSTRUCT gIoctlsocketHookData;

////////////////////////////////////////
//将原来的函数内容写回——关闭HOOK
void HookOffOne(HOOKSTRUCT *hookfunc)
{
    WriteProcessMemory(GetCurrentProcess(), hookfunc->funcaddr, hookfunc->olddata, HOOK_SIZE, 0);
}

//将我们构建的代码写入——打开HOOK
void HookOnOne(HOOKSTRUCT *hookfunc)
{          
    WriteProcessMemory(GetCurrentProcess(), hookfunc->funcaddr, hookfunc->newdata, HOOK_SIZE, 0);
}

bool HOOKAPI(DWORD Address,HOOKSTRUCT *hookfunc,DWORD mFncAddress)
{
    hookfunc->funcaddr = (FARPROC)Address;  //这个地方用了一个转换，把传入值转换成何结构当中声明的一样
    memcpy(hookfunc->olddata, hookfunc->funcaddr, HOOK_SIZE); //记录RECV函数前8字节原始数据


    //下面开始构建汇编代码
    hookfunc->newdata[0]=0x68;   //PUSH
    memcpy(&hookfunc->newdata[1], (char *)(&mFncAddress), 4);  //这里替换了上面提到的 00000000 为我们用来代替RECV的函数的地址
    hookfunc->newdata[5]=0xC3;   //RET

    HookOnOne(hookfunc);  //将RECV函数前8字节替换为我们构建的代码——开始HOOK RECV
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
WINSOCK_API_LINKAGE
		int
		WSAAPI
		ConnectHook(
			IN SOCKET s,
            __in_bcount(namelen) const struct sockaddr FAR * name,
            IN int namelen
			)
{
	int nReturn = 0;
	WORD hport = (BYTE)name->sa_data[0];
    WORD lport = (BYTE)name->sa_data[1];
	hport *= 0x100;
	SendHint(BinToStr((char *)name->sa_data, sizeof(name->sa_data)));
	WORD port = hport + lport;
	String ip = String((BYTE)name->sa_data[2]) \
			+ "." + String((BYTE)name->sa_data[3]) \
			+ "." + String((BYTE)name->sa_data[4]) \
			+ "." + String((BYTE)name->sa_data[5]);

	int nonBlocking = 0;
	if(gNonBlockingSocket.find(s) != gNonBlockingSocket.end())
    {
        nonBlocking = 1;
	}
	SendHint("Connect, IP=%s, Port=%d, nonBlocking=%d", ip, port, nonBlocking);
    HookOffOne(&gConnectHookData);

	if(port == 80 || ip == "127.0.0.1")
	{
		SendHint("no redirect");
		nReturn = connect(s, name, namelen);
		HookOnOne(&gConnectHookData);

		SendHint("no redirect. nReturn=%d, LastError=%d", nReturn, WSAGetLastError());
		return nReturn;
	}

	//redirect IP
	static int index = 0;

	SendConnect("tcp|%s|%d|%d", ip, port, index);
	sockaddr_in * their_addr = (sockaddr_in *)name;
	their_addr->sin_port = htons(gConnectPort);
	AnsiString ansiIP = HOST_IP;
	their_addr->sin_addr.s_addr=inet_addr(ansiIP.c_str());

	nReturn = connect(s, name, namelen);

	//注意: 不要摄入封包流逻辑, 不要在这里主动发包..
	HookOnOne(&gConnectHookData);

	index++;
	return nReturn;
}

WINSOCK_API_LINKAGE
int
WSAAPI
IoctlsocketHook(
	IN SOCKET s,
	IN long cmd,
	__inout u_long FAR * argp
	)
{
	HookOffOne(&gIoctlsocketHookData);

	if((DWORD)cmd == FIONBIO)
	{
		SendHint("socket(%d) non-blocking : %d", s, *argp);
		gNonBlockingSocket[s] = 1;
	}

	int nReturn = ioctlsocket(s, cmd, argp);
	HookOnOne(&gIoctlsocketHookData);
	return(nReturn);
}
//////////////////////////////END///////////////////////////////

void        ProcessHook()
{
	String libStr = "ws2_32.dll";

	HANDLE libHandle = GetModuleHandle(libStr.c_str());

	{
		DWORD connectAddr = (DWORD)GetProcAddress(libHandle, "connect");
		SendHint("Hook Connect, Addr = 0x%x", connectAddr);
		HOOKAPI(connectAddr,&gConnectHookData,(DWORD)ConnectHook);
	}

	{
		DWORD ioctlAddr = (DWORD)GetProcAddress(libHandle, "ioctlsocket");
		SendHint("Hook Ioctl, Addr = 0x%x", ioctlAddr);
		HOOKAPI(ioctlAddr,&gIoctlsocketHookData,(DWORD)IoctlsocketHook);
	}

    SendHint("HiJack OK!");
}

int __stdcall DllMain(HINSTANCE hinstDLL, DWORD fwdreason, LPVOID lpvReserved)
{
    switch(fwdreason)
    {
        case DLL_PROCESS_ATTACH:
        {
            InitDLL(hinstDLL);
			SendHint("ProcessHook");
            ProcessHook();
            break;
        }

        case DLL_PROCESS_DETACH:
           break;

    }

    return 1;
}
//---------------------------------------------------------------------------
