
// BTvmDlgDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "BTvmDlg.h"
#include "BTvmDlgDlg.h"
#include "afxdialogex.h"

#include "File_Env_ini.h"
//#include "File_Option_ini.h"
#include "event_if.h"
#include "MyUtil.h"

#include "data_main.h"
#include "damo_ktc.h"

#include "oper_config.h"
#include "oper_kos_file.h"

#include "dev_bill_main.h"
#include "dev_cardreader_main.h"
#include "dev_coin_main.h"
#include "dev_dispenser_main.h"
#include "dev_prt_main.h"
#include "dev_prt_ticket_main.h"
#include "dev_scanner_main.h"
#include "dev_ui_main.h"
#include "dev_tr_main.h"
#include "svr_main.h"

#include "oper_kos_file.h"

#include "md5.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CBTvmDlgDlg 대화 상자




CBTvmDlgDlg::CBTvmDlgDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CBTvmDlgDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CBTvmDlgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_CmbCommand);
	DDX_Control(pDX, IDC_COMBO2, m_CmbTrState);
}

BEGIN_MESSAGE_MAP(CBTvmDlgDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON2, &CBTvmDlgDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON1, &CBTvmDlgDlg::OnBnClickedButton1)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CBTvmDlgDlg 메시지 처리기

BOOL CBTvmDlgDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다. 응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	{
		int i;

		ShowWindow(SW_SHOWMINIMIZED);

		// 명령
		{
// 			char *pCmdStr[] = {
// 				"TR_MRS_MAIN_STATE"			,	// 예매발권 시작
// 				"TR_MRS_CARD_READ_STATE"	,	// 예매발권 - 신용카드 읽기
// 				"TR_MRS_INPUT_STATE"		,	// 예매발권 - 예매번호, 핸드폰번호입력
// 				"TR_MRS_LIST_STATE"			,	// 예매발권 - List 내역
// 				"TR_MRS_DETAIL_STATE"		,	// 예매발권 - 상세내역
// 				"TR_MRS_ISSUE_STATE"		,	// 예매발권 - 발권
// 				"TR_MRS_COMPLETE_STATE"		,	// 예매발권 - 완료
// 				"TR_MRS_CANCEL_STATE"		,	// 예매발권 - 취소
// 			};
// 
// 			for(i = 0; i < sizeof(pCmdStr) / sizeof(char *); i++)
// 			{
// 				m_CmbCommand.AddString(pCmdStr[i]);
// 			}
// 			m_CmbCommand.SetCurSel(0);
		}

		/// test code
		if(0)
		{
			BYTE cmdStr[] = "\x91\x06\x07\x08";
			DWORD  dwCmd;

			dwCmd = *(DWORD *)cmdStr;

			TRACE("1. dwCmd = (0x%08lX), (%s) ...\n", dwCmd, cmdStr);
			TRACE("2. dwCmd = (0x%08lX), (%lu) ...\n", dwCmd, dwCmd);
		}

		// tr state
		if(0)
		{
			char *pTrStr[] = {
				"TR_MRS_MAIN_STATE"			,	// 예매발권 시작
				"TR_MRS_CARD_READ_STATE"	,	// 예매발권 - 신용카드 읽기
				"TR_MRS_INPUT_STATE"		,	// 예매발권 - 예매번호, 핸드폰번호입력
				"TR_MRS_LIST_STATE"			,	// 예매발권 - List 내역
				"TR_MRS_DETAIL_STATE"		,	// 예매발권 - 상세내역
				"TR_MRS_ISSUE_STATE"		,	// 예매발권 - 발권
				"TR_MRS_COMPLETE_STATE"		,	// 예매발권 - 완료
				"TR_MRS_CANCEL_STATE"		,	// 예매발권 - 취소
				"TR_CANCRY_MAIN_STATE"		,	// 환불 - 시작
				"TR_CANCRY_TCK_READ"		,	// 환불 - 승차권 읽기
				"TR_CANCRY_FARE_STATE"		,	// 환불 - 카드 또는 현금 환불 처리
				"TR_CANCRY_COMPLETE_STATE"	,	// 환불 - 완료
				"TR_CANCRY_CANCEL_STATE"	,	// 환불 - 취소

			};

			for(i = 0; i < sizeof(pTrStr) / sizeof(char *); i++)
			{
				m_CmbTrState.AddString(pTrStr[i]);
			}
			m_CmbTrState.SetCurSel(0);
		}

		if(0)
		{
			char *pTrStr[] = {
				"승차권 보급 (2000매)"		,	
				"동전 보급 (500개,500개)"		,	
				"지폐 보급 (200개,200개)"		,	
				"회계 내역 조회", 
			};

			for(i = 0; i < sizeof(pTrStr) / sizeof(char *); i++)
			{
				m_CmbTrState.AddString(pTrStr[i]);
			}
			m_CmbTrState.SetCurSel(0);
		}

		if(0)
		{
			char Buffer[100];

			OperInitFile();
			Oper_GetTmExpRefundCode(95, Buffer);
		}

		//ShowCursor(FALSE);

		ProcMain();

		//SetTimer(ID_MY_TIMER, 100, NULL);
	}

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CBTvmDlgDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다. 문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CBTvmDlgDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CBTvmDlgDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CBTvmDlgDlg::OnTimer(UINT nIDEvent) 
{
	//CardReader_Polling();	
}

int CBTvmDlgDlg::ProcMain(void) 
{
	int nRet;
	WSADATA	wsaData;
	char szBasePath[256];
	char szFullName[256];

	::ZeroMemory(szBasePath, sizeof(szBasePath));
	Util_GetModulePath(szBasePath);

	if(WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
	{
		return -1;
	}

	//GetModuleFileName(NULL, szPath, MAX_PATH);

	// UI 프로그램 실행
	if(1)
	{
		DWORD dwTick;
		CString		strUiName("TicketMachine.exe");

		/// 프로그램 종료
		Util_CloseProcess(strUiName);
		dwTick = GetTickCount();
		while( Util_CheckExpire(dwTick) < (1000 * 2) );

		/// 프로그램 실행
		sprintf(szFullName, "%s\\TicketMachine.exe", szBasePath);
		::ShellExecute(NULL, "open", szFullName, NULL, NULL, SW_SHOW);
	}

	Transact_LogInitialize();

	TR_LOG_OUT("################### 시작, Version(%s) ################### ", MAIN_VERSION);

	{
		InitEventCode();
		Init_EnvIniFile();
		
		TR_LOG_OUT("InitDataMng()_1 ");
		InitDataMng();
		TR_LOG_OUT("InitDataMng()_2 ");
	}

	/// load D'amo Library
	{
		InitDamoCrypto();
	}

	///< Create Device Communication
	{
		///< 01. UI 통신
#ifdef __DEV_TR_SPLIT__
		switch(GetConfigPayment())
		{
		case PAY_ONLY_CARD :
		case PAY_ONLY_RF :
		case PAY_CARD_RF :
			{
				///< UI 통신
				nRet = UI_Initialize();
				TR_LOG_OUT("UI_Initialize() nRet(%d) !!", nRet);
				///< 승차권 프린터
				nRet = TckPrt_Initialize();
				TR_LOG_OUT("TckPrt_Initialize() nRet(%d) !!", nRet);
				///< 영수증 프린터
				nRet = Printer_Initialize();
				TR_LOG_OUT("Printer_Initialize() nRet(%d) !!", nRet);
				///< 비지니스
				nRet = Transact_Initialize();
				TR_LOG_OUT("Transact_Initialize() nRet(%d) !!", nRet);
				///< tmax server 
				nRet = Svr_Initialize();

			}
			break;
		case PAY_ONLY_CASH :
		case PAY_CARD_CASH :
			{
				///< UI 통신
				nRet = UI_Initialize();
				TR_LOG_OUT("UI_Initialize() nRet(%d) !!", nRet);
				///< 승차권 리더기
				nRet = Scanner_Initialize();
				TR_LOG_OUT("Scanner_Initialize() nRet(%d) !!", nRet);
				///< 지폐방출기
				nRet = CDU_Initialize();
				TR_LOG_OUT("CDU_Initialize() nRet(%d) !!", nRet);
				///< 동전방출기
				nRet = Coin_Initialize();
				TR_LOG_OUT("Coin_Initialize(() nRet(%d) !!", nRet);
				///< 지폐 입금기
				nRet = Bill_Initialize();
				TR_LOG_OUT("Bill_Initialize() nRet(%d) !!", nRet);
				///< 승차권 프린터
				nRet = TckPrt_Initialize();
				TR_LOG_OUT("TckPrt_Initialize() nRet(%d) !!", nRet);
				///< 영수증 프린터
				nRet = Printer_Initialize();
				TR_LOG_OUT("Printer_Initialize() nRet(%d) !!", nRet);
				///< 비지니스
				nRet = Transact_Initialize();
				TR_LOG_OUT("Transact_Initialize() nRet(%d) !!", nRet);
				///< tmax server 
				nRet = Svr_Initialize();
			}
			break;
		}
#else
		///< UI 통신
		nRet = UI_Initialize();
		TR_LOG_OUT("UI_Initialize() nRet(%d) !!", nRet);
		///< 승차권 리더기
		nRet = Scanner_Initialize();
		TR_LOG_OUT("Scanner_Initialize() nRet(%d) !!", nRet);
		///< 지폐방출기
		nRet = CDU_Initialize();
		TR_LOG_OUT("CDU_Initialize() nRet(%d) !!", nRet);
		///< 동전방출기
		nRet = Coin_Initialize();
		TR_LOG_OUT("Coin_Initialize(() nRet(%d) !!", nRet);
		///< 지폐 입금기
		nRet = Bill_Initialize();
		TR_LOG_OUT("Bill_Initialize() nRet(%d) !!", nRet);
		///< 승차권 프린터
		nRet = TckPrt_Initialize();
		TR_LOG_OUT("TckPrt_Initialize() nRet(%d) !!", nRet);
		///< 영수증 프린터
		nRet = Printer_Initialize();
		TR_LOG_OUT("Printer_Initialize() nRet(%d) !!", nRet);
		///< 비지니스
		nRet = Transact_Initialize();
		TR_LOG_OUT("Transact_Initialize() nRet(%d) !!", nRet);
		///< tmax server 
		nRet = Svr_Initialize();
#endif
	}
	return 0;
}


void CBTvmDlgDlg::OnBnClickedButton2()
{
	// TODO: "명령어 콤보박스"
}


void CBTvmDlgDlg::OnBnClickedButton1()
{
	// TODO: "trans 콤보박스"
	/***
	int arrState[] = {
		TR_MRS_MAIN_STATE		,	// 예매발권 시작
		TR_MRS_CARD_READ_STATE	,	// 예매발권 - 신용카드 읽기
		TR_MRS_INPUT_STATE		,	// 예매발권 - 예매번호, 핸드폰번호입력
		TR_MRS_LIST_STATE		,	// 예매발권 - List 내역
		TR_MRS_DETAIL_STATE		,	// 예매발권 - 상세내역
		TR_MRS_ISSUE_STATE		,	// 예매발권 - 발권
		TR_MRS_COMPLETE_STATE	,	// 예매발권 - 완료
		TR_MRS_CANCEL_STATE		,	// 예매발권 - 취소
		TR_CANCRY_MAIN_STATE	,	// 환불 - 시작
		TR_CANCRY_TCK_READ		,	// 환불 - 승차권 읽기
		TR_CANCRY_FARE_STATE	,	// 환불 - 카드 또는 현금 환불 처리
		TR_CANCRY_COMPLETE_STATE,	// 환불 - 완료
		TR_CANCRY_CANCEL_STATE	,	// 환불 - 취소
	};

	int nSelect = m_CmbTrState.GetCurSel();

	Transact_SetState(arrState[nSelect], 0);
	***/

#if 0
	int nSelect = m_CmbTrState.GetCurSel();

	switch(nSelect)
	{
	case 0 :
		{
			int nTicket = 2000;
			AddAccumTicketData(ACC_TICKET_INSERT, nTicket);
		}
		break;
	case 1 :
		{
			int n100 = 500, n500 = 500;
			
			AddAccumCoinData(ACC_COIN_SUPPLY, n100, n500);
		}
		break;
	case 2 :
		{
			int n1k = 200, n5k = 0, n10k = 200, n50k = 0;

			AddAccumBillData(ACC_BILL_SUPPLY, n1k, n5k, n10k, n50k);
		}
		break;
	case 3 :
		{
			PrintAccumData(TRUE);
		}
		break;
	}
#endif
}


void CBTvmDlgDlg::OnClose()
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	TR_LOG_OUT("%s", "STEP #1 !!");

	if(1)
	{
		CString		strUiName("TicketMachine.exe");
		Util_CloseProcess(strUiName);
		
		DWORD dwTick = ::GetTickCount();
		while( Util_CheckExpire(dwTick) < (1000 * 1) );
	}

	//TR_LOG_OUT("%s", "STEP #2 !!");
	if(1)
	{
		int nRet;

		nRet = Bill_Terminate();
		//nRet = CardReader_Initialize();
		nRet = Coin_Terminate();
		nRet = CDU_Terminate();
		nRet = Printer_Terminate();
		nRet = TckPrt_Terminate();
		nRet = Scanner_Terminate();
	}

	//TR_LOG_OUT("%s", "STEP #3 !!");

	if(1)
	{
		TermDataMng();

// 		TermCashTrData();
// 		TermCashCloseData();
// 		TermSmsTRData();
// 		TermAccumData();
	}

	CDialogEx::OnClose();
}
