// oper_kos_file.h : 
// 
// 
//

#pragma once

//----------------------------------------------------------------------------------------------------------------------

enum _en_oper_list_
{
	OPER_FILE_ID_0100 = 0	,
	OPER_FILE_ID_0208		,
	OPER_FILE_ID_0101		,
	OPER_FILE_ID_0127		,
	OPER_FILE_ID_0128		,
	OPER_FILE_ID_0102		,
	OPER_FILE_ID_0106		,
	OPER_FILE_ID_0107		,
	OPER_FILE_ID_0108		,
	OPER_FILE_ID_0120		,
	OPER_FILE_ID_0114		,
	OPER_FILE_ID_0129		,
	OPER_FILE_ID_0118		,
	OPER_FILE_ID_0119		,
	OPER_FILE_ID_0122		,
	OPER_FILE_ID_0209		,
	OPER_FILE_ID_0124		,
	OPER_FILE_ID_0125		,
	OPER_FILE_ID_0126		,
	OPER_FILE_ID_0200		,
	OPER_FILE_ID_0201		,

	OPER_FILE_ID_0217		,
	OPER_FILE_ID_0268		,


	OPER_FILE_ID_0130		,	// 배차리스트
	OPER_FILE_ID_0135		,	// 발행내역 
	OPER_FILE_ID_0157		,	// 재발행 내역
	OPER_FILE_ID_0155		,	// 환불 내역

	OPER_FILE_ID_TRML		,	// 터미널 설정 정보
	OPER_FILE_ID_TICKET		,	// 승차권 설정 정보
	OPER_FILE_ID_CACM		,	// 운수사 설정 정보

	OPER_FILE_ID_0999		,	// CodeConv.xml
	OPER_FILE_ID_CONFIG		,


	OPER_FILE_ID_KO_CM_READMSG		= 1001,		///	(고속버스-코버스) 메시지 코드 조회	
	OPER_FILE_ID_KO_CM_READNTC		,			/// (고속버스-코버스) RTC
	OPER_FILE_ID_KO_TK_AUTHCMPT		,			/// (고속버스-코버스) 접속 컴퓨터 인증 & 터미널 코드 확인(코버스는 skip)
	OPER_FILE_ID_KO_CM_READCMNCD	,			/// (고속버스-코버스) 공통코드 조회
	OPER_FILE_ID_KO_TK_READTCKPRTG	,			/// (고속버스-코버스) 승차권 인쇄정보 조회
	OPER_FILE_ID_KO_TK_READOWNRTRML	,			/// (고속버스-코버스) 자기터미널 정보 조회 (코버스는 skip)
	OPER_FILE_ID_KO_CM_READTRML		,			/// (고속버스-코버스) 터미널 조회(전국터미널)
	OPER_FILE_ID_KO_CM_READTRMLINF	,			/// (고속버스-코버스) 터미널 정보 조회
	OPER_FILE_ID_KO_CM_READTRMLSTUP	,			/// (고속버스-코버스) 터미널환경설정보
	OPER_FILE_ID_KO_TM_EWNDINFO		,			/// (고속버스-코버스) 창구정보 조회
	OPER_FILE_ID_KO_CM_READRTRPTRML	,			/// (고속버스-코버스) 왕복터미널 정보 조회
	OPER_FILE_ID_KO_CM_READTCKKND	,			/// (고속버스-코버스) 승차권 정보 조회 (코버스는 skip)
	OPER_FILE_ID_KO_CM_MNGCACM		,			/// (고속버스-코버스) 버스 정보 조회
	OPER_FILE_ID_KO_CM_READROTINF	,			/// (고속버스-코버스) 노선 정보 조회 (코버스는 skip)
	OPER_FILE_ID_KO_CM_RDHMINQR		,			/// (고속버스-코버스) 승차홈 조회
	OPER_FILE_ID_KO_CM_READTHRUTRML	,			/// (고속버스-코버스) 경유지정보 조회
	OPER_FILE_ID_KO_TK_INQPUBPT		,			/// (고속버스-코버스) 발권내역 조회
	OPER_FILE_ID_KO_RD_CORPINF		,			/// (고속버스-코버스) 로컬 운송사 정보 조회

	OPER_FILE_ID_EZ_CM_READMSG		= 2001,		///	(고속버스-티머니고속) 메시지 코드 조회	
	OPER_FILE_ID_EZ_CM_READNTC		,			/// (고속버스-티머니고속) 공통코드 상세조회
	OPER_FILE_ID_EZ_TK_AUTHCMPT		,			/// (고속버스-티머니고속) 접속 컴퓨터 인증 & 터미널 코드 확인
	OPER_FILE_ID_EZ_CM_READCMNCD	,			/// (고속버스-티머니고속) 공통코드 조회
	OPER_FILE_ID_EZ_TK_READTCKPRTG	,			/// (고속버스-티머니고속) 승차권 인쇄정보 조회
	OPER_FILE_ID_EZ_TK_READOWNRTRML	,			/// (고속버스-티머니고속) 자기터미널 정보 조회 
	OPER_FILE_ID_EZ_CM_READTRML		,			/// (고속버스-티머니고속) 터미널 조회(전국터미널)
	OPER_FILE_ID_EZ_CM_READTRMLINF	,			/// (고속버스-티머니고속) 터미널 정보 조회
	OPER_FILE_ID_EZ_CM_READTRMLSTUP	,			/// (고속버스-티머니고속) 터미널환경설정보
	OPER_FILE_ID_EZ_MG_READWND		,			/// (고속버스-티머니고속) 창구정보 조회
	OPER_FILE_ID_EZ_CM_READRTRPTRML	,			/// (고속버스-티머니고속) 왕복터미널 정보 조회
	OPER_FILE_ID_EZ_CM_READTCKKND	,			/// (고속버스-티머니고속) 승차권 정보 조회
	OPER_FILE_ID_EZ_CM_MNGCACM		,			/// (고속버스-티머니고속) 버스 정보 조회
	OPER_FILE_ID_EZ_CM_READROTINF	,			/// (고속버스-티머니고속) 노선 정보 조회
	OPER_FILE_ID_EZ_CM_RDHMINQR		,			/// (고속버스-티머니고속) 승차홈 조회
	OPER_FILE_ID_EZ_CM_READRYRT		,			/// (고속버스-티머니고속) 환불율 정보 조회

	OPER_FILE_ID_EZ_BUS_CLS			,			/// (고속버스-티머니고속) 버스등급 데이타

	OPER_FILE_ID_EZ_TK_INQPUBPT		,			/// (고속버스-티머니고속) 발권내역 조회
	OPER_FILE_ID_EZ_TK_INQREPUBPT	,			/// (고속버스-티머니고속) 재발행내역 조회
	OPER_FILE_ID_EZ_TK_INQCANRYPT	,			/// (고속버스-티머니고속) 환불내역 조회

	OPER_FILE_ID_MAX				,
};

//----------------------------------------------------------------------------------------------------------------------

#pragma pack(1)

typedef struct 
{
	int nID;
	//char* pSvcName;
	CString strFileName;
} OPER_FILE_LIST_T, OPER_FILE_LIST_T;

typedef struct
{
	char bus_cls_cd   		[3+1]	;	///< 버스등급코드	
	char bus_cls_nm			[100]	;	///< 버스등급 명
} KOBUS_BUS_CLS_DT, *PKOBUS_BUS_CLS_DT;

typedef struct
{
	int  n_bus_dvs					;	///< 시외/고속 구분
	int  n_tck_index				;	///< 티켓 index
	char tck_kind			[4+1]	;	///< 티켓종류
	char* str_nm					;
} TCK_KIND_LIST_T, *PTCK_KIND_LIST_T;


#pragma pack()
  
//----------------------------------------------------------------------------------------------------------------------

int OperInitFile(void);
int OperGetFileName(int nID, CString& strFileName);
int OperGetFileInfo(int nID, char *pData, int *nFileSize);
int OperRecordReadFile(int nID, char *pData, int nRecodLen);
int OperReadFile(int nID, char *pData, int nRecodLen);
int OperWriteFile(int nID, char *pData, int nDataLen);
void OperTermFile(void);

int FindBusCacmName(int nSvrKind, char *bus_cacm_cd, char *retName, char *retBizNo, char *retTelNo);
int FindBusClsName(int nSvrKind, char *bus_cls_cd, char *retName);
int FindBusClsShctName(int nSvrKind, char *bus_cls_cd, char *retName);
int FindBusTckKndName(int nSvrKind, char *bus_tck_knd_cd, char *retName);
int FindTerminalName(int nLanguage, char *trml_cd, char *retName);
int Find_KobusTrmlName(int nLanguage, char *trml_cd, char *retName);
int Find_TmExpTrmlName(int nLanguage, char *trml_cd, char *retName);

int FindTicketKindData(int n_bus_dvs, char *tck_knd, char *retBuf);
int GetTckkndListData(int n_bus_dvs, int n_tck_idx, char *retBuf);

int SaveThruInfo(char *pSTrmlCode);
int Find_CCS_RotID(char *pSTrmlCode, char *pRotID);
int Find_CCS_RotName(char *pRotID, char *retNm);

int ConvertDCRT(char *dcrt, char *retBuff);
int ConvertDCKnd(char *knd, char *retBuff);

int Oper_GetTmExpRefundCode(int nRate, char *retBuf);

//----------------------------------------------------------------------------------------------------------------------


