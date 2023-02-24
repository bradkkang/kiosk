// 
// 
// dev_scanner_atec.h : 승차권 스캐너 (위텍)
//

#pragma once

#include "MySerial.h"
#include "MyLogFile.h"
#include "WOMRDll.h"
#include "wscanlib_SSTA6.h"

//----------------------------------------------------------------------------------------------------------------------

#define SCANNER_TOP_DIR			0
#define SCANNER_BOTTOM_DIR		1

//----------------------------------------------------------------------------------------------------------------------

/**
 * @details		스캐너 구조체
 */
#pragma pack(1)

#pragma pack()

//----------	------------------------------------------------------------------------------------------------------------

/**
 * @brief		CScannerWTLogFile
 * @details		승차권리더기 로그파일 생성자
 */
class CScannerWTLogFile : public CMyLogFile 
{
public:
	CScannerWTLogFile(void) {};
	virtual ~CScannerWTLogFile(void) {};
};

//======================================================================= 

class CScannerWT
{
public:
	CScannerWT();
	~CScannerWT();

protected :
	BOOL		m_bConnected;
	HANDLE		m_hAccMutex;
	HANDLE		m_hThread;

	CScannerWTLogFile	m_clsLog;

private :
	void LOG_INIT(void);
	int Locking(void);
	int UnLocking(void);

	static DWORD WINAPI RunThread(LPVOID lParam);

public:

	WORD	m_wVersion;
	
	int		m_nScanSpeed;
	int		m_nPaperOutSpeed;
	int		m_nWidth;				
	int		m_nLine;
	int		m_nPixelFormat;
	BOOL	m_bScanHold;
	BOOL	m_bBarcode;				//< 바코드 처리 유무
	BOOL	m_bBranding;			///< 소인처리 유무

	char	m_szBarCodeText[1024];

	int		m_nOnlyOneScan;
	int		m_bScan;
	int		m_nStatus;
	int		m_nDir;
	BYTE	m_byGain0, m_byOffset0, m_byBright0, m_byContrast0;
	BYTE	m_byRlevel, m_byAdcSpeed, m_byThreshold0, m_byThreshold1;

	MY_IMAGE	m_ImageTop;
	MY_IMAGE	m_ImageBottom;

	char	m_szFileName0[256];
	char	m_szFileName1[256];

	int Initialize(void);
	int Enable(void);
	int Disable(void);
	int GetStatus(int *nStatus);
	int GetDeviceStatus(void); // 디바이스 인식 체크
	int Eject(void);
	int Reject(void);
	int TPHPrint(void);

	int OnBarcode(char* filename, int toporbottom);
	int OnScanning(void);

	int StartProcess(int nCommIdx);
	int EndProcess(void);


};
