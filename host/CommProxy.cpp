//---------------------------------------------------------------------------


#pragma hdrstop

#include "Log4Me.h"
#include "StrUtil.h"
#include "CommProxy.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

#define     MAX_BUF_SIZE    5120

static CommProxy *gCommProxy = NULL;

CommProxy *GetCommProxy()
{
	if (gCommProxy == NULL)
    {
		gCommProxy = new CommProxy;
    }
    return gCommProxy;
}

CommProxy::CommProxy()
{
	m_ListenPort = 0;
	m_DestPort = 0;
}

CommProxy::~CommProxy()
{
    WSACleanup();
}

void CommProxy::SetDestAddress(String addr)
{
    auto_ptr<TStringList> splitStr(new TStringList);
    SplitStr(addr, "|", splitStr.get());
    if (splitStr->Count != 2)
	{
		GetLog()->Error("Dest Address Split error");
		return;
	}
	
	m_DestIP = splitStr->Strings[0];
	m_DestPort = splitStr->Strings[1].ToIntDef(0);
}

bool CommProxy::StartListenPort(int listenPort, int redirectPort)
{
	m_ListenPort = listenPort;
	m_RedirectPort = redirectPort;

	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,0),&wsaData);

	m_UDPSocket = socket(AF_INET, SOCK_DGRAM, 0);//�����˿�ʶ���׽���
	//��Ҫ�󶨵Ĳ���
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//ip��ַ
	addr.sin_port = htons(m_ListenPort);//�󶨶˿�
	if (bind(m_UDPSocket, (SOCKADDR*)&addr, sizeof(SOCKADDR)) == SOCKET_ERROR)
	{
		GetLog()->Error("Bind Port Failed:%d. %s", m_ListenPort, SysErrorMessage(WSAGetLastError()));
		return false;
	}

	GetThreadManager()->ManagerCreateThread("RecvUDPThread", RecvUDPThread, false);
	return true;
}

int CommProxy::RecvUDPThread(SingleThread *self)
{
	SOCKADDR_IN clientSocket;
	int len = sizeof(SOCKADDR);
	char recvBuf[MAX_BUF_SIZE] = {'\0'};
	int recvLen = 0;

	recvLen = recvfrom(m_UDPSocket, recvBuf, sizeof(recvBuf), 0, (SOCKADDR*)&clientSocket,&len);
	if (recvLen <= 0)
	{
		GetLog()->Warn("Recv UDP Thread socket error. msg = %s", SysErrorMessage(GetLastError()));

		// TODO: Close();
		return -1;
	}

	GetLog()->Info("Recv UDP Thread. recv %d", recvLen);

	String addr(recvBuf, recvLen);
	GetLog()->Info("UDP Set Dest Address: %s", addr);
	SetDestAddress(addr);

	if (!StartRedirectPort(m_RedirectPort))
	{
        return -1;
	}
	
	return 0;
}

bool CommProxy::StartRedirectPort(int redirectPort)
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,0),&wsaData);

	m_ListenSocket = socket(AF_INET, SOCK_STREAM, 0);//�����˿�ʶ���׽���
    //��Ҫ�󶨵Ĳ���
    SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//ip��ַ
	addr.sin_port = htons(redirectPort);//�󶨶˿�
	if (bind(m_ListenSocket, (SOCKADDR*)&addr, sizeof(SOCKADDR)) == SOCKET_ERROR)
	{
		GetLog()->Error("Bind Port Failed:%d. %s", redirectPort, SysErrorMessage(WSAGetLastError()));
		return false;
	}
	// TODO: listen count
	if (listen(m_ListenSocket, 100) == SOCKET_ERROR)
	{
		GetLog()->Error("List Port Failed:%d. %s", redirectPort, SysErrorMessage(WSAGetLastError()));
		return false;
	}

	GetThreadManager()->ManagerCreateThread("ListenThread", ListenThread, false);
	return true;
}

int CommProxy::ListenThread(SingleThread *self)
{
	SOCKADDR_IN clientAddr;
	int len = sizeof(SOCKADDR);

	m_ClientSocket = accept(m_ListenSocket, (SOCKADDR*)&clientAddr, &len);
	if (m_ClientSocket == (SOCKET)SOCKET_ERROR)
	{
		return -1;
	}

	m_HostSocket = socket(AF_INET,SOCK_STREAM,0);

    sockaddr_in their_addr; /* connector's address information */
	their_addr.sin_family = AF_INET; /* host byte order */
	their_addr.sin_port = htons(m_DestPort); /* Զ�������˿� */
	AnsiString ansiIP = m_DestIP;
	their_addr.sin_addr.s_addr=inet_addr(ansiIP.c_str()); /* Զ������ip��ַ */

	/* ����connect������������������� */
	GetLog()->Info("CommProxy Connect. ip=%s, port=%d", m_DestIP, m_DestPort);
	if (connect(m_HostSocket, (const sockaddr *)&their_addr, sizeof(their_addr)) == -1)
	{
		GetLog()->Error("CommProxy Connect Error");
        return -1;
	}

	GetLog()->Info("Starting Recv Threads");
	UserThread *curThread = GetThreadManager()->ManagerCreateThread("ClientRecvThread", ClientRecvThread, false);
	curThread = GetThreadManager()->ManagerCreateThread("HostRecvThread", HostRecvThread, false);

	GetLog()->Info("Finish Recv Threads");

	return 0;
}

int CommProxy::ClientRecvThread(SingleThread *self)
{
	char recvBuf[MAX_BUF_SIZE] = {'\0'};
	int recvLen = 0;

	recvLen = recv(m_ClientSocket, recvBuf, sizeof(recvBuf), 0);
	if (recvLen <= 0)
	{
		GetLog()->Warn("Client Recv Thread socket error. msg = %s", SysErrorMessage(GetLastError()));

		// TODO: Close();
		return -1;
	}

	GetLog()->Info("Client Recv Thread. recv %d", recvLen);

	if (!SendBuf_O(m_HostSocket, recvBuf, recvLen))
    {
        GetLog()->Warn("Client Recv Thread socket send error");
		// TODO: Close();
        return -1;
	}
	return 0;
}

int CommProxy::HostRecvThread(SingleThread *self)
{
	char recvBuf[MAX_BUF_SIZE] = {'\0'};
	int recvLen = 0;

	recvLen = recv(m_HostSocket, recvBuf, sizeof(recvBuf), 0);
	if (recvLen <= 0)
	{
		GetLog()->Warn("Host Recv Thread socket error. msg = %s", SysErrorMessage(GetLastError()));

		// TODO: Close();
		return -1;
	}

	GetLog()->Info("Host Recv Thread. recv %d", recvLen);

	if (!SendBuf_O(m_ClientSocket, recvBuf, recvLen))
    {
		GetLog()->Warn("Host Recv Thread socket send error");
		// TODO: Close();
        return -1;
	}
	return 0;
}

bool CommProxy::SendBuf_O(SOCKET s, char *buf, int len)
{
    int result = 0;
    while (len != 0)
    {
        result = send(s, buf, len, 0);
		if (result == SOCKET_ERROR)
		{
			return false;
        }

        len = len - result;
        buf = buf + result;

		if (len == 0)
		{
			return true;
		}
	}

	return true;
}

void CommProxy::Close()
{
	if (m_UDPSocket)
	{
		closesocket(m_UDPSocket);
	}
	if (m_ListenSocket)
	{
		closesocket(m_ListenSocket);
	}
	if (m_ClientSocket)
	{
		closesocket(m_ClientSocket);
	}
	if (m_HostSocket)
	{
		closesocket(m_HostSocket);
	}
}
