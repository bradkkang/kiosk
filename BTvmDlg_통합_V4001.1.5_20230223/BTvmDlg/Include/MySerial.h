
#pragma once


class NHS_AFX_EXPORT CMySerial
{
public:
	CMySerial();
	~CMySerial();

public:
	COMMTIMEOUTS	m_TimeoutOrigin;
	HANDLE			m_hCommHandle;
	HANDLE			m_hEventKill;
	OVERLAPPED		m_Ovl;

	char			m_LineBuf[1024];
	int				m_nLeftRecv;
	char			*p;


public:
	int Open(int nPortIdx, int nBaudrate, int nDataBit, int nPariry, int nStopBit, BOOL bRts = FALSE);
	int Close(void);
	int SendData(BYTE *data, int nLen);
	int ReadData(void);
};


