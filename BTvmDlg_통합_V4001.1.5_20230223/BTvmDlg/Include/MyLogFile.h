//   file   : LogFile.cpp
//	 Author : nhso
//	 history
//	 2018-06-04		first create
//

#pragma once

class NHS_AFX_EXPORT CMyLogFile
{
public:
	CMyLogFile(void);
	virtual ~CMyLogFile(void);

public:
	int				m_bInit;
	int				m_nKind;
	int				m_nBackupDays;
	SYSTEMTIME		m_stToday;
	char			m_szDirectory[256];
	char			m_szBuf[1024*4];
	CRITICAL_SECTION cs_LogWrite;
	char			m_szBuffer[1024*30];
	HANDLE			m_hFile;

public:
	int SetData(int nBackupDay, char *pDirPath);
	int Initialize(void);
	int Delete(void);
	int LogOut(const char *Format, ...);
	
	int LogOpen(void);
	int LogOpen(BOOL bCreate);
	int LogWrite(const char *Format, ... );
	int LogClose(void);
	
	int LogOutNew(const int iType, const char *Format, ...);
	int HexaDump(char *pTitle, unsigned char *data, int nLen);
};

