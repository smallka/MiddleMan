//---------------------------------------------------------------------------

#ifndef ThreadManagerH
#define ThreadManagerH
//---------------------------------------------------------------------------
#include    <vcl.h>
#include    <Classes.hpp>
#include    <SyncObjs.hpp>
#include    "AList.h"

enum        TWYThreadState
{
    tsSuspend, tsFree, tsBusy, tsIniting, tsUnIniting, tsTerminate, tsError, tsFinished
};

//�̹߳�����
//��� TDefThreadFunc �ķ���ֵΪSleepTime
//�������0, �᲻��ת
//�������-1, ��ر�����߳�

class SingleThread;
typedef int       (__closure     * TDefThreadFunc)(SingleThread * self);
class   WYThreadEvent;

class   WYThread  : public TThread
{
private:
	bool                            m_IsCOMObj;
protected:
	String                          m_Name;
	TWYThreadState                  m_ThreadState;
	bool							m_CreateSuspended;
	WYThreadEvent	*				m_CreateEvent;
    virtual   void __fastcall Execute(){}

    virtual   void                  Initialize(){}
	virtual   void                  UnInitialize(){}

public:
    __fastcall WYThread(bool    CreateSuspened, bool IsCOMObj = false);
	virtual __fastcall ~WYThread();

	TDefThreadFunc      			fpThreadFunc;
	TDefThreadFunc					fpInitFunc;
	TDefThreadFunc					fpUnInitFunc;

    virtual bool                    GetIsCOMObj(){return        m_IsCOMObj;}
    virtual String                  GetName(){return    m_Name;}
    virtual String                  GetStateName();
	virtual TDefThreadFunc          GetThreadFunc(){return  NULL;}
	WYThreadEvent	*               GetCreateEvent(){return m_CreateEvent;}
	TWYThreadState                  GetThreadState(){return	m_ThreadState;}
};

//�����̵߳���
//ʹ����Ҫע�� :  TDefThreadFunc ����ֵΪSleep��ʱ��
//-1��ʾ�˳�,  ������Sleep�� ���޵ȴ�
class   SingleThread : public WYThread
{
private:

protected:
    virtual   void __fastcall Execute();


public:
	__fastcall SingleThread(bool    CreateSuspened, String  name, TDefThreadFunc  func, bool createCOMObj);
    __fastcall SingleThread(bool    CreateSuspened, String  name);
	__fastcall virtual ~SingleThread();



    virtual   void                  Initialize();
    virtual   void                  UnInitialize();
    virtual   TDefThreadFunc        GetThreadFunc(){return      fpThreadFunc;}
};

//Event����
class   WYThreadEvent : public TEvent
{
private:
    bool                m_bEnabled;
public:
    WYThreadEvent(Windows::PSecurityAttributes EventAttributes, bool ManualReset, bool InitialState, const AnsiString Name, bool UseCOMWait);
    virtual TWaitResult __fastcall WaitFor(unsigned Timeout);
    void                Enable();
    void                Disable();
    bool                IsEnabled(){return  m_bEnabled;}
};

////////////////////////////////////////////////////////////////
class   UserThread
{
private:
    WYThread    *       m_Thread;
    String              m_ThreadName;
    TDefThreadFunc      m_ThreadFunc;
    bool                m_IsComObj;
public:
    UserThread();
    ~UserThread();

    void                UserCreate(bool    CreateSuspened, String  name, TDefThreadFunc  func, bool createCOMObj = false);
    WYThread    *       GetThread(){return  m_Thread;}
    void                SetThread(WYThread    *thread){m_Thread = thread;}

};

////////////////////////////////////////////////////////////////

//�̹߳�����
class   ThreadManager
{
private:
	AList<UserThread>        m_Threads;
	AList<WYThreadEvent>     m_Events;
	bool                     m_bThreadStarted;

	UserThread    *   FindThreadContainerByName(String threadName);
	UserThread    *   FindThreadContainerByID(DWORD threadID);
public:
	ThreadManager();
	~ThreadManager();

	UserThread * 	ManagerCreateThread(String  name, TDefThreadFunc  func, bool createCOMObj = false);

	void            AddThread(WYThread  * thread);

	void            StartAll();
	void            StopAll();
	void            FreeAll();

	WYThread    *   At(int index);
	int             Count(){return  m_Threads.Count();}

	WYThreadEvent*  AddEvent(String name);

	WYThread    *   FindThreadByThreadID(int    threadID);
	WYThread    *   FindThreadByName(String threadName);
	void			ProcessDeletedThread();
};

ThreadManager       *       GetThreadManager();

#endif
