// 
// 
// dev_coin_main.cpp : 동전방출기 MAIN
//

#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "File_Env_ini.h"
#include "dev_coin_main.h"
#include "dev_coin_ws.h"
#include "dev_tr_main.h"
#include "event_if.h"


//----------------------------------------------------------------------------------------------------------------------
#define MAX_COIN		250

//----------------------------------------------------------------------------------------------------------------------

static CCoinWS*			pCoin100WS = NULL;
static CCoinWS*			pCoin500WS = NULL;
static PDEV_CFG_T		pEnv100 = NULL;
static PDEV_CFG_T		pEnv500 = NULL;

//----------------------------------------------------------------------------------------------------------------------

/**
 * @brief		Coin100_Out
 * @details		동전방출기(100원) 동전방출
 * @param		int nCount			방출갯수
 * @return		성공 >= 0, 실패 < 0
 */
static int Coin100_Out(int nCount)
{
	int nRet;

	nRet = -1;
	pEnv100 = (PDEV_CFG_T) GetEnvCoin100Info();

	///< 사용유무
	if(pEnv100->nUse <= 0)
	{
		TR_LOG_OUT("Coin100 Device not use !!!!\n");
		return -1;
	}

	if(pEnv100 != NULL)
	{
		///< 모델
		switch(pEnv100->nModel)
		{
		case 1 :	// 코인테크
			if( pCoin100WS != NULL )
			{
				nRet = pCoin100WS->OutCoin(nCount);
				TR_LOG_OUT("pCoin100WS->OutCoin(), nRet(%d), nCount(%d) !!!!", nRet, nCount);
			}
			else
			{
				TR_LOG_OUT("pCoin100WS == NULL errror !!!!");
				return -2;
			}
			break;
		default:
			TR_LOG_OUT("pCoin100WS nModel errror !!!!");
			nRet = -3;
			break;
		}
	}

	return nRet;
}

/**
 * @brief		Coin100_Reset
 * @details		동전방출기(100원) Reset
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int Coin100_Reset(void)
{
	int nRet;

	nRet = -1;
	pEnv100 = (PDEV_CFG_T) GetEnvCoin100Info();

	///< 사용유무
	if(pEnv100->nUse <= 0)
	{
		TR_LOG_OUT("Coin100 Device not use !!!!\n");
		return -1;
	}

	if(pEnv100 != NULL)
	{
		///< 모델
		switch(pEnv100->nModel)
		{
		case 1 :	// 코인테크
			if( pCoin100WS != NULL )
			{
				nRet = pCoin100WS->Reset();
				TR_LOG_OUT("pCoin100WS->Reset(), nRet(%d) !!!!", nRet);
			}
			else
			{
				TR_LOG_OUT("pCoin100WS == NULL errror !!!!");
				return -2;
			}
			break;
		default:
			TR_LOG_OUT("pCoin100WS nModel errror !!!!");
			nRet = -3;
			break;
		}
	}

	return nRet;
}

/**
 * @brief		Coin100_GetStatus
 * @details		동전방출기(100원) get status info
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int Coin100_GetStatus(void)
{
	int nRet;

	nRet = -99;
	pEnv100 = (PDEV_CFG_T) GetEnvCoin100Info();

	///< 사용유무
	if(pEnv100->nUse <= 0)
	{
		TR_LOG_OUT("Coin100 Device not use !!!!\n");
		return -99;
	}


	if(pEnv100 != NULL)
	{
		///< 모델
		switch(pEnv100->nModel)
		{
		case 1 :	// 코인테크
			if( pCoin100WS != NULL )
			{
				nRet = pCoin100WS->GetStatus();
				//TR_LOG_OUT("pCoin100WS->GetStatus(), nRet(%d) !!!!", nRet);
			}
			else
			{
				TR_LOG_OUT("pCoin100WS == NULL errror !!!!");
				return -99;
			}
			break;
		default:
			TR_LOG_OUT("pCoin100WS nModel errror !!!!");
			nRet = -99;
			break;
		}
	}
	return nRet;
}

/**
 * @brief		Coin100_GetOutInfo
 * @details		동전방출기(100원) get 배출정보
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int Coin100_GetOutInfo(void)
{
	int nRet;

	nRet = -1;
	pEnv100 = (PDEV_CFG_T) GetEnvCoin100Info();

	///< 사용유무
	if(pEnv100->nUse <= 0)
	{
		TR_LOG_OUT("Coin100 Device not use !!!!\n");
		return -1;
	}

	if(pEnv100 != NULL)
	{
		///< 모델
		switch(pEnv100->nModel)
		{
		case 1 :	// 코인테크
			if( pCoin100WS != NULL )
			{
				nRet = pCoin100WS->GetOutInfo();
				TR_LOG_OUT("pCoin100WS->GetOutInfo(), nRet(%d) !!!!", nRet);
			}
			else
			{
				TR_LOG_OUT("pCoin100WS == NULL errror !!!!");
				return -2;
			}
			break;
		default:
			TR_LOG_OUT("pCoin100WS nModel errror !!!!");
			nRet = -3;
			break;
		}
	}

	return nRet;
}

/**
 * @brief		Coin100_Initialize
 * @details		동전방출기(100원) 초기화
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int Coin100_Initialize(void)
{
	int nRet;

	pEnv100 = (PDEV_CFG_T) GetEnvCoin100Info();
	TR_LOG_OUT("100원 동전방출기 Use(%d), Model(%d), Port(%d) ", pEnv100->nUse, pEnv100->nModel, pEnv100->nPort);

	///< 사용유무
	if(pEnv100->nUse <= 0)
	{
		TR_LOG_OUT("Coin100 Device not use !!!!\n");
		return -1;
	}

	///< 모델
	switch(pEnv100->nModel)
	{
	case 1 :	// 우성테크
		if( pCoin100WS != NULL )
		{
			delete pCoin100WS;
			pCoin100WS = NULL;
		}

		pCoin100WS = new CCoinWS();
		nRet = pCoin100WS->StartProcess(pEnv100->nPort, 100);
		if(nRet < 0)
		{
			TRACE("pCoin100WS->StartProcess() Failure !!!!\n", nRet);
			return nRet;
		}
		break;
	case 2 :
		break;
	}

	return 0;
}

/**
 * @brief		Coin100_Terminate
 * @details		동전방출기(100원) 종료
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int Coin100_Terminate(void)
{
	pEnv100 = (PDEV_CFG_T) GetEnvCoin100Info();

	TR_LOG_OUT(" use(%d), model_no(%d), start.", pEnv100->nUse, pEnv100->nModel);

	///< 사용유무
	if(pEnv100->nUse <= 0)
	{
		TR_LOG_OUT("CardReader Device not use !!!!\n");
		return -1;
	}

	///< 모델
	switch(pEnv100->nModel)
	{
	case 1 :	// 
		if( pCoin100WS != NULL )
		{
			pCoin100WS->EndProcess();

			delete pCoin100WS;
			pCoin100WS = NULL;
		}
		break;
	case 2 :
		break;
	}

	return 0;
}

/**
 * @brief		Coin500_Out
 * @details		동전방출기(500원) 동전방출
 * @param		int nCount			방출갯수
 * @return		성공 >= 0, 실패 < 0
 */
static int Coin500_Out(int nCount)
{
	int nRet;

	nRet = -1;
	pEnv500 = (PDEV_CFG_T) GetEnvCoin500Info();

	///< 사용유무
	if(pEnv500->nUse <= 0)
	{
		TR_LOG_OUT("Coin500 Device not use !!!!\n");
		return -1;
	}

	if(pEnv500 != NULL)
	{
		///< 모델
		switch(pEnv500->nModel)
		{
		case 1 :	// 코인테크
			if( pCoin500WS != NULL )
			{
				nRet = pCoin500WS->OutCoin(nCount);
				TR_LOG_OUT("pCoin500WS->OutCoin(), nRet(%d), nCount(%d) !!!!", nRet, nCount);
			}
			else
			{
				TR_LOG_OUT("pCoin500WS == NULL errror !!!!");
				return -2;
			}
			break;
		default:
			TR_LOG_OUT("pCoin500WS nModel errror !!!!");
			nRet = -3;
			break;
		}
	}

	return nRet;
}

/**
 * @brief		Coin500_Reset
 * @details		동전방출기(500원) Reset
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int Coin500_Reset(void)
{
	int nRet;

	nRet = -1;
	pEnv500 = (PDEV_CFG_T) GetEnvCoin500Info();

	///< 사용유무
	if(pEnv500->nUse <= 0)
	{
		TR_LOG_OUT("Coin500 Device not use !!!!\n");
		return -1;
	}

	if(pEnv500 != NULL)
	{
		///< 모델
		switch(pEnv500->nModel)
		{
		case 1 :	// 코인테크
			if( pCoin500WS != NULL )
			{
				nRet = pCoin500WS->Reset();
				TR_LOG_OUT("pCoin500WS->Reset(), nRet(%d) !!!!", nRet);
			}
			else
			{
				TR_LOG_OUT("pCoin500WS == NULL errror !!!!");
				return -2;
			}
			break;
		default:
			TR_LOG_OUT("pCoin500WS nModel errror !!!!");
			nRet = -3;
			break;
		}
	}

	return nRet;
}

/**
 * @brief		Coin500_GetStatus
 * @details		동전방출기(500원) get status info
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int Coin500_GetStatus(void)
{
	int nRet;

	nRet = -99;
	pEnv500 = (PDEV_CFG_T) GetEnvCoin500Info();

	///< 사용유무
	if(pEnv500->nUse <= 0)
	{
		TR_LOG_OUT("Coin500 Device not use !!!!\n");
		return -99;
	}

	if(pEnv500 != NULL)
	{
		///< 모델
		switch(pEnv500->nModel)
		{
		case 1 :	// 코인테크
			if( pCoin500WS != NULL )
			{
				nRet = pCoin500WS->GetStatus();
				//TR_LOG_OUT("pCoin500WS->GetStatus(), nRet(%d) !!!!", nRet);
			}
			else
			{
				TR_LOG_OUT("pCoin500WS == NULL errror !!!!");
				return -99;
			}
			break;
		default:
			TR_LOG_OUT("pCoin500WS nModel errror !!!!");
			nRet = -99;
			break;
		}
	}

	return nRet;
}

/**
 * @brief		Coin500_GetOutInfo
 * @details		동전방출기(500원) get 배출정보
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int Coin500_GetOutInfo(void)
{
	int nRet;

	nRet = -1;
	pEnv500 = (PDEV_CFG_T) GetEnvCoin500Info();

	///< 사용유무
	if(pEnv500->nUse <= 0)
	{
		TR_LOG_OUT("Coin500 Device not use !!!!\n");
		return -1;
	}

	if(pEnv500 != NULL)
	{
		///< 모델
		switch(pEnv500->nModel)
		{
		case 1 :	// 코인테크
			if( pCoin500WS != NULL )
			{
				nRet = pCoin500WS->GetOutInfo();
				TR_LOG_OUT("pCoin500WS->GetOutInfo(), nRet(%d) !!!!", nRet);
			}
			else
			{
				TR_LOG_OUT("pCoin500WS == NULL errror !!!!");
				return -2;
			}
			break;
		default:
			TR_LOG_OUT("pCoin500WS nModel errror !!!!");
			nRet = -3;
			break;
		}
	}

	return nRet;
}

/**
 * @brief		Coin500_Initialize
 * @details		동전방출기(500원) 초기화
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int Coin500_Initialize(void)
{
	int nRet;

	pEnv500 = (PDEV_CFG_T) GetEnvCoin500Info();
	TR_LOG_OUT("500원 동전방출기 Use(%d), Model(%d), Port(%d) ", pEnv500->nUse, pEnv500->nModel, pEnv500->nPort);

	///< 사용유무
	if(pEnv500->nUse <= 0)
	{
		TR_LOG_OUT("Coin500 Device not use !!!!\n");
		return -1;
	}

	///< 모델
	switch(pEnv500->nModel)
	{
	case 1 :	// 
		if( pCoin500WS != NULL )
		{
			delete pCoin500WS;
			pCoin500WS = NULL;
		}

		pCoin500WS = new CCoinWS();
		nRet = pCoin500WS->StartProcess(pEnv500->nPort, 500);
		if(nRet < 0)
		{
			TRACE("pCoin500WS->StartProcess() Failure !!!!\n", nRet);
			return nRet;
		}
		break;
	case 2 :
		break;
	}

	return 0;
}

/**
 * @brief		Coin500_Terminate
 * @details		동전방출기(500원) 종료
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int Coin500_Terminate(void)
{
	pEnv500 = (PDEV_CFG_T) GetEnvCoin500Info();

	TR_LOG_OUT(" use(%d), model_no(%d), start.", pEnv500->nUse, pEnv500->nModel);

	///< 사용유무
	if(pEnv500->nUse <= 0)
	{
		TR_LOG_OUT("CardReader Device not use !!!!\n");
		return -1;
	}

	///< 모델
	switch(pEnv500->nModel)
	{
	case 1 :	// 
		if( pCoin500WS != NULL )
		{
			pCoin500WS->EndProcess();

			delete pCoin500WS;
			pCoin500WS = NULL;
		}
		break;
	case 2 :
		break;
	}

	return 0;
}

/**
 * @brief		Coin_Initialize
 * @details		동전방출기(100원, 500원) 초기화
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int Coin_Initialize(void)
{
	int nRet;

	nRet = Coin100_Initialize();
	if(nRet < 0)
	{
		TR_LOG_OUT("Coin100_Initialize() Failure. !!");
	}

	nRet = Coin500_Initialize();
	if(nRet < 0)
	{
		TR_LOG_OUT("Coin500_Initialize() Failure. !!");
	}

	return nRet;
}

/**
 * @brief		Coin_Terminate
 * @details		동전방출기(100원, 500원) 종료
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int Coin_Terminate(void)
{
	int nRet;

	nRet = Coin100_Terminate();
	if(nRet < 0)
	{
		TR_LOG_OUT("Coin100_Terminate() Failure. !!");
	}
	nRet = Coin500_Terminate();
	if(nRet < 0)
	{
		TR_LOG_OUT("Coin500_Terminate() Failure. !!");
	}

	return 0;
}

/**
 * @brief		Coin_ChangeMoney
 * @details		동전 거스름돈 방출
 * @param		int nCount_100		100원 동전 방출갯수
 * @param		int nCount_500		500원 동전 방출갯수
 * @return		성공 >= 0, 실패 < 0
 */
int Coin_ChangeMoney(int n100, int n500)
{
	int nRet1 = 0, nRet2 = 0;

	if(n100 > 0)
	{
		nRet1 = Coin100_GetStatus();

		nRet1 = Coin100_Out(n100);
		TR_LOG_OUT("Coin100_Out(), input(%d), nRet(%d) !!!!", n100, nRet1);
	}

	if(n500 > 0)
	{
		nRet2 = Coin500_GetStatus();

		nRet2 = Coin500_Out(n500);
		TR_LOG_OUT("Coin500_Out(), input(%d), nRet(%d) !!!!", n500, nRet2);
	}

	if( (nRet1 < 0) || (nRet2 < 0) )
	{
		return -1;
	}

	SetCheckEventCode(EC_COIN_NO_OUT, FALSE);

	return 0;
}

/**
 * @brief		Coin_OutMoney
 * @details		동전 방출
 * @param		int nCount_100		100원 동전 방출갯수
 * @param		int nCount_500		500원 동전 방출갯수
 * @return		성공 >= 0, 실패 < 0
 */
int Coin_OutMoney(int n100, int n500, int *nOut100, int *nOut500)
{
	int nRet1 = 0, nRet2 = 0;
	int i, nLoop, nCount, nValue, nMAX;

	i = nValue = nLoop = nCount = 0;
//	nMAX = 5;
	nMAX = MAX_COIN;

	if(n100 > 0)
	{
		nLoop = n100 / nMAX;
		if( (n100 % nMAX) > 0 )
		{
			nLoop += 1;
		}

		TR_LOG_OUT("#1. nLoop(%d) !!!!", nLoop);

		for(i = 0; i < nLoop; i++)
		{
			if( i == (nLoop - 1) )
			{
				if( (n100 % nMAX) > 0 )
				{
					nValue = n100 % nMAX;
				}
				else
				{
					nValue = nMAX;
				}
			}
			else
			{
				nValue = nMAX;
			}
			nRet1 = Coin100_Out(nValue);
			*nOut100 += Coin100_GetOutInfo();
			TR_LOG_OUT("Coin100_Out(), in(%d), out(%d), nRet(%d) !!!!", nValue, *nOut100, nRet1);
			if(nRet1 < 0)
			{
				break;
			}
		}
	}

	if(n500 > 0)
	{
		nLoop = n500 / nMAX;
		if( (n500 % nMAX) > 0 )
		{
			nLoop += 1;
		}

		TR_LOG_OUT("#2. nLoop(%d) !!!!", nLoop);

		for(i = 0; i < nLoop; i++)
		{
			if( i == (nLoop - 1) )
			{
				if( (n500 % nMAX) > 0 )
				{
					nValue = n500 % nMAX;
				}
				else
				{
					nValue = nMAX;
				}
			}
			else
			{
				nValue = nMAX;
			}

			nRet2 = Coin500_Out(nValue);
			*nOut500 += Coin500_GetOutInfo();
			TR_LOG_OUT("Coin500_Out(), in(%d), out(%d), nRet(%d) !!!!", nValue, *nOut500, nRet2);
			if(nRet2 < 0)
			{
				break;
			}
		}
	}

	if( (nRet1 < 0) || (nRet2 < 0) )
	{
		return -1;
	}

	return 0;
}

/**
 * @brief		Coin_ChangeMoney2
 * @details		동전 거스름돈 방출
 * @param		int nCount_100		100원 동전 방출갯수
 * @param		int nCount_500		500원 동전 방출갯수
 * @return		성공 >= 0, 실패 < 0
 */
int Coin_ChangeMoney2(int n100, int n500)
{
	int nRet1 = 0, nRet2 = 0;
	int i = 0, nLoop = 0, nCount = 0;
	int nMAX = 250;
	int nOut100 = 0, nOut500 = 0;
	int n100_sum = 0, n500_sum = 0;

	if(n100 > 0)
	{
		nLoop = n100 / nMAX;
		if(n100 % nMAX)
		{
			nLoop += 1;
		}

		for(i = 0; i < nCount; i++)
		{
			if( i == (nLoop - 1) )
			{
				nCount = n100 % nMAX;
			}
			else
			{
				nCount = nMAX;
			}

			nRet1 = Coin100_Out(nCount);
			TR_LOG_OUT("Coin100_Out(), input(%d), nRet(%d) !!!!", nCount, nRet1);

			Coin_GetOutInfo(&nOut100, &nOut500);
			n100_sum += nOut100;
			n500_sum += nOut500;

			if(nRet1 < 0)
			{
				break;
			}
		}
	}

	nOut100 = 0;
	nOut500 = 0;

	if(n500 > 0)
	{
		nLoop = n500 / nMAX;
		if(n500 % nMAX)
		{
			nLoop += 1;
		}

		for(i = 0; i < nCount; i++)
		{
			if( i == (nLoop - 1) )
			{
				nCount = n500 % nMAX;
			}
			else
			{
				nCount = nMAX;
			}

			nRet2 = Coin500_Out(nCount);
			TR_LOG_OUT("Coin500_Out(), input(%d), nRet(%d) !!!!", nCount, nRet2);

			Coin_GetOutInfo(&nOut100, &nOut500);
			n100_sum += nOut100;
			n500_sum += nOut500;

			if(nRet2 < 0)
			{
				break;
			}
		}
	}

	TR_LOG_OUT("시재방출 -> 갯수 = (100 = %d, 500 = %d) !!!!", n100_sum, n500_sum);

	if( (nRet1 < 0) || (nRet2 < 0) )
	{
		return -1;
	}

	SetCheckEventCode(EC_COIN_NO_OUT, FALSE);

	return 0;
}

/**
 * @brief		Coin_Reset
 * @details		동전 방출기 리셋
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int Coin_Reset(BOOL b100, BOOL b500)
{
	int nRet1 = 0, nRet2 = 0;

	TR_LOG_OUT(" start, b100(%d), b500(%d) ...", b100, b500);

	if(b100 == TRUE)
	{
		nRet1 = Coin100_Reset();
		Coin100_GetStatus();
	}

	if(b500 == TRUE)
	{
		nRet2 = Coin500_Reset();
		Coin500_GetStatus();
	}

	if( (nRet1 < 0) || (nRet2 < 0) )
	{
		return -1;
	}

	return 0;
}

/**
 * @brief		Coin_GetStatus
 * @details		동전 방출기 상태정보 얻기
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int Coin_GetStatus(void)
{
	int nRet;
	static int nRetry100 = 0;
	static int nRetry500 = 0;

	nRet = Coin100_GetStatus();
	if(nRet < 0)
	{
		if(nRet != -99)
		{

			if(++nRetry100 >= 3)
			{
				SetCheckEventCode(EC_COIN_100_COMM_ERR, TRUE);
			}
		}
	}
	else
	{
		if(nRetry100 > 0)
		{
			nRetry100 = 0;
		}
		SetCheckEventCode(EC_COIN_100_COMM_ERR, FALSE);
	}


	nRet = Coin500_GetStatus();
	if(nRet < 0)
	{
		if(nRet != -99)
		{
			if(++nRetry500 >= 3)
			{
				SetCheckEventCode(EC_COIN_500_COMM_ERR, TRUE);
			}
		}
	}
	else
	{
		if(nRetry500 > 0)
		{
			nRetry500 = 0;
		}
		SetCheckEventCode(EC_COIN_500_COMM_ERR, FALSE);
	}

	return nRet;
}

/**
 * @brief		Coin_GetOutInfo
 * @details		동전 방출기 배출정보 얻기
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int Coin_GetOutInfo(int *n100, int *n500)
{
	*n100 = Coin100_GetOutInfo();
	*n500 = Coin500_GetOutInfo();

	return 0;
}

/**
 * @brief		Coin100_TotalEnd
 * @details		동전방출기(100원) 데이타 Clear
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int Coin100_TotalEnd(void)
{
	int nRet;

	nRet = -1;
	pEnv100 = (PDEV_CFG_T) GetEnvCoin100Info();

	///< 사용유무
	if(pEnv100->nUse <= 0)
	{
		TR_LOG_OUT("Coin100 Device not use !!!!\n");
		return -1;
	}

	if(pEnv100 != NULL)
	{
		///< 모델
		switch(pEnv100->nModel)
		{
		case 1 :	// 코인테크
			if( pCoin100WS != NULL )
			{
				nRet = pCoin100WS->TotalEnd();
				TR_LOG_OUT("pCoin100WS->TotalEnd(), nRet(%d) !!!!", nRet);
			}
			else
			{
				TR_LOG_OUT("pCoin100WS == NULL errror !!!!");
				return -2;
			}
			break;
		default:
			TR_LOG_OUT("pCoin100WS nModel errror !!!!");
			nRet = -3;
			break;
		}
	}

	return nRet;
}

/**
 * @brief		Coin500_TotalEnd
 * @details		동전방출기(500원) 데이타 Clear
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
static int Coin500_TotalEnd(void)
{
	int nRet;

	nRet = -1;
	pEnv500 = (PDEV_CFG_T) GetEnvCoin500Info();

	///< 사용유무
	if(pEnv500->nUse <= 0)
	{
		TR_LOG_OUT("Coin500 Device not use !!!!\n");
		return -1;
	}

	if(pEnv500 != NULL)
	{
		///< 모델
		switch(pEnv500->nModel)
		{
		case 1 :	// 코인테크
			if( pCoin500WS != NULL )
			{
				nRet = pCoin500WS->TotalEnd();
				TR_LOG_OUT("pCoin500WS->TotalEnd(), nRet(%d) !!!!", nRet);
			}
			else
			{
				TR_LOG_OUT("pCoin500WS == NULL errror !!!!");
				return -2;
			}
			break;
		default:
			TR_LOG_OUT("pCoin500WS nModel errror !!!!");
			nRet = -3;
			break;
		}
	}

	return nRet;
}

/**
 * @brief		Coin_TotalEnd
 * @details		동전 데이타 Clear
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int Coin_TotalEnd(void)
{
	int nRet1 = 0, nRet2 = 0;

	nRet1 = Coin100_TotalEnd();
	nRet2 = Coin500_TotalEnd();

	return 0;
}
