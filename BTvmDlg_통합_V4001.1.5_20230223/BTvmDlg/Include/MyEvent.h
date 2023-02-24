// MyEvent.h : 이벤트 처리 헤더 파일
// 
// 
//

#pragma once

//----------------------------------------------------------------------------------------------------------------------

#define EVT_ERROR				0x01		
#define EVT_RECOVER				0x02		
#define EVT_WARN				0x04		
#define EVT_FLAG				0x08		
#define EVT_LOG     			0x10		
#define EVT_LOG_ERR				0x20		
#define EVT_DATA     			0x40		
#define EVT_AUTO_RESET			0x80		
#define EVT_EVENT				0x100
#define EVT_ALARM				0x200
#define EVT_AUTO_RST			0x400
#define EVT_VAR					0x800		

#define FLAG_COIN				0x0001		///> 동전방출기
#define FLAG_BILL				0x0002		///> 지폐입금기
#define FLAG_CDU				0x0003		///> 지폐방출기
#define FLAG_TCK_SCANNER		0x0004		///> 승차권 리더기
#define FLAG_TCK_PRT			0x0005		///> 승차권 프린터
#define FLAG_PRT				0x0006		///> 영수증 프린터
#define FLAG_ICCARD				0x0007		///> IC CARD 리더기
#define FLAG_TMAX				0x0008		///> TMAX 
#define FLAG_OP					0x0009		///> Operation
#define FLAG_RF					0x0010		///> RF
#define FLAG_SMS				0x0011		///> 관제서버

#define FLAG_UI					0x1000		///> UI Module
#define FLAG_ALL				0x8000		///> All

#pragma pack(1)

//---------------------------------------------
// event 관련 구조체
//---------------------------------------------
typedef struct 
{
	int		evtNo;						///> event code 
	int		flag;						///> event flag
	int		device;
	DWORD	dwTick;						///> 발생일시
	char	*Contents;
} MYEVENTTABLE_T, *PMYEVENTTABLE_T;

#pragma pack()

//----------------------------------------------------------------------------------------------------------------------

#define MAX_NHEVENT		200

class NHS_AFX_EXPORT CMyEvent
{
public:
	CMyEvent();
	~CMyEvent();

private :
	BYTE			m_szCodeTable[MAX_NHEVENT];
	int				m_nCount;
	MYEVENTTABLE_T* m_pEvCodes;
	
public:
	int SetCode(int nCode);
	int GetCode(int nCode);
	int GetCondition(void);
	int ResetCode(void);
	int GetErrorCount(void);
	void ClearCode(void);
	int SetCode(int nEvt, BOOL bHappen);
	void Initialize(int nCount, BYTE *pData);
	void Terminate(void);
};



