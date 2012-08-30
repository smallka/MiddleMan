//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Defines.h"
#include "Log4Me.h"
#include "DllInjecter.h"
#include "CommProxy.h"
#include "MiddleMan.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMiddleManFrm *MiddleManFrm;

#define MAX_DEBUG_LINE_COUNT		100000

//---------------------------------------------------------------------------
__fastcall TMiddleManFrm::TMiddleManFrm(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------

void __fastcall TMiddleManFrm::WndProc(TMessage &Msg)
{
	if (Msg.Msg == WM_COPYDATA)
	{
        COPYDATASTRUCT * sendData = (COPYDATASTRUCT *)Msg.LParam;
		String msg = String((TCHAR *)sendData->lpData, sendData->cbData / sizeof(TCHAR));

		this->ShowMsg(msg);

		if(sendData->dwData == MSG_CONNECT)
		{
			GetLog()->Info("MSG_CONNECT Set Dest Address: %s", msg);
			GetCommProxy()->SetDestAddress(msg);
		}
	}

    TForm::WndProc(Msg);
}

void TMiddleManFrm::ShowMsg(const String &Msg)
{
	if(msgMemo->Lines->Count >= MAX_DEBUG_LINE_COUNT)
	{
		msgMemo->Clear();
	}

	msgMemo->Lines->Add(Msg);
}

void __fastcall TMiddleManFrm::FormCreate(TObject *Sender)
{
	String filePath = ExtractFilePath(Application->ExeName) + "Log\\";
	GetLog()->SetEnableLog(true);
	GetLog()->SetPath(filePath);
	GetLog()->SetFileName(filePath + "MiddleMan.log");
	GetLog()->Info("Log Init");

	GetLog()->SetGUIWindow(this->Handle);

	String iniName = ExtractFilePath(Application->ExeName)+"config.ini";
	if(!FileExists(iniName))
	{
		ShowMessage("config.ini missing!");
		return;
	}
	m_MemIniFile = new TMemIniFile(iniName);

	String dllName = ExtractFilePath(Application->ExeName) + "wowgo.dll";
	String userDefineClassName = m_MemIniFile->ReadString("SET", "UserDefineClassName", "");
	GetDllInjecter()->InjectDll(dllName, userDefineClassName);

	String remoteIP = m_MemIniFile->ReadString("SET", "RemoteIP", "");
	int redirectPort = m_MemIniFile->ReadString("SET", "RedirectPort", "").ToIntDef(0);

	if (remoteIP == "")
	{
		int listenPort = m_MemIniFile->ReadString("SET", "ViewerPort", "").ToIntDef(0);
		if (listenPort != 0)
		{
			if (!GetCommProxy()->StartListenPort(listenPort, redirectPort))
			{
				ShowMessage("fail to start listen port!");
				return;
			}
		}
		else
		{
			if (!GetCommProxy()->StartRedirectPort(redirectPort))
			{
				ShowMessage("fail to start redirect port!");
				return;
			}
		}

		GetThreadManager()->StartAll();
	}

	// TODO: GetThreadManager()->FreeAll(); on FormDestroy
}
//---------------------------------------------------------------------------


void __fastcall TMiddleManFrm::FormDestroy(TObject *Sender)
{
	  GetLog()->Warn("TMiddleManFrm::FormDestroy");
	  GetLog()->SetEnableLog(false);

	  GetCommProxy()->Close();

	  GetThreadManager()->FreeAll();
}
//---------------------------------------------------------------------------


