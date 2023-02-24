// 
// 
// dev_scanner_atec.h : 승차권 스캐너 (ATEC) 헤더 파일
//

#pragma once

#include "MySerial.h"
#include "MyLogFile.h"
#include "AtecScanLib.h"

//----------------------------------------------------------------------------------------------------------------------

#define SCANNER_TOP_DIR			0
#define SCANNER_BOTTOM_DIR		1

//----------------------------------------------------------------------------------------------------------------------

/**
 * @details		스캐너 구조체
 */
#pragma pack(1)

#pragma pack()

//----------------------------------------------------------------------------------------------------------------------

/**
 * @brief		CScannerAtecLogFile
 * @details		승차권리더기 로그파일 생성자
 */
class CScannerAtecLogFile : public CMyLogFile 
{
public:
	CScannerAtecLogFile(void) {};
	virtual ~CScannerAtecLogFile(void) {};
};

//======================================================================= 

class CScannerAtec
{
public:
	CScannerAtec();
	~CScannerAtec();

protected :
	BOOL		m_bConnected;
	HANDLE		m_hAccMutex;
	HANDLE		m_hThread;

	CScannerAtecLogFile	m_clsLog;

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

	//MY_IMAGE	m_ImageTop;
	//MY_IMAGE	m_ImageBottom;

	char	m_szFileName0[256];
	char	m_szFileName1[256];

	// atec variable
	BOOL	m_bBbarcode_Ok;
	int		m_nScanline;
	IMAGE_INFO	m_Imagefront;
	IMAGE_INFO	m_ImageBack;




	int Initialize(void);
	int Enable(void);
	int Disable(void);
	int GetStatus(void);
	int Eject(void);
	int Reject(void);

	//int OnBarcode(char* filename, int toporbottom);
	int OnBarcode(char *rawfile, int TopOrBottom, int height);
	int OnScanning(void);

	int StartProcess(int nCommIdx);
	int EndProcess(void);

};
