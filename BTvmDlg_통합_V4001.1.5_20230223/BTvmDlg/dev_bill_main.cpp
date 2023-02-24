// 
// 
// dev_bill_main.cpp : 지폐인식기 MAIN
//

#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "File_Env_ini.h"
#include "dev_bill_main.h"
#include "dev_bill_ict.h"
#include "dev_bill_mei.h"
#include "dev_bill_onep.h"
#include "dev_tr_main.h"

//----------------------------------------------------------------------------------------------------------------------

// ICT 지폐입금기
static CBillICT*	pBillICT = NULL;
static CBillMEI*	pBillMei = NULL;
static CBillONEP*	pBillOnep = NULL;

static PDEV_CFG_T	pEnv = NULL;

static int			s_nBillState = BILL_STATE_NONE;

//----------------------------------------------------------------------------------------------------------------------

/**
 * @brief		Bill_Initialize
 * @details		지폐입금기 초기화
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int Bill_Initialize(void)
{
	int nRet = 0;

	pEnv = (PDEV_CFG_T) GetEnvBillInfo();

	TR_LOG_OUT("pEnv->nUse(%d), pEnv->nModel(%d), pEnv->nPort(%d) ", pEnv->nUse, pEnv->nModel, pEnv->nPort);

	///< 사용유무
	if(pEnv->nUse <= 0)
	{
		TR_LOG_OUT("BILL Device not use !!!!");
		return -1;
	}

	///< 모델
	switch(pEnv->nModel)
	{
	case DEV_BILL_ICT :	// 코인테크
		{
			if( pBillICT != NULL )
			{
				delete pBillICT;
				pBillICT = NULL;
			}

			pBillICT = new CBillICT();
			nRet = pBillICT->StartProcess(pEnv->nPort);
			if(nRet < 0)
			{
				TR_LOG_OUT("pBillICT->StartProcess Failure !!!!", nRet);
				return nRet;
			}
		}
		break;
	case DEV_BILL_MEI :
		{
			if( pBillMei != NULL )
			{
				delete pBillMei;
				pBillMei = NULL;
			}

			pBillMei = new CBillMEI();
			nRet = pBillMei->StartProcess(pEnv->nPort);
			if(nRet < 0)
			{
				TR_LOG_OUT("pBillMei->StartProcess Failure !!!!", nRet);
				return nRet;
			}
		}
		break;
	case DEV_BILL_ONEP :	
		{
			if( pBillOnep != NULL )
			{
				delete pBillOnep;
				pBillOnep = NULL;
			}

			pBillOnep = new CBillONEP();
			nRet = pBillOnep->StartProcess(pEnv->nPort);
			if(nRet < 0)
			{
				TR_LOG_OUT("pBillOnep->StartProcess Failure !!!!", nRet);
				return nRet;
			}
		}
		break;
	}

	return 0;
}

/**
 * @brief		Bill_Reset
 * @details		지폐입금기 리셋
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int Bill_Reset(void)
{
	int nRet = 0;

	pEnv = (PDEV_CFG_T) GetEnvBillInfo();
	///< 사용유무
	if(pEnv->nUse <= 0)
	{
		TR_LOG_OUT("BILL Device not use !!!!");
		return -1;
	}

	s_nBillState = BILL_STATE_RESET;

	if(pEnv != NULL)
	{
		///< 모델
		switch(pEnv->nModel)
		{
		case DEV_BILL_ICT :	// ICT
			{
				if( pBillICT != NULL )
				{
#ifdef __DIRECT_BILL__
					nRet = pBillICT->Reset();
					TR_LOG_OUT("pBillICT->Reset(), nRet(%d) !!!!", nRet);
#else
					nRet = pBillICT->AddQue(BILL_CMD_RESET);
#endif
				}
				else
				{
					TR_LOG_OUT("pBillICT == NULL errror !!!!");
					return -1;
				}
			}
			break;
		case DEV_BILL_MEI :	// MEI
			{
				if( pBillMei != NULL )
				{
					nRet = pBillMei->Reset();
					TR_LOG_OUT("pBillMei->Reset(), nRet(%d) !!!!", nRet);
				}
				else
				{
					TR_LOG_OUT("pBillMei == NULL errror !!!!");
					return -1;
				}
			}
			break;
		case DEV_BILL_ONEP :
			{
				if( pBillOnep != NULL )
				{
					nRet = pBillOnep->Reset();
					TR_LOG_OUT("pBillOnep->Reset(), nRet(%d) !!!!", nRet);
				}
				else
				{
					TR_LOG_OUT("pBillOnep == NULL errror !!!!");
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
 * @brief		Bill_GetStatus
 * @details		지폐입금기 상태정보 체크
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int Bill_GetStatus(void)
{
	int nRet = 0;

	pEnv = (PDEV_CFG_T) GetEnvBillInfo();
	///< 사용유무
	if(pEnv->nUse <= 0)
	{
		TR_LOG_OUT("BILL Device not use !!!!");
		return -1;
	}

	if(pEnv != NULL)
	{
		///< 모델
		switch(pEnv->nModel)
		{
		case DEV_BILL_ICT :	// 코인테크
			{
				if( pBillICT != NULL )
				{
#ifdef __DIRECT_BILL__
					nRet = pBillICT->GetDevStatus();
					TR_LOG_OUT("pBillICT->GetDevStatus(), nRet(%d) !!!!", nRet);
#else
					pBillICT->AddQue(BILL_CMD_GETSTATUS);
#endif
				}
				else
				{
					TR_LOG_OUT("pBillICT == NULL errror !!!!");
					return -1;
				}
			}
			break;
		case DEV_BILL_MEI :	// MEI
			{
				if( pBillMei != NULL )
				{
					nRet = pBillMei->GetStatus();
					TR_LOG_OUT("pBillMei->GetStatus(), nRet(%d) !!!!", nRet);
				}
				else
				{
					TR_LOG_OUT("pBillMei == NULL errror !!!!");
					return -1;
				}
			}
			break;
		case DEV_BILL_ONEP :
			{
				if( pBillOnep != NULL )
				{
					nRet = pBillOnep->GetStatus();
					TR_LOG_OUT("pBillOnep->GetStatus(), nRet(%d) !!!!", nRet);
				}
				else
				{
					TR_LOG_OUT("pBillOnep == NULL errror !!!!");
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
 * @brief		Bill_GetActionStatus
 * @details		지폐입금기 동작상태 정보
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int Bill_GetActionStatus(void)
{
	int nRet = 0;

	pEnv = (PDEV_CFG_T) GetEnvBillInfo();
	///< 사용유무
	if(pEnv->nUse <= 0)
	{
		TR_LOG_OUT("BILL Device not use !!!!");
		return -1;
	}

	if(pEnv != NULL)
	{
		///< 모델
		switch(pEnv->nModel)
		{
		case DEV_BILL_ICT :	// 코인테크
			{
// 				if( pBillICT != NULL )
// 				{
// 					nRet = pBillICT->GetStatus();
// 					TR_LOG_OUT("pBillICT->GetStatus(), nRet(%d) !!!!", nRet);
// 				}
// 				else
// 				{
// 					TR_LOG_OUT("pBillICT == NULL errror !!!!");
// 					return -1;
// 				}
			}
			break;
		case DEV_BILL_MEI :	// MEI
			{
				if( pBillMei != NULL )
				{
					nRet = pBillMei->GetActionStatus();
					//TR_LOG_OUT("pBillMei->GetActionStatus(), nRet(%d) !!!!", nRet);
				}
				else
				{
					TR_LOG_OUT("pBillMei == NULL errror !!!!");
					return -1;
				}
			}
			break;
		case DEV_BILL_ONEP :
			{
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
 * @brief		Bill_Enable
 * @details		지폐입금기 투입허가
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int Bill_Enable(void)
{
	int nRet = 0;

	if(s_nBillState == BILL_STATE_ENABLE)
	{
		TR_LOG_OUT("Already BILL_STATE_ENABLE !!!");
		return 0;
	}

	pEnv = (PDEV_CFG_T) GetEnvBillInfo();
	///< 사용유무
	if(pEnv->nUse <= 0)
	{
		TR_LOG_OUT("BILL Device not use !!!!");
		return -1;
	}

	s_nBillState = BILL_STATE_ENABLE;

	if(pEnv != NULL)
	{
		///< 모델
		switch(pEnv->nModel)
		{
		case DEV_BILL_ICT :	// 코인테크
			{
				if( pBillICT != NULL )
				{
#ifdef __DIRECT_BILL__
					nRet = pBillICT->Enable();
					TR_LOG_OUT("pBillICT->Enable(), nRet(%d) !!!!", nRet);
#else
					nRet = pBillICT->AddQue(BILL_CMD_ENABLE);
#endif
				}
				else
				{
					TR_LOG_OUT("pBillICT == NULL errror !!!!");
					return -1;
				}
			}
			break;
		case DEV_BILL_MEI :	// MEI
			{
				if( pBillMei != NULL )
				{
					nRet = pBillMei->Enable();
					TR_LOG_OUT("pBillMei->Enable(), nRet(%d) !!!!", nRet);
				}
				else
				{
					TR_LOG_OUT("pBillMei == NULL errror !!!!");
					return -1;
				}
			}
			break;
		case DEV_BILL_ONEP :
			{
				if( pBillOnep != NULL )
				{
					nRet = pBillOnep->Enable();
					TR_LOG_OUT("pBillOnep->Enable(), nRet(%d) !!!!", nRet);
				}
				else
				{
					TR_LOG_OUT("pBillOnep == NULL errror !!!!");
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
 * @brief		Bill_Inhibit
 * @details		지폐입금기 투입금지
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int Bill_Inhibit(void)
{
	int nRet = 0;

	if(s_nBillState == BILL_STATE_INHIBIT)
	{
		TR_LOG_OUT("Already BILL_STATE_INHIBIT !!!");
		return 0;
	}

	pEnv = (PDEV_CFG_T) GetEnvBillInfo();
	///< 사용유무
	if(pEnv->nUse <= 0)
	{
		TR_LOG_OUT("BILL Device not use !!!!");
		return -1;
	}

	s_nBillState = BILL_STATE_INHIBIT;

	if(pEnv != NULL)
	{
		///< 모델
		switch(pEnv->nModel)
		{
		case DEV_BILL_ICT :	// 코인테크
			{
				if( pBillICT != NULL )
				{
#ifdef __DIRECT_BILL__
					nRet = pBillICT->Inhibit();
					TR_LOG_OUT("pBillICT->Inhibit(), nRet(%d) !!!!", nRet);
#else
					pBillICT->AddQue(BILL_CMD_DISABLE);
#endif
				}
				else
				{
					TR_LOG_OUT("pBillICT == NULL errror !!!!");
					return -1;
				}
			}
			break;
		case DEV_BILL_MEI :	// MEI
			{
				if( pBillMei != NULL )
				{
					nRet = pBillMei->Inhibit();
					TR_LOG_OUT("pBillMei->Inhibit(), nRet(%d) !!!!", nRet);
				}
				else
				{
					TR_LOG_OUT("pBillMei == NULL errror !!!!");
					return -1;
				}
			}
			break;
		case DEV_BILL_ONEP :	/// ONE Plus
			{
				if( pBillOnep != NULL )
				{
					nRet = pBillOnep->Inhibit();
					TR_LOG_OUT("pBillOnep->Inhibit(), nRet(%d) !!!!", nRet);
				}
				else
				{
					TR_LOG_OUT("pBillOnep == NULL errror !!!!");
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
 * @brief		Bill_GetMoney
 * @details		지폐입금기 - 투입금액
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int Bill_GetMoney(int *ret_1k, int *ret_5k, int *ret_10k, int *ret_50k)
{
	int nRet =  -1;
	int n1k = 0, n5k = 0, n10k = 0, n50k = 0;
	static int nDebug = 0;

	pEnv = (PDEV_CFG_T) GetEnvBillInfo();
	///< 사용유무
	if(pEnv->nUse <= 0)
	{
		if(nDebug == 0)
		{
			TR_LOG_OUT("BILL Device not use !!!!");
			nDebug = 1;
		}
		return -1;
	}

	if(pEnv != NULL)
	{
		///< 모델
		switch(pEnv->nModel)
		{
		case DEV_BILL_ICT :		/// 코인테크
			{
				if( pBillICT != NULL )
				{
					nRet = pBillICT->GetMoney(&n1k, &n5k, &n10k, &n50k);

					*ret_1k = n1k;
					*ret_5k = n5k;
					*ret_10k = n10k;
					*ret_50k = n50k;

					//TR_LOG_OUT("pBillICT->GetMoney(), nRet(%d) !!!!", nRet);
					return nRet;
				}
				else
				{
					TR_LOG_OUT("pBillICT == NULL errror !!!!");
					return -1;
				}
			}
			break;
		case DEV_BILL_MEI :		/// MEI
			{
				if( pBillMei != NULL )
				{
					nRet = pBillMei->GetMoney(&n1k, &n5k, &n10k, &n50k);

					*ret_1k = n1k;
					*ret_5k = n5k;
					*ret_10k = n10k;
					*ret_50k = n50k;

					//TR_LOG_OUT("pBillMei->GetMoney(), nRet(%d) !!!!", nRet);
					return nRet;
				}
				else
				{
					TR_LOG_OUT("pBillMei == NULL errror !!!!");
					return -1;
				}
			}
			break;
		case DEV_BILL_ONEP :	/// ONE Plus
			{
				if( pBillOnep != NULL )
				{
					nRet = pBillOnep->GetMoney(&n1k, &n5k, &n10k, &n50k);

					*ret_1k = n1k;
					*ret_5k = n5k;
					*ret_10k = n10k;
					*ret_50k = n50k;

					//TR_LOG_OUT("pBillOnep->GetMoney(), nRet(%d) !!!!", nRet);
					return nRet;
				}
				else
				{
					TR_LOG_OUT("pBillMei == NULL errror !!!!");
					return -1;
				}
			}
			break;
		}
	}
	return -3;
}

/**
 * @brief		Bill_TotalEnd
 * @details		지폐입금기 투입금지
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int Bill_TotalEnd(void)
{
	int nRet = 0;

	pEnv = (PDEV_CFG_T) GetEnvBillInfo();
	///< 사용유무
	if(pEnv->nUse <= 0)
	{
		TR_LOG_OUT("BILL Device not use !!!!");
		return -1;
	}

	if(pEnv != NULL)
	{
		///< 모델
		switch(pEnv->nModel)
		{
		case DEV_BILL_ICT :		/// 코인테크
			{
				if( pBillICT != NULL )
				{
					nRet = pBillICT->TotalEnd();
					TR_LOG_OUT("pBillICT->TotalEnd(), nRet(%d) !!!!", nRet);
				}
				else
				{
					TR_LOG_OUT("pBillICT == NULL errror !!!!");
					return -1;
				}
			}
			break;
		case DEV_BILL_MEI :		/// MEI
			{
				if( pBillMei != NULL )
				{
					nRet = pBillMei->TotalEnd();
					TR_LOG_OUT("pBillMei->TotalEnd(), nRet(%d) !!!!", nRet);
				}
				else
				{
					TR_LOG_OUT("pBillMei == NULL errror !!!!");
					return -1;
				}
			}
			break;
		case DEV_BILL_ONEP :	/// ONE Plus
			{
				if( pBillOnep != NULL )
				{
					nRet = pBillOnep->TotalEnd();
					TR_LOG_OUT("pBillOnep->TotalEnd(), nRet(%d) !!!!", nRet);
				}
				else
				{
					TR_LOG_OUT("pBillOnep == NULL errror !!!!");
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
 * @brief		Bill_Terminate
 * @details		지폐입금기 통신 종료
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int Bill_Terminate(void)
{
	int nRet = 0;

	pEnv = (PDEV_CFG_T) GetEnvBillInfo();

	TR_LOG_OUT("pEnv->nUse(%d), pEnv->nModel(%d), pEnv->nPort(%d) ", pEnv->nUse, pEnv->nModel, pEnv->nPort);

	///< 사용유무
	if(pEnv->nUse <= 0)
	{
		TR_LOG_OUT("BILL Device not use !!!!");
		return -1;
	}

	///< 모델
	switch(pEnv->nModel)
	{
	case DEV_BILL_ICT :		/// 코인테크
		{
			if( pBillICT == NULL )
			{
				return -1;
			}

			nRet = pBillICT->EndProcess();
			if(nRet < 0)
			{
				TR_LOG_OUT("pBillICT->EndProcess Failure !!!!", nRet);
				return nRet;
			}
		}
		break;
	case DEV_BILL_MEI :		/// MEI
		{
			if( pBillMei == NULL )
			{
				return -1;
			}

			nRet = pBillMei->EndProcess();
			if(nRet < 0)
			{
				TR_LOG_OUT("pBillMei->EndProcess Failure !!!!", nRet);
				return nRet;
			}
		}
		break;
	case DEV_BILL_ONEP :	/// ONE Plus
		{
			if( pBillOnep == NULL )
			{
				return -1;
			}

			nRet = pBillOnep->EndProcess();
			if(nRet < 0)
			{
				TR_LOG_OUT("pBillOnep->EndProcess Failure !!!!", nRet);
				return nRet;
			}
		}
		break;
	}

	return 0;
}

