//---------------------------------------------------------------------------

#ifndef DllInjecterH
#define DllInjecterH

#include <vcl.h>

//---------------------------------------------------------------------------

class DllInjecter
{
private:

	TTimer *m_FindTimer;
	String m_DllPath;
	String m_ClassName;
	HWND m_DestWindowHandle;

	void __fastcall OnFindTimer(System::TObject* Sender);
	void ProcessInject();
public:
    DllInjecter();
	~DllInjecter();

	void InjectDll(String dllPath, String className);
	inline void SetDestWindowHandle(HWND handle)
	{
		m_DestWindowHandle = handle;
	}
	inline String GetClassName() const
	{
		return m_ClassName;
	}
};

DllInjecter *GetDllInjecter();

#endif
