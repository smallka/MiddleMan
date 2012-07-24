//---------------------------------------------------------------------------

#ifndef MiddleManH
#define MiddleManH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>

#include <IniFiles.hpp>

//---------------------------------------------------------------------------
class TMiddleManFrm : public TForm
{
__published:	// IDE-managed Components
	TMemo *msgMemo;
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall FormDestroy(TObject *Sender);
private:	// User declarations
	void __fastcall WndProc(TMessage &Msg);
	void ShowMsg(const String &Msg);
public:		// User declarations
	__fastcall TMiddleManFrm(TComponent* Owner);

private:
	TMemIniFile *m_MemIniFile;
};
//---------------------------------------------------------------------------
extern PACKAGE TMiddleManFrm *MiddleManFrm;
//---------------------------------------------------------------------------
#endif
