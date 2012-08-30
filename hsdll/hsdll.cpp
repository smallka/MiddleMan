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

static HINSTANCE gHInstDLL = 0;
static String gViewerDefineClassName;
static HANDLE gViewerWindowHandle = 0;
static int gViewerPort = 0;

void SendMsg(int par, String msg)
{
	COPYDATASTRUCT copyData;
	copyData.cbData = msg.Length() * sizeof(TCHAR);
	copyData.lpData = msg.c_str();
    copyData.dwData = par;

	HANDLE pWindow = gViewerWindowHandle;
	if (pWindow)
    {
        ::SendMessage(pWindow, WM_COPYDATA, NULL, (LPARAM)&copyData);
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

#define HOOK_SIZE 6

/**
	�Զ���APIHOOK�ṹ
**/
struct  HOOKSTRUCT
{
    FARPROC funcaddr;			// �������ڵ�ַ
	BYTE olddata[HOOK_SIZE];	// ������ͷ��8���ֽ�ԭʼ����
	BYTE newdata[HOOK_SIZE];	// ����Ҫд������ݣ������������棬������EAX�洢Ҫ��ת�ĵ�ַ������FF E0��������Ǿ��Ե�ַ��
};

HOOKSTRUCT gConnectHookData; 
HOOKSTRUCT gIoctlsocketHookData;

/**
	��ԭ���ĺ�������д�ء����ر�HOOK
**/
void HookOffOne(HOOKSTRUCT *hookfunc)
{
    WriteProcessMemory(GetCurrentProcess(), hookfunc->funcaddr, hookfunc->olddata, HOOK_SIZE, 0);
}

/**
	�����ǹ����Ĵ���д�롪����HOOK
**/
void HookOnOne(HOOKSTRUCT *hookfunc)
{          
    WriteProcessMemory(GetCurrentProcess(), hookfunc->funcaddr, hookfunc->newdata, HOOK_SIZE, 0);
}

bool HOOKAPI(DWORD Address,HOOKSTRUCT *hookfunc, DWORD mFncAddress)
{
    hookfunc->funcaddr = (FARPROC)Address;	// ����ط�����һ��ת�����Ѵ���ֵת���ɺνṹ����������һ��
    memcpy(hookfunc->olddata, hookfunc->funcaddr, HOOK_SIZE);	// ��¼RECV����ǰ8�ֽ�ԭʼ����

    // ���濪ʼ����������
    hookfunc->newdata[0] = 0x68;   // PUSH
    memcpy(&hookfunc->newdata[1], (char *)(&mFncAddress), 4);  // �����滻�������ᵽ�� 00000000 Ϊ������������RECV�ĺ����ĵ�ַ
    hookfunc->newdata[5] = 0xC3;   // RET

    HookOnOne(hookfunc);  // ��RECV����ǰ8�ֽ��滻Ϊ���ǹ����Ĵ��롪����ʼHOOK RECV
    return true;
}

WINSOCK_API_LINKAGE int WSAAPI ConnectHook(
	IN SOCKET sock,
	__in_bcount(namelen) const struct sockaddr FAR * name,
	IN int namelen)
{
	int nReturn = 0;

	WORD hport = (BYTE)name->sa_data[0];
    WORD lport = (BYTE)name->sa_data[1];
	hport *= 0x100;
	WORD port = hport + lport;

	String ip = String((BYTE)name->sa_data[2]) \
			+ "." + String((BYTE)name->sa_data[3]) \
			+ "." + String((BYTE)name->sa_data[4]) \
			+ "." + String((BYTE)name->sa_data[5]);

	SendHint("socket(%d) connect %s:%d", sock, ip, port);

	HookOffOne(&gConnectHookData);

	if (port != 80 && ip != "127.0.0.1")
	{
		//SendConnect("tcp|%s|%d|%d", ip, port);
		AnsiString ansiIP = HOST_IP;

		SOCKET sockUDP = socket(AF_INET, SOCK_DGRAM, 0);
		SOCKADDR_IN addrUDP;
		addrUDP.sin_family = AF_INET;
		addrUDP.sin_addr.S_un.S_addr = inet_addr(ansiIP.c_str());
		addrUDP.sin_port = htons(gViewerPort);
		AnsiString addr = ip + "|" + String(port);
		sendto(sockUDP, addr.c_str(), addr.Length(), 0, (SOCKADDR*)&addrUDP, sizeof(SOCKADDR));
		closesocket(sockUDP);

		sockaddr_in * redirect_addr = (sockaddr_in *)name;
		redirect_addr->sin_port = htons(gViewerPort + 1);
		redirect_addr->sin_addr.s_addr=inet_addr(ansiIP.c_str());

		SendHint("redirect to %s", HOST_IP);
	}

	nReturn = connect(sock, name, namelen);
	SendHint("connect return : %d, error : %d", nReturn, WSAGetLastError());

	// ע��: ��Ҫ���������߼�, ��Ҫ��������������..
	HookOnOne(&gConnectHookData);

	return nReturn;
}

WINSOCK_API_LINKAGE int WSAAPI IoctlsocketHook(
	IN SOCKET sock,
	IN long cmd,
	__inout u_long FAR * argp)
{
	HookOffOne(&gIoctlsocketHookData);

	if((DWORD)cmd == FIONBIO)
	{
		SendHint("socket(%d) non-blocking : %d", sock, *argp);
	}

	int nReturn = ioctlsocket(sock, cmd, argp);

	HookOnOne(&gIoctlsocketHookData);

	return(nReturn);
}

/**
	����
**/
void ProcessHook()
{
	static String libStr = "ws2_32.dll";

	HANDLE libHandle = GetModuleHandle(libStr.c_str());

	{
		DWORD connectAddr = (DWORD)GetProcAddress(libHandle, "connect");
		HOOKAPI(connectAddr, &gConnectHookData, (DWORD)ConnectHook);
		SendHint("hook connect, addr = 0x%x", connectAddr);
	}

	{
		DWORD ioctlAddr = (DWORD)GetProcAddress(libHandle, "ioctlsocket");
		HOOKAPI(ioctlAddr, &gIoctlsocketHookData, (DWORD)IoctlsocketHook);
		SendHint("hook ioctlsocket, addr = 0x%x", ioctlAddr);
	}
}

BOOL CALLBACK EnumFindViewerWindowsProc(HWND hwnd, LPARAM lParam)
{
	TCHAR pclassName[100];
	GetClassName(hwnd, pclassName, sizeof(pclassName) / sizeof(TCHAR));
	String className = pclassName;
	if (className == gViewerDefineClassName)
	{
        gViewerWindowHandle = hwnd;
        return false;
	}
    return true;
}

/**
	�ҳ�Viewer����
**/
bool FindViewerWindow()
{
	EnumWindows((WNDENUMPROC)EnumFindViewerWindowsProc, NULL);

	if (gViewerWindowHandle == 0)
	{
		GetLog()->Error("can't find viewer window: %s", gViewerDefineClassName);
		return false;
	}

	GetLog()->Info("viewer window found: %u", gViewerWindowHandle);
	return true;
}

/**
	��ȡ���ã��ҳ�MiddleMan����
**/
bool InitDLL(HINSTANCE hinstDLL)
{
	gHInstDLL = hinstDLL;

	TCHAR pfileName[255];
	GetModuleFileName(gHInstDLL, pfileName, sizeof(pfileName) / sizeof(TCHAR));
	String fileName = pfileName;
	String path = ExtractFilePath(fileName) + "config.ini";
	if(!FileExists(path))
	{
		GetLog()->Error("config missing: %s", path);
		return false;
	}

	TMemIniFile *config = new TMemIniFile(path);
	gViewerDefineClassName = config->ReadString("SET", "ViewerDefineClassName", "");
	gViewerPort = config->ReadString("SET", "ViewerPort", "").ToIntDef(0);

	GetLog()->Info("config loaded: %s", path);

	return true;
}

/**
	�������
**/
int __stdcall DllMain(HINSTANCE hinstDLL, DWORD fwdreason, LPVOID lpvReserved)
{
    switch(fwdreason)
    {
        case DLL_PROCESS_ATTACH:
        {
			if (!InitDLL(hinstDLL))
			{
				return 0;
			}
			if (!FindViewerWindow())
			{
				return 0;
			}
            ProcessHook();
            break;
        }

        case DLL_PROCESS_DETACH:
           break;

    }

    return 1;
}
