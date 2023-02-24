// 
// 
// dev_prt_main.cpp : 영수증 프린터 MAIN
//

#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "MyDefine.h"
#include "File_Env_ini.h"
#include "MyUtil.h"
#include "dev_prt_ticket_main.h"
#include "dev_prt_ticket_hs.h"
#include "dev_prt_ticket_nhs.h"
#include "dev_tr_main.h"
#include "oper_config.h"

//----------------------------------------------------------------------------------------------------------------------

static CPrtTicketHS*	pTicketHS = NULL;
static CPrtTicketHS_NP*	pTicketNHS = NULL;
static PDEV_CFG_T		pEnv = NULL;

//----------------------------------------------------------------------------------------------------------------------

/**
 * @brief		TckPrt_Initialize
 * @details		승차권 프린터 초기화
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int TckPrt_Initialize(void)
{
	int nRet;

	pEnv = (PDEV_CFG_T) GetEnvPrtTicketInfo();
	TR_LOG_OUT("승차권 프린터 Use(%d), Model(%d), Port(%d) ", pEnv->nUse, pEnv->nModel, pEnv->nPort);

	///< 사용유무
	if(pEnv->nUse <= 0)
	{
		TR_LOG_OUT("TicketPrinter Device not use !!!!\n");
		return -1;
	}

	///< 모델
	switch(pEnv->nModel)
	{
	case DEV_TCKPRT_HS :		/// 화성프린터 825 모델 
		{
			if( pTicketHS != NULL )
			{
				delete pTicketHS;
				pTicketHS = NULL;
			}

			pTicketHS = new CPrtTicketHS();
			nRet = pTicketHS->StartProcess(pEnv->nPort);
			if(nRet < 0)
			{
				TRACE("pTicketHS->StartProcess() Failure !!!!\n", nRet);
				return nRet;
			}
		}
		break;
	case DEV_TCKPRT_NHS :		/// 화성프린터 825 모델 - 신규 프로토콜 적용
		{
			if( pTicketNHS != NULL )
			{
				delete pTicketNHS;
				pTicketNHS = NULL;
			}

			pTicketNHS = new CPrtTicketHS_NP();
			nRet = pTicketNHS->StartProcess(pEnv->nPort);
			if(nRet < 0)
			{
				TRACE("pTicketNHS->StartProcess() Failure !!!!\n", nRet);
				return nRet;
			}
		}
		break;
	}

	return 0;
}

/**
 * @brief		TicketPrinter_Terminate
 * @details		승차권 프린터 종료
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int TckPrt_Terminate(void)
{
	pEnv = (PDEV_CFG_T) GetEnvPrtTicketInfo();

	TR_LOG_OUT(" use(%d), model_no(%d), start.", pEnv->nUse, pEnv->nModel);

	///< 사용유무
	if(pEnv->nUse <= 0)
	{
		TR_LOG_OUT("TicketPrinter Device not use !!!!\n");
		return -1;
	}

	///< 모델
	switch(pEnv->nModel)
	{
	case DEV_TCKPRT_HS :		/// 화성프린터 825 모델
		{
			if( pTicketHS != NULL )
			{
				pTicketHS->EndProcess();

				delete pTicketHS;
				pTicketHS = NULL;
			}
		}
		break;
	case DEV_TCKPRT_NHS :		/// 화성프린터 825 모델 - 신규 프로토콜 적용
		{
			if( pTicketNHS != NULL )
			{
				pTicketNHS->EndProcess();

				delete pTicketNHS;
				pTicketNHS = NULL;
			}
		}
		break;
	}

	return 0;
}

/**
 * @brief		TckPrt_GetStatus
 * @details		승차권 프린터 상태 체크
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int TckPrt_GetStatus(void)
{
	int nRet; 

	pEnv = (PDEV_CFG_T) GetEnvPrtTicketInfo();

	//TR_LOG_OUT(" use(%d), model_no(%d), start.", pEnv->nUse, pEnv->nModel);

	///< 사용유무
	if(pEnv->nUse <= 0)
	{
		//TR_LOG_OUT("Printer Device not use !!!!\n");
		return -1;
	}

	nRet = -2;

	///< 모델
	switch(pEnv->nModel)
	{
	case DEV_TCKPRT_HS :		/// 화성프린터 825 모델
		{
			if( pTicketHS != NULL )
			{
				nRet = pTicketHS->GetStatus();
			}
		}
		break;
	case DEV_TCKPRT_NHS :		/// 화성프린터 825 모델 - 신규 프로토콜 적용
		{
			if( pTicketNHS != NULL )
			{
				nRet = pTicketNHS->GetStatus();
			}
		}
		break;
	}

	return nRet;
}


/**
 * @brief		TckPrt_MrnpPrintTicket
 * @details		예매 승차권 발권
 * @param		int nSvrKind		서버종류
 * @param		char *pData			프린트 데이타
 * @return		성공 >= 0, 실패 < 0
 */
int TckPrt_MrnpPrintTicket(int nSvrKind, char *pData)
{
	int nRet = -1;
	POPER_FILE_CONFIG_T pConfig;

	pConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();

	TR_LOG_OUT("프린터 포맷 이름 = %s", pConfig->base_t.tck_paper_name);

	///< 사용유무
	if(pEnv->nUse <= 0)
	{
		TR_LOG_OUT("TicketPrinter Device not use !!!!\n");
		return -1;
	}

	if(pEnv != NULL)
	{
		///< 모델
		switch(pEnv->nModel)
		{
		case DEV_TCKPRT_HS :		/// 화성프린터 825 모델
			{
				if( pTicketHS != NULL )
				{
					TR_LOG_OUT("승차권 포맷 종류 = %s ..", pConfig->base_t.tck_paper_name);
					
					if( nSvrKind == SVR_DVS_CCBUS )
					{
						if( memcmp(pConfig->base_t.tck_paper_name, "DFJUN", 5) == 0 )
						{
							TR_LOG_OUT("전주고속 승차권은 시외를 지원하지 않음..");
						}
						else if( memcmp(pConfig->base_t.tck_paper_name, "DFIIAC", 6) == 0 )
						{	/// 인천공항 시외 포맷
							nRet = pTicketHS->CCS_DFAll_IIAC_PrintTicket(pData);
						}
						else if( memcmp(pConfig->base_t.tck_paper_name, "DF3", 3) == 0 )
						{	/// 시외 DF3 포맷
							nRet = pTicketHS->CCS_DF3_PrintTicket(pData);
						}
						else
						{	/// 금호 시외 포맷
							nRet = pTicketHS->CCS_DFAll_PrintTicket(pData);
						}
					}
					else
					{
						if( memcmp(pConfig->base_t.tck_paper_name, "DFJUN", 5) == 0 )
						{	/// 전주 고속 포맷
							nRet = pTicketHS->ExpOnlyPrintTicket(pData);
						}
						else
						{
							nRet = pTicketHS->Exp_DFAll_PrintTicket(pData);
						}
					}
					
				}
				else
				{
					TR_LOG_OUT("pTicketHS == NULL errror !!!!");
					return -1;
				}
			}
			break;
		case DEV_TCKPRT_NHS :		/// 화성프린터 825 모델 - 신규 프로토콜 적용
			{
				if( pTicketNHS != NULL )
				{			
					if( nSvrKind == SVR_DVS_CCBUS )
					{
						nRet = pTicketNHS->CbusTicketPrint(pData);
					}
					else if( nSvrKind == SVR_DVS_KOBUS )
					{
						nRet = pTicketNHS->KobusTicketPrint(pData);
					}
					else
					{
						nRet = pTicketNHS->TmExpTicketPrint(pData);
					}
				}
				else
				{
					TR_LOG_OUT("pTicketHS == NULL errror !!!!");
					return -1;
				}
			}
			break;
		}
	}
	else
	{
		TR_LOG_OUT("pEnv == NULL errror !!!!");
		return -3;
	}

	return nRet;
}

/**
 * @brief		TckPrt_PubTckPrint
 * @details		현장발권 - 승차권 발권
 * @param		int nSvrKind		서버종류
 * @param		char *pData			프린트 데이타
 * @return		성공 >= 0, 실패 < 0
 */
int TckPrt_PubPrintTicket(int nSvrKind, char *pData)
{
	int nRet = -1;
	POPER_FILE_CONFIG_T pConfig;

	pConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();

	TR_LOG_OUT("프린터 포맷 이름 = %s", pConfig->base_t.tck_paper_name);
	TR_LOG_OUT("pEnv->nUse = %d, pEnv->nModel = %d", pEnv->nUse, pEnv->nModel);

	///< 사용유무
	if(pEnv->nUse <= 0)
	{
		TR_LOG_OUT("TicketPrinter Device not use !!!!\n");
		return -1;
	}

	if(pEnv != NULL)
	{
		///< 모델
		switch(pEnv->nModel)
		{
		case DEV_TCKPRT_HS :		/// 화성프린터 825 모델
			{
				if( pTicketHS != NULL )
				{
					if( nSvrKind == SVR_DVS_CCBUS )
					{
						if( memcmp(pConfig->base_t.tck_paper_name, "DFJUN", 5) == 0 )
						{
							TR_LOG_OUT("전주고속 승차권은 시외를 지원하지 않음..");
						}
						else if( memcmp(pConfig->base_t.tck_paper_name, "DFIIAC", 6) == 0 )
						{	/// 인천공항 시외 포맷
							nRet = pTicketHS->CCS_DFAll_IIAC_PrintTicket(pData);
						}
						else if( memcmp(pConfig->base_t.tck_paper_name, "DF3", 3) == 0 )
						{	/// 시외 DF3 포맷
							nRet = pTicketHS->CCS_DF3_PrintTicket(pData);
						}
						else if( memcmp(pConfig->base_t.tck_paper_name, "DF8", 3) == 0 )
						{
							nRet = pTicketHS->CbusTicketPrint(pData);
						}
						else
						{
							nRet = pTicketHS->CCS_DFAll_PrintTicket(pData);
						}
					}
					else
					{
						if( memcmp(pConfig->base_t.tck_paper_name, "DFJUN", 3) == 0 )
						{	/// 전주 고속 포맷
							nRet = pTicketHS->ExpOnlyPrintTicket(pData);
						}
						else if( memcmp(pConfig->base_t.tck_paper_name, "DF8", 3) == 0 )
						{
							nRet = pTicketHS->KobusTicketPrint(pData);
						}
						else
						{
							nRet = pTicketHS->Exp_DFAll_PrintTicket(pData);
						}
					}
				}
				else
				{
					TR_LOG_OUT("pTicketHS == NULL errror !!!!");
					return -1;
				}
			}
			break;
		case DEV_TCKPRT_NHS :		/// 화성프린터 825 모델 - 신규 프로토콜 적용
			{
				if( pTicketNHS != NULL )
				{
					if( nSvrKind == SVR_DVS_CCBUS )
					{
						nRet = pTicketNHS->CbusTicketPrint(pData);
						//nRet = pTicketNHS->TestTicketPrint(pData);	// 20211028 ADD for TEST
					}
					else if( nSvrKind == SVR_DVS_KOBUS )
					{
						nRet = pTicketNHS->KobusTicketPrint(pData);
					}
					else
					{
						nRet = pTicketNHS->TmExpTicketPrint(pData);
					}
				}
				else
				{
					TR_LOG_OUT("pTicketHS == NULL errror !!!!");
					return -1;
				}
			}
			break;
		}
	}
	else
	{
		TR_LOG_OUT("pEnv == NULL errror !!!!");
		return -3;
	}

	return nRet;
}

// 20211116 ADD
/**
 * @brief		TckPrt_TestTckPrint
 * @details		관리자 - 테스트 승차권 발권
 * @param		int nSvrKind		서버종류
 * @param		char *pData			프린트 데이타
 * @return		성공 >= 0, 실패 < 0
 */
int TckPrt_TestPrintTicket(int nSvrKind, char *pData)
{
	int nRet = -1;
	POPER_FILE_CONFIG_T pConfig;

	pConfig = (POPER_FILE_CONFIG_T) GetOperConfigData();

	TR_LOG_OUT("프린터 포맷 이름 = %s", pConfig->base_t.tck_paper_name);
	TR_LOG_OUT("pEnv->nUse = %d, pEnv->nModel = %d", pEnv->nUse, pEnv->nModel);

	///< 사용유무
	if(pEnv->nUse <= 0)
	{
		TR_LOG_OUT("TicketPrinter Device not use !!!!\n");
		return -1;
	}

	if(pEnv != NULL)
	{
		///< 모델
		switch(pEnv->nModel)
		{
			case DEV_TCKPRT_HS :		/// 화성프린터 825 모델
				{
					if( pTicketHS != NULL )
					{
						TR_LOG_OUT("화성프린터 825 Old Model 지원하지 않음..");
						TR_LOG_OUT("화성프린터 825 신규 프로토콜만 지원..........................");
					}
					else
					{
						TR_LOG_OUT("pTicketHS == NULL errror !!!!");
						return -1;
					}
				}
				break;
			case DEV_TCKPRT_NHS :		/// 화성프린터 825 모델 - 신규 프로토콜 적용
				{
					if( pTicketNHS != NULL )
					{
						TR_LOG_OUT("화성프린터 825 신규 프로토콜만 지원..........................");
						if( nSvrKind == SVR_DVS_CCBUS )
						{
							nRet = pTicketNHS->TestTicketPrint(pData, nSvrKind);
						}
						else if( nSvrKind == SVR_DVS_KOBUS || nSvrKind == SVR_DVS_TMEXP )
						{
							nRet = pTicketNHS->TestTicketPrint(pData, nSvrKind);
						}
					}
					else
					{
						TR_LOG_OUT("pTicketHS == NULL errror !!!!");
						return -1;
					}
				}
				break;
		}
	}
	else
	{
		TR_LOG_OUT("pEnv == NULL errror !!!!");
		return -3;
	}

	return nRet;
}
// 20211116 ~ADD

/**
 * @brief		TckPrt_MilliToPoint
 * @details		승차권 프린터 mm -> point 변환
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int TckPrt_MilliToPoint(CString strMilli, CString strMod, CString &strRet)
{
	int nPoint, nCount, i;
	double dMili;
	double dMod;
	CString strVal;
	CStringArray retStr;

	dMili = atof(strMilli);
	dMod = atof(strMod);

	strVal.Format("%.*f", 3, dMili + dMod);
	nCount = Util_StringSplit(strVal, '.', retStr);

	TRACE("retStr.GetSize() = %d \n", retStr.GetSize());

	for(i = 0; i < nCount; i++)
	{
		TRACE("i = %d, str = %s \n", i, (LPSTR)(LPCTSTR)retStr[i]);
	}

	nPoint = _ttoi(retStr[0]) * 8;
	if( nCount > 1 )
	{
		int tmppoint;

		tmppoint = _ttoi(retStr[1]);

		if (tmppoint > 0 && tmppoint <= 125) 
			nPoint += 1;
		else if (tmppoint > 125 && tmppoint <= 250) 
			nPoint += 2;
		else if (tmppoint > 250 && tmppoint <= 375) 
			nPoint += 3;
		else if (tmppoint > 375 && tmppoint <= 500) 
			nPoint += 4;
		else if (tmppoint > 500 && tmppoint <= 625) 
			nPoint += 5;
		else if (tmppoint > 625 && tmppoint <= 750) 
			nPoint += 6;
		else if (tmppoint > 750 && tmppoint <= 875) 
			nPoint += 7;
		else if (tmppoint > 875 && tmppoint <= 999) 
			nPoint += 8;
	}

	strRet.Format("%04d", nPoint);

	return nPoint;
}

