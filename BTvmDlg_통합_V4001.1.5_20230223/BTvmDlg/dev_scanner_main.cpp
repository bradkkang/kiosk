// 
// 
// dev_scanner_main.cpp : 승차권 리더기 MAIN
//

#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "File_Env_ini.h"
#include "dev_scanner_main.h"
#include "dev_scanner_wt.h"

#if (_USE_ATEC_SCANNER_ > 0)
#include "dev_scanner_atec.h"
#endif

#include "dev_tr_main.h"
#include "dev_tr_mem.h"
#include "dev_tr_kobus_mem.h"
#include "dev_tr_tmexp_mem.h"

#include "event_if.h"
#include "MyUtil.h"
#include "oper_config.h"

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------

static CScannerWT*		pScannerWT = NULL;

#if (_USE_ATEC_SCANNER_ > 0)
static CScannerAtec*	pScannerATEC = NULL;
#endif

static int				s_nEnable = -1;

static PDEV_CFG_T		pEnv = NULL;

//----------------------------------------------------------------------------------------------------------------------

/**
 * @brief		Scanner_Initialize
 * @details		승차권 리더기 초기화
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int Scanner_Initialize(void)
{
	int nRet;

	pEnv = (PDEV_CFG_T) GetEnvTicketReaderInfo();
	TR_LOG_OUT("승차권 리더기 Use(%d), Model(%d), Port(%d) ", pEnv->nUse, pEnv->nModel, pEnv->nPort);

	///< Scanner 사용유무
	if(pEnv->nUse <= 0)
	{
		TR_LOG_OUT("Scanner Device not use, value(%d) !!!!\n", pEnv->nUse);
		return -1;
	}

	///< Scanner 모델
	switch(pEnv->nModel)
	{
	case DEV_SCANNER_WT :	// 
	case DEV_SCANNER_ATEC : //
		break;
	default:
		TR_LOG_OUT("Scanner model error !!!!\n", pEnv->nModel);
		return -1;
	}

	///< 모델
	switch(pEnv->nModel)
	{
	case DEV_SCANNER_WT :	// 
		{
			if( pScannerWT != NULL )
			{
				delete pScannerWT;
				pScannerWT = NULL;
			}

			pScannerWT = new CScannerWT();
			nRet = pScannerWT->StartProcess(pEnv->nPort);
			if(nRet < 0)
			{
				TR_LOG_OUT("pScannerWT->StartProcess() Failure !!!!\n", nRet);
				return nRet;
			}
		}
		break;
	case DEV_SCANNER_ATEC :
		{
#if (_USE_ATEC_SCANNER_ > 0)
			if( pScannerATEC != NULL )
			{
				delete pScannerATEC;
				pScannerATEC = NULL;
			}

			pScannerATEC = new CScannerAtec();
			nRet = pScannerATEC->StartProcess(pEnv->nPort);
			if(nRet < 0)
			{
				TR_LOG_OUT("pScannerATEC->StartProcess() Failure !!!!\n", nRet);
				return nRet;
			}
#endif
		}
		break;
	}

	return 0;
}

/**
 * @brief		Scanner_Terminate
 * @details		승차권 리더기 종료
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int Scanner_Terminate(void)
{
	pEnv = (PDEV_CFG_T) GetEnvTicketReaderInfo();

	TR_LOG_OUT(" use(%d), model_no(%d), start.", pEnv->nUse, pEnv->nModel);

	///< Scanner 사용유무
	if(pEnv->nUse <= 0)
	{
		TR_LOG_OUT("Scanner Device not use, value(%d) !!!!\n", pEnv->nUse);
		return -1;
	}

	///< Scanner 모델
	switch(pEnv->nModel)
	{
	case DEV_SCANNER_WT :	// 
	case DEV_SCANNER_ATEC : //
		break;
	default:
		TR_LOG_OUT("Scanner model error !!!!\n", pEnv->nModel);
		return -1;
	}

	///< 모델
	switch(pEnv->nModel)
	{
	case DEV_SCANNER_WT :	// 
		{
			if( pScannerWT != NULL )
			{
				pScannerWT->EndProcess();

				delete pScannerWT;
				pScannerWT = NULL;
			}
		}
		break;
	case DEV_SCANNER_ATEC :
		{
#if (_USE_ATEC_SCANNER_ > 0)
			if( pScannerATEC != NULL )
			{
				pScannerATEC->EndProcess();

				delete pScannerATEC;
				pScannerATEC = NULL;
			}
#endif
		}
		break;
	}

	return 0;
}

/**
 * @brief		Scanner_SetTicketRead
 * @details		승차권 읽기 시작 or 종료
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int Scanner_SetTicketRead(BOOL bRead)
{
	int nRet = -1;
	
	pEnv = (PDEV_CFG_T) GetEnvTicketReaderInfo();

	///< Scanner 사용유무
	if(pEnv->nUse <= 0)
	{
		TR_LOG_OUT("Scanner Device not use, value(%d) !!!!\n", pEnv->nUse);
		return -1;
	}

	///< Scanner 모델
	switch(pEnv->nModel)
	{
	case DEV_SCANNER_WT :	// 
	case DEV_SCANNER_ATEC : //
		break;
	default:
		TR_LOG_OUT("Scanner model error !!!!\n", pEnv->nModel);
		return -1;
	}

	/***
	if( s_nEnable != (int)bRead )
	{
		s_nEnable = (int)bRead;
	}
	else
	{
		return 0;
	}
	***/

	///< 모델
	switch(pEnv->nModel)
	{
	case DEV_SCANNER_WT :	// 
		{
			if( pScannerWT != NULL )
			{
				if(bRead == TRUE)
				{
					pScannerWT->Enable();
				}
				else
				{
					pScannerWT->Disable();
				}

				nRet = 0;
			}
		}
		break;
	case DEV_SCANNER_ATEC :
		{
#if (_USE_ATEC_SCANNER_ > 0)
			if( pScannerATEC != NULL )
			{
				if(bRead == TRUE)
				{
					pScannerATEC->Enable();
				}
				else
				{
					pScannerATEC->Disable();
				}
				nRet = 0;
			}
#endif
		}
		break;
	}

	return nRet;
}

/**
 * @brief		Scanner_ReadTicket
 * @details		승차권 읽기 
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int Scanner_ReadTicket(void)
{
	int nRet = -1;
	int nLen = 0;
	
	pEnv = (PDEV_CFG_T) GetEnvTicketReaderInfo();

	///< Scanner 사용유무
	if(pEnv->nUse <= 0)
	{
		TR_LOG_OUT("Scanner Device not use, value(%d) !!!!\n", pEnv->nUse);
		return -1;
	}

	///< Scanner 모델
	switch(pEnv->nModel)
	{
	case DEV_SCANNER_WT :	// 
	case DEV_SCANNER_ATEC : //
		break;
	default:
		TR_LOG_OUT("Scanner model error !!!!\n", pEnv->nModel);
		return -1;
	}

	///< 모델
	switch(pEnv->nModel)
	{
	case DEV_SCANNER_WT :	// 
		{
			if( pScannerWT != NULL )
			{
				nRet = pScannerWT->OnScanning();
				if(nRet > 0)
				{
					nLen = strlen((char *)pScannerWT->m_szBarCodeText);

					CConfigTkMem::GetInstance()->m_Refund.bExp = FALSE;
					CCancRyTkKobusMem::GetInstance()->tBase.bStart = FALSE;
					CCancRyTkTmExpMem::GetInstance()->tBase.bStart = FALSE;

					TR_LOG_OUT("Len(%d : %d), data(%s) ..", nLen, sizeof(FMT_QRCODE_T), pScannerWT->m_szBarCodeText);

					if(nLen == sizeof(FMT_QRCODE_T))
					{	/// 티머니고속
						int nSvrKind = 0;

						CConfigTkMem::GetInstance()->m_Refund.bExp = TRUE;

						nSvrKind = GetConfigServerKind();
						if(nSvrKind & SVR_DVS_KOBUS)
						{	/// 코버스 
							//CCancRyTkKobusMem::GetInstance()->tBase.bStart = TRUE;
							sprintf(CCancRyTkKobusMem::GetInstance()->tBase.szTicketData, "%s", pScannerWT->m_szBarCodeText);
						}
						else
						{	/// 티머니고속
							//CCancRyTkTmExpMem::GetInstance()->tBase.bStart = TRUE;
							sprintf(CCancRyTkTmExpMem::GetInstance()->tBase.szTicketData, "%s", pScannerWT->m_szBarCodeText);
						}
					}
					else
					{	/// 시외버스
						sprintf(CCancRyTkMem::GetInstance()->szTicketData, "%s", pScannerWT->m_szBarCodeText);
					}
				}
				return nRet;
			}
		}
		break;
	case DEV_SCANNER_ATEC :
		{
#if (_USE_ATEC_SCANNER_ > 0)
			if( pScannerATEC != NULL )
			{
				nRet = pScannerATEC->OnScanning();
				if(nRet > 0)
				{
					//sprintf(CCancRyTkMem::GetInstance()->szTicketData, "%s", pScannerATEC->m_szBarCodeText);
					nLen = strlen((char *)pScannerATEC->m_szBarCodeText);

					CConfigTkMem::GetInstance()->m_Refund.bExp = FALSE;
					CCancRyTkKobusMem::GetInstance()->tBase.bStart = FALSE;
					CCancRyTkTmExpMem::GetInstance()->tBase.bStart = FALSE;

					TR_LOG_OUT("Len(%d : %d), data(%s) ..", nLen, sizeof(FMT_QRCODE_T), pScannerATEC->m_szBarCodeText);

					if(nLen == sizeof(FMT_QRCODE_T))
					{	/// 티머니고속
						int nSvrKind = 0;

						CConfigTkMem::GetInstance()->m_Refund.bExp = TRUE;

						nSvrKind = GetConfigServerKind();
						if(nSvrKind & SVR_DVS_KOBUS)
						{	/// 코버스 
							TR_LOG_OUT("코버스 환불 티켓 ..");
							sprintf(CCancRyTkKobusMem::GetInstance()->tBase.szTicketData, "%s", pScannerATEC->m_szBarCodeText);
						}
						else
						{	/// 티머니고속
							TR_LOG_OUT("티머니고속 환불 티켓 ..");
							sprintf(CCancRyTkTmExpMem::GetInstance()->tBase.szTicketData, "%s", pScannerATEC->m_szBarCodeText);
						}
					}
					else
					{	/// 시외버스
						TR_LOG_OUT("시외 환불 티켓 ..");
						sprintf(CCancRyTkMem::GetInstance()->szTicketData, "%s", pScannerATEC->m_szBarCodeText);
					}
				}
				return nRet;
			}
#endif
		}
		break;
	}

	return -1;
}

/**
 * @brief		Scanner_Reject
 * @details		승차권 반환 
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int Scanner_Reject(void)
{
	int nRet = -1;
	
	pEnv = (PDEV_CFG_T) GetEnvTicketReaderInfo();

	///< Scanner 사용유무
	if(pEnv->nUse <= 0)
	{
		TR_LOG_OUT("Scanner Device not use, value(%d) !!!!\n", pEnv->nUse);
		return -1;
	}

	///< Scanner 모델
	switch(pEnv->nModel)
	{
	case DEV_SCANNER_WT :	// 
	case DEV_SCANNER_ATEC : //
		break;
	default:
		TR_LOG_OUT("Scanner model error !!!!\n", pEnv->nModel);
		return -1;
	}

	///< 모델
	switch(pEnv->nModel)
	{
	case DEV_SCANNER_WT :	// 
		{
			if( pScannerWT != NULL )
			{
				nRet = pScannerWT->Reject();
				TR_LOG_OUT("pScannerWT->Reject(), nRet(%d) ..", nRet);
				return nRet;
			}
		}
		break;
	case DEV_SCANNER_ATEC :
		{
#if (_USE_ATEC_SCANNER_ > 0)
			if( pScannerATEC != NULL )
			{
				nRet = pScannerATEC->Reject();
				TR_LOG_OUT("pScannerATEC->Reject(), nRet(%d) ..", nRet);
				return nRet;
			}
#endif
		}
		break;
	}

	return -1;
}

/**
 * @brief		Scanner_Eject
 * @details		승차권 입수
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int Scanner_Eject(void)
{
	int nRet = -1;
	
	pEnv = (PDEV_CFG_T) GetEnvTicketReaderInfo();

	///< Scanner 사용유무
	if(pEnv->nUse <= 0)
	{
		TR_LOG_OUT("Scanner Device not use, value(%d) !!!!\n", pEnv->nUse);
		return -1;
	}

	///< Scanner 모델
	switch(pEnv->nModel)
	{
	case DEV_SCANNER_WT :	// 
	case DEV_SCANNER_ATEC : //
		break;
	default:
		TR_LOG_OUT("Scanner model error !!!!\n", pEnv->nModel);
		return -1;
	}

	///< 모델
	switch(pEnv->nModel)
	{
	case DEV_SCANNER_WT :	// 
		{
			if( pScannerWT != NULL )
			{
				nRet = pScannerWT->Eject();
				TR_LOG_OUT("pScannerWT->Eject(), nRet(%d) ..", nRet);
				return nRet;
			}
			else
			{
				TR_LOG_OUT("pScannerWT = NULL !!");
			}
		}
		break;
	case DEV_SCANNER_ATEC :
		{
#if (_USE_ATEC_SCANNER_ > 0)
			if( pScannerATEC != NULL )
			{
				nRet = pScannerATEC->Eject();
				TR_LOG_OUT("pScannerATEC->Eject(), nRet(%d) ..", nRet);
				return nRet;
			}
#endif
		}
		break;
	}

	return -1;
}

/**
 * @brief		Scanner_TPHPrint
 * @details		승차권 소인 처리
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int Scanner_TPHPrint(void)
{
	int nRet = -1;
	
	pEnv = (PDEV_CFG_T) GetEnvTicketReaderInfo();

	///< Scanner 사용유무
	if(pEnv->nUse <= 0)
	{
		TR_LOG_OUT("Scanner Device not use, value(%d) !!!!\n", pEnv->nUse);
		return -1;
	}

	///< Scanner 모델
	switch(pEnv->nModel)
	{
	case DEV_SCANNER_WT :	// 
	case DEV_SCANNER_ATEC : //
		break;
	default:
		TR_LOG_OUT("Scanner model error !!!!\n", pEnv->nModel);
		return -1;
	}

	///< 모델
	switch(pEnv->nModel)
	{
	case DEV_SCANNER_WT :	// 
		{
			if( pScannerWT != NULL )
			{
				nRet = pScannerWT->TPHPrint();
				return nRet;
			}
		}
		break;
	case DEV_SCANNER_ATEC :
		{
#if (_USE_ATEC_SCANNER_ > 0)
			if( pScannerATEC != NULL )
			{
				nRet = pScannerATEC->Branding(1, pScannerATEC->m_wLastPosY);
				//nRet = 0;
				return nRet;
			}
#endif
		}
		break;
	}

	return -1;
}


/**
 * @brief		Scanner_GetStatus
 * @details		승차권 스캐너 상태 체크
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int Scanner_GetStatus(int *nStatus)
{
	int nRet = -1;
	
	pEnv = (PDEV_CFG_T) GetEnvTicketReaderInfo();

	///< Scanner 사용유무
	if(pEnv->nUse <= 0)
	{
		TR_LOG_OUT("Scanner Device not use, value(%d) !!!!\n", pEnv->nUse);
		return -1;
	}

	///< 모델
	switch(pEnv->nModel)
	{
	case DEV_SCANNER_WT :	// 
		{
			if( pScannerWT != NULL )
			{
				nRet = pScannerWT->GetStatus(nStatus);
				return nRet;
			}
		}
		break;
	case DEV_SCANNER_ATEC :
		{
#if (_USE_ATEC_SCANNER_ > 0)
			if( pScannerATEC != NULL )
			{
				nRet = pScannerATEC->GetStatus(nStatus);
				return nRet;
			}
#endif
		}
		break;
	default:
		TR_LOG_OUT("Scanner model error !!!!\n", pEnv->nModel);
		return -1;
	}

	return -1;
}

/**
 * @brief		Scanner_GetDeviceStatus
 * @details		승차권 스캐너 상태 체크
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int Scanner_GetDeviceStatus(void)
{
	int nRet = -1;
	
	pEnv = (PDEV_CFG_T) GetEnvTicketReaderInfo();

	///< Scanner 사용유무
	if(pEnv->nUse <= 0)
	{
		TR_LOG_OUT("Scanner Device not use, value(%d) !!!!\n", pEnv->nUse);
		return -1;
	}

	///< 모델
	switch(pEnv->nModel)
	{
	case DEV_SCANNER_WT :	// 
		{
			if( pScannerWT != NULL )
			{
				nRet = pScannerWT->GetDeviceStatus();
				return nRet;
			}
		}
		break;
	case DEV_SCANNER_ATEC :
		{
#if (_USE_ATEC_SCANNER_ > 0)
			if( pScannerATEC != NULL )
			{
				nRet = pScannerATEC->GetDeviceStatus();
				return nRet;
			}
#endif
		}
		break;
	default:
		TR_LOG_OUT("Scanner model error !!!!\n", pEnv->nModel);
		return -1;
	}

	return -1;
}

