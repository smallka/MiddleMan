//---------------------------------------------------------------------------

#ifndef Log4MeH
#define Log4MeH
//---------------------------------------------------------------------------
#include <fstream>
#include <vcl.h>
#include "SingletonEx.h"
using namespace std;

#define     GEN_GET_SET(TYPE, PAR)         \
    TYPE      Get##PAR()                \
    {                                       \
        return  m_##PAR;                       \
    }                                           \
    void        Set##PAR(TYPE par)   \
    {                                \
        m_##PAR = par;               \
    }

#define     GEN_POINTER_GET(PAR)  \
    PAR    *   Get##PAR()  \
    {   \
        return  &m_##PAR;    \
    }
    

#ifdef _DEBUG
#define ASSERT(x)   WY_ASSERT(x)

#define WY_ASSERT(x)        \
if (!(x))						\
{                               \
	String Info;                \
	Info.sprintf(_TEXT("assert fail: (%s). %s:%s"),#x,__LINE__,__FUNC__); \
    GetLog()->Fatal(Info);              \
	throw Exception(Info,123456);       \
}
#else
#define ASSERT(x)   WY_ASSERT(x)
#define WY_ASSERT(x)
#endif

//****************************************************
//֧��Dump���ö�ջ
//��Ŀ�����:  USE_EUREKA_DUMP
//****************************************************

//����: д��־�ļ�
//ʵ�ַ�ʽ:singletonģʽ
//����:lev--���༶��  log--����  logLen--���ݵĳ���
//���߳������������Ŀ��ͷβ����GetLog��Free

enum        TLogLev
{
    llvDebug = 1, llvInfo, llvWarn, llvError, llvFatal
};

typedef Set<TLogLev, llvDebug, llvFatal>  TLogLvState;


#define     SOURCE_INFO         FormatStr("%s : %d", __FILE__, __LINE__)
#define     LOG_ERROR(s)        GetLog()->Error(FormatStr("%s , info : %s", SOURCE_INFO, s))

class Log4Me
{
    DEFINE_SINGLETONEX(Log4Me)
private:
    CRITICAL_SECTION m_csLock;

    HWND                m_HostWindow;
	String	m_Path;
    String  m_FileName;
	String  GetFileName();
	bool m_bEnableLog;
	ofstream fout;
	void				WriteFile(String str);

	void WriteNormal(String log);
	void OpenFile(String fullName);

	void WarnLog(String log);
	void ErrorLog(String log);

    void DebugLog(String log);
	void InfoLog(String log);
    void FatalLog(String log);

    void MessageWarnLog(int channel, String log);

	bool m_bWritenLog;

    TLogLvState m_LogLev;

    Log4Me();
	~Log4Me();
public:
    void SetFileName(String fileName);
    void DebugOutput(String log, ...);

	void SetEnableLog(bool enAbleLog);
	void SetPath(String path);
	bool HaveLoged(){return m_bWritenLog;}

    /*
    ��־��¼����Logger������Ϊ�Ƿֵȼ��ġ����±���ʾ��
    ��Ϊ
    OFF��FATAL��ERROR��WARN��INFO��DEBUG��ALL ����������ļ���
    Log4j����ֻʹ���ĸ��������ȼ��Ӹߵ��ͷֱ���ERROR��WARN��INFO��DEBUG��
    ͨ�������ﶨ��ļ��������Կ��Ƶ�Ӧ�ó�������Ӧ�������־��Ϣ�Ŀ��ء�
    ���������ﶨ����INFO������Ӧ�ó���������DEBUG�������־��Ϣ��������ӡ����
    */
    //DEBUG Levelָ��ϸ������Ϣ�¼��Ե���Ӧ�ó����Ƿǳ��а�����
    void Debug(String log, ...);
    //INFO level���� ֱ�������GUI��
    void Info(String log, ...);
    //WARN level���������Ǳ�ڴ�������Ρ�
    void Warn(String log, ...);
    //ERROR levelָ����Ȼ���������¼�������Ȼ��Ӱ��ϵͳ�ļ������С�
    void Error(String log, ...);
    //FATAL levelָ��ÿ�����صĴ����¼����ᵼ��Ӧ�ó�����˳���
    void Fatal(String log, ...);

    void SetLogLev(TLogLvState  lvState);

    //////////////////////////////////��Windows Message�ķ�ʽд��־
    void                SetGUIWindow(HWND   hwnd);
    //���
    void                MessageWarn(int channel, String log, ...);
};

Log4Me	*	GetLog();

#endif
