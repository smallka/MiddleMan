//---------------------------------------------------------------------------

#ifndef CommProxyH
#define CommProxyH

#include <vcl.h>
#include <WINSOCK2.H>
#include "ThreadManager.h"

//---------------------------------------------------------------------------

class CommProxy
{
private:
	int m_DestPort;
	String m_DestIP;
	int m_ListenPort;
	SOCKET m_ListenSocket;
	SOCKET m_ClientSocket;
	SOCKET m_HostSocket;

	bool SendBuf_O(SOCKET s, char *buf, int len);
public:
	CommProxy();
	~CommProxy();

	void SetDestAddress(String addr);
	bool StartListenPort(int listenPort);
	int ListenThread(SingleThread *self);
	int ClientRecvThread(SingleThread *self);
	int HostRecvThread(SingleThread *self);
	void Close();
};

CommProxy *GetCommProxy();

#endif
