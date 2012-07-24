//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <Psapi.h>
#include "Log4Me.h"
#include "DllInjecter.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

static DllInjecter *gDllInjecter = NULL;

DllInjecter *GetDllInjecter()
{
    if(gDllInjecter == NULL)
    {
        gDllInjecter = new DllInjecter;
    }
    return gDllInjecter;
}

DllInjecter::DllInjecter()
{
    m_FindTimer = new TTimer(NULL);
	m_FindTimer->OnTimer = OnFindTimer;
	m_FindTimer->Interval = 1000;
	m_DestWindowHandle = NULL;
}

DllInjecter::~DllInjecter()
{
	delete m_FindTimer;
}

void __fastcall DllInjecter::OnFindTimer(System::TObject* Sender)
{
	ProcessInject();
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, DllInjecter *injecter)
{
	TCHAR className[30] = {'\0'};
	GetClassName(hwnd, className, sizeof(className) / sizeof(TCHAR));
	String clsName = className;
	if(clsName == injecter->GetClassName())
	{
		injecter->SetDestWindowHandle(hwnd);
		return false;
	}
	return true;
}

void EnableDebugPriv(String name)
{
	HANDLE hToken;
	TOKEN_PRIVILEGES tp;
	LUID luid;
	//�򿪽������ƻ�
	OpenProcessToken(
		GetCurrentProcess(),
		TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,
		&hToken);
	//��ý��̱���ΨһID
	LookupPrivilegeValue(NULL, name.c_str(), &luid) ;

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	tp.Privileges[0].Luid = luid;
	//����Ȩ��
	AdjustTokenPrivileges(hToken, 0, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
}

void DllInjecter::ProcessInject()
{
	EnumWindows((WNDENUMPROC)EnumWindowsProc, (LPARAM)this);
	if(m_DestWindowHandle == NULL)
    {
        return;
    }

	DWORD processID;
	GetWindowThreadProcessId(m_DestWindowHandle, &processID);
	if(processID == 0)
	{
		return;
	}

	GetLog()->Info("find the processs: %d", processID);
	BOOL result;

	EnableDebugPriv(SE_DEBUG_NAME);

	//��Զ���߳�
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
	if(!hProcess)
	{
		GetLog()->Error("OpenProcess Failed. %s", SysErrorMessage(GetLastError()));
		return;
	}

	char *pszLibFileRemote;

	//ʹ��VirtualAllocEx������Զ�̽��̵��ڴ��ַ�ռ����DLL�ļ����ռ�
	AnsiString pathName = m_DllPath;
	GetLog()->Info("Inject Dll Path = %s", m_DllPath.c_str());
	pszLibFileRemote = (char *)VirtualAllocEx(
		hProcess,
		NULL,
		pathName.Length(),
		MEM_COMMIT,
		PAGE_READWRITE);

	if(!pszLibFileRemote)
	{
		GetLog()->Info("VirtualAllocEx Failed. %s", SysErrorMessage(GetLastError()));
		return;
	}

	//ʹ��WriteProcessMemory������DLL��·����д�뵽Զ�̽��̵��ڴ�ռ�
	result = WriteProcessMemory(
		hProcess,
		pszLibFileRemote,
		(void *)pathName.c_str(),
		pathName.Length(),
		NULL);
	if(!result)
	{
		GetLog()->Info("WriteProcessMemory Failed. %s", SysErrorMessage(GetLastError()));
		return;
	}

	//����LoadLibraryA����ڵ�ַ
    PTHREAD_START_ROUTINE pfnStartAddr = (PTHREAD_START_ROUTINE)
		GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "LoadLibraryA");
	if(!pfnStartAddr)
	{
		GetLog()->Info("Load Library Failed!. %s", SysErrorMessage(GetLastError()));
		return;
	}

    //����Զ���߳�LoadLibraryA��ͨ��Զ���̵߳��ô����µ��߳�
	HANDLE hRemoteThread = CreateRemoteThread(
		hProcess,
		NULL,
		0,
		pfnStartAddr,
		pszLibFileRemote,
		0,
		NULL);
	if(hRemoteThread != NULL)
    {
		GetLog()->Info("inject OK!");
	}
	else
	{
        GetLog()->Info("inject Fail! %s", SysErrorMessage(GetLastError()));
    }

	// �ͷž��
	CloseHandle(hProcess);
	CloseHandle(hRemoteThread);

    m_FindTimer->Enabled = false;
}

void DllInjecter::InjectDll(String dllName, String className)
{
	m_DllPath = dllName;
	m_ClassName = className;
	m_FindTimer->Enabled = true;
}

