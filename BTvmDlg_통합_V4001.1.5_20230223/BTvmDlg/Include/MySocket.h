
#pragma once

#define RET_SNSOCKET_CLOSE		-99

class NHS_AFX_EXPORT CMySocket
{
public:

protected:

public:
	// 소켓 관련
	SOCKET	m_clientSock;
	int		m_client_len;
	struct sockaddr_in m_server_addr;
	
	int		m_nTimeout;
	
public:
	CMySocket();
	~CMySocket();


public:

	int ConnectSocket(char * szIpAddr, int nPort, int tmo);
	int Send(BYTE * szData, int nSize);
	int Recv(BYTE *szData, int nSize);
	int Recv(BYTE *pData, int nLen, int nTmo);
	void CloseSocket(void);
	int Select(int nSecond);
};


