// 
// 
// dev_dispenser_main.cpp : 지폐방출기 MAIN 
//

#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "File_Env_ini.h"
#include "dev_dispenser_main.h"
#include "dev_dispenser_ht.h"
#include "dev_dispenser_nht.h"
#include "dev_tr_main.h"

//----------------------------------------------------------------------------------------------------------------------

#define MAX_CDU			250

static CCduHt*			pDispenserHT = NULL;
static CCduNHt*			pDispenserNHT = NULL;
static PDEV_CFG_T		pEnv = NULL;

//----------------------------------------------------------------------------------------------------------------------

/**
 * @brief		CDU_SensorStatus
 * @details		지폐방출기 센서 상태값 가져오기
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int CDU_SensorStatus(void)
{
	int nRet;

	///< 사용유무
	if(pEnv->nUse <= 0)
	{
		TR_LOG_OUT("Dispenser Device not use !!!!\n");
		return -99;
	}

	if(pEnv != NULL)
	{
		///< 모델
		switch(pEnv->nModel)
		{
		case 1 :	// 한틀시스템
			if( pDispenserHT != NULL )
			{
				nRet = pDispenserHT->SensorStatus();
				TR_LOG_OUT("pDispenserHT->SensorStatus(), nRet(%d) !!!!", nRet);
			}
			else
			{
				TR_LOG_OUT("pDispenserHT == NULL errror !!!!");
				return -1;
			}
			break;
		case 2 :	// 한틀시스템_new
			if( pDispenserNHT != NULL )
			{
				nRet = pDispenserNHT->SensorStatus();
				TR_LOG_OUT("pDispenserNHT->SensorStatus(), nRet(%d) !!!!", nRet);
			}
			else
			{
				TR_LOG_OUT("pDispenserNHT == NULL errror !!!!");
				return -1;
			}
			break;
		case 3 :
			TR_LOG_OUT("pEnv->nModel(%d), unknown errror !!!!", pEnv->nModel);
			return -2;
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
 * @brief		CDU_GetVersion
 * @details		지폐방출기 버전 정보
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int CDU_GetVersion(void)
{
	int nRet;

	///< 사용유무
	if(pEnv->nUse <= 0)
	{
		TR_LOG_OUT("Dispenser Device not use !!!!\n");
		return -1;
	}

	if(pEnv != NULL)
	{
		///< 모델
		switch(pEnv->nModel)
		{
		case 1 :	// 한틀시스템
			if( pDispenserHT != NULL )
			{
				nRet = pDispenserHT->GetVersion();
				TR_LOG_OUT("pDispenserHT->GetVersion(), nRet(%d) !!!!", nRet);
			}
			else
			{
				TR_LOG_OUT("pDispenserHT == NULL errror !!!!");
				return -1;
			}
			break;
		case 2 :	// 한틀시스템_new
			if( pDispenserNHT != NULL )
			{
				nRet = pDispenserNHT->GetVersion();
				TR_LOG_OUT("pDispenserNHT->GetVersion(), nRet(%d) !!!!", nRet);
			}
			else
			{
				TR_LOG_OUT("pDispenserNHT == NULL errror !!!!");
				return -1;
			}
			break;
		case 3 :
			TR_LOG_OUT("pEnv->nModel(%d), unknown errror !!!!", pEnv->nModel);
			return -2;
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
 * @brief		CDU_Dispense
 * @details		지폐방출기 방출
 * @param		int n1k			1,000원권 배출 수량
 * @param		int n10k		10,000원권 배출 수량
 * @return		성공 >= 0, 실패 < 0
 */
int CDU_Dispense(int n1k, int n10k)
{
	int nRet;

	///< 사용유무
	if(pEnv->nUse <= 0)
	{
		TR_LOG_OUT("Dispenser Device not use !!!!\n");
		return -1;
	}

	if(pEnv != NULL)
	{
		///< 모델
		switch(pEnv->nModel)
		{
		case 1 :	// 한틀시스템
			if( pDispenserHT != NULL )
			{
				nRet = pDispenserHT->Dispense(n1k, n10k);
				TR_LOG_OUT("pDispenserHT->Dispense(), nRet(%d), n1k(%d), n10k(%d) !!!!", nRet, n1k, n10k);
			}
			else
			{
				TR_LOG_OUT("pDispenserHT == NULL errror !!!!");
				return -1;
			}
			break;
		case 2 :	// 한틀시스템_new
			if( pDispenserNHT != NULL )
			{
				nRet = pDispenserNHT->Dispense(n1k, n10k);
				TR_LOG_OUT("pDispenserNHT->Dispense(), nRet(%d), n1k(%d), n10k(%d) !!!!", nRet, n1k, n10k);
			}
			else
			{
				TR_LOG_OUT("pDispenserNHT == NULL errror !!!!");
				return -1;
			}
			break;
		case 3 :
			TR_LOG_OUT("pEnv->nModel(%d), unknown errror !!!!", pEnv->nModel);
			return -2;
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
 * @brief		CDU_OutMoney
 * @details		지폐방출기 지폐 출금
 * @param		int n1k			1,000원권 배출 수량
 * @param		int n10k		10,000원권 배출 수량
 * @return		성공 >= 0, 실패 < 0
 */
int CDU_OutMoney(int n1k, int n10k, int* nOut1k, int* nOut10k)
{
	int nRet;
	int i, nLoop, nCount, nValue, nMAX;
	DISPENSE_INFO_T dspInfo;

	i = nLoop = nCount = nValue = 0;
//	nMAX = 5;
	nMAX = MAX_CDU;

	::ZeroMemory(&dspInfo, sizeof(DISPENSE_INFO_T));

	///< 사용유무
	if(pEnv->nUse <= 0)
	{
		TR_LOG_OUT("Dispenser Device not use !!!!\n");
		return -1;
	}

	if(pEnv != NULL)
	{
		///< 모델
		switch(pEnv->nModel)
		{
		case 1 :	// 한틀시스템
			if( pDispenserHT != NULL )
			{
				if(n1k > 0)
				{
					nLoop = n1k / nMAX;
					if( (n1k % nMAX) > 0 )
					{
						nLoop += 1;
					}

					TR_LOG_OUT("#1. nLoop(%d) !!!!", nLoop);

					for(i = 0; i < nLoop; i++)
					{
						if( i == (nLoop - 1) )
						{
							if( (n1k % nMAX) > 0 )
							{
								nValue = n1k % nMAX;
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

						//nRet = pDispenserHT->Dispense(n1k, n10k);
						nRet = pDispenserHT->Dispense(nValue, 0);
						CDU_GetDispenseInfo((char *)&dspInfo);
						*nOut1k += dspInfo.nOutCount[0];
						TR_LOG_OUT("pDispenserHT->Dispense(1k), nRet(%d), In(%d), Out(%d) !!!!", nRet, nValue, *nOut1k);
						if(nRet < 0)
						{
							break;
						}
					}
				}

				if(n10k > 0)
				{
					nLoop = n10k / nMAX;
					if( (n10k % nMAX) > 0 )
					{
						nLoop += 1;
					}

					TR_LOG_OUT("#1. nLoop(%d) !!!!", nLoop);

					for(i = 0; i < nLoop; i++)
					{
						if( i == (nLoop - 1) )
						{
							if( (n10k % nMAX) > 0 )
							{
								nValue = n10k % nMAX;
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

						//nRet = pDispenserHT->Dispense(n1k, n10k);
						nRet = pDispenserHT->Dispense(0, nValue);
						CDU_GetDispenseInfo((char *)&dspInfo);
						*nOut10k += dspInfo.nOutCount[1];
						TR_LOG_OUT("pDispenserHT->Dispense(10k), nRet(%d), In(%d), Out(%d) !!!!", nRet, nValue, *nOut10k);
						if(nRet < 0)
						{
							break;
						}
					}
				}
			}
			else
			{
				TR_LOG_OUT("pDispenserHT == NULL errror !!!!");
				return -1;
			}
			break;
		case 2 :	// 한틀시스템_new
			if( pDispenserNHT != NULL )
			{
				if(n1k > 0)
				{
					nLoop = n1k / nMAX;
					if( (n1k % nMAX) > 0 )
					{
						nLoop += 1;
					}

					TR_LOG_OUT("#1. nLoop(%d) !!!!", nLoop);

					for(i = 0; i < nLoop; i++)
					{
						if( i == (nLoop - 1) )
						{
							if( (n1k % nMAX) > 0 )
							{
								nValue = n1k % nMAX;
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

						//nRet = pDispenserHT->Dispense(n1k, n10k);
						nRet = pDispenserNHT->Dispense(nValue, 0);
						CDU_GetDispenseInfo((char *)&dspInfo);
						*nOut1k += dspInfo.nOutCount[0];
						TR_LOG_OUT("pDispenserNHT->Dispense(1k), nRet(%d), In(%d), Out(%d) !!!!", nRet, nValue, *nOut1k);
						if(nRet < 0)
						{
							break;
						}
					}
				}

				if(n10k > 0)
				{
					nLoop = n10k / nMAX;
					if( (n10k % nMAX) > 0 )
					{
						nLoop += 1;
					}

					TR_LOG_OUT("#1. nLoop(%d) !!!!", nLoop);

					for(i = 0; i < nLoop; i++)
					{
						if( i == (nLoop - 1) )
						{
							if( (n10k % nMAX) > 0 )
							{
								nValue = n10k % nMAX;
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

						//nRet = pDispenserHT->Dispense(n1k, n10k);
						nRet = pDispenserNHT->Dispense(0, nValue);
						CDU_GetDispenseInfo((char *)&dspInfo);
						*nOut10k += dspInfo.nOutCount[1];
						TR_LOG_OUT("pDispenserNHT->Dispense(10k), nRet(%d), In(%d), Out(%d) !!!!", nRet, nValue, *nOut10k);
						if(nRet < 0)
						{
							break;
						}
					}
				}
			}
			else
			{
				TR_LOG_OUT("pDispenserNHT == NULL errror !!!!");
				return -1;
			}
			break;
		case 3 :
			TR_LOG_OUT("pEnv->nModel(%d), unknown errror !!!!", pEnv->nModel);
			return -2;
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
 * @brief		CDU_TestDispense
 * @details		지폐방출기 시험 방출
 * @param		int n1k			1,000원권 배출 수량
 * @param		int n10k		10,000원권 배출 수량
 * @return		성공 >= 0, 실패 < 0
 */
int CDU_TestDispense(int n1k, int n10k)
{
	int nRet;

	///< 사용유무
	if(pEnv->nUse <= 0)
	{
		TR_LOG_OUT("Dispenser Device not use !!!!\n");
		return -1;
	}

	if(pEnv != NULL)
	{
		///< 모델
		switch(pEnv->nModel)
		{
		case 1 :	// 한틀시스템
			if( pDispenserHT != NULL )
			{
				nRet = pDispenserHT->TestDispense(n1k, n10k);
				TR_LOG_OUT("pDispenserHT->TestDispense(), nRet(%d), n1k(%d), n10k(%d) !!!!", nRet, n1k, n10k);
			}
			else
			{
				TR_LOG_OUT("pDispenserHT == NULL errror !!!!");
				return -1;
			}
			break;
		case 2 :	// 한틀시스템_new
			if( pDispenserNHT != NULL )
			{
				nRet = pDispenserNHT->TestDispense(n1k, n10k);
				TR_LOG_OUT("pDispenserNHT->TestDispense(), nRet(%d), n1k(%d), n10k(%d) !!!!", nRet, n1k, n10k);
			}
			else
			{
				TR_LOG_OUT("pDispenserNHT == NULL errror !!!!");
				return -1;
			}
			break;
		case 3 :
			TR_LOG_OUT("pEnv->nModel(%d), unknown errror !!!!", pEnv->nModel);
			return -2;
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
 * @brief		CDU_GetDispenseInfo
 * @details		지폐방출기 방출
 * @param		char *pData		방출된 정보
 * @return		항상 = 0
 */
int CDU_GetDispenseInfo(char *pData)
{
	int nRet;

	///< 사용유무
	if(pEnv->nUse <= 0)
	{
		TR_LOG_OUT("Dispenser Device not use !!!!\n");
		return -1;
	}

	if(pEnv != NULL)
	{
		///< 모델
		switch(pEnv->nModel)
		{
		case 1 :	// 한틀시스템
			if( pDispenserHT != NULL )
			{
				nRet = pDispenserHT->GetDispenseInfo(pData);
			}
			else
			{
				TR_LOG_OUT("pDispenserHT == NULL errror !!!!");
				return -1;
			}
			break;
		case 2 :	// 한틀시스템_new
			if( pDispenserNHT != NULL )
			{
				nRet = pDispenserNHT->GetDispenseInfo(pData);
			}
			else
			{
				TR_LOG_OUT("pDispenserNHT == NULL errror !!!!");
				return -1;
			}
			break;
		case 3 :
			TR_LOG_OUT("pEnv->nModel(%d), unknown errror !!!!", pEnv->nModel);
			return -2;
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
 * @brief		CDU_SetInfo
 * @details		지폐방출기 버전 정보
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int CDU_SetInfo(void)
{
	int nRet;

	///< 사용유무
	if(pEnv->nUse <= 0)
	{
		TR_LOG_OUT("Dispenser Device not use !!!!\n");
		return -1;
	}

	if(pEnv != NULL)
	{
		///< 모델
		switch(pEnv->nModel)
		{
		case 1 :	// 한틀시스템
			if( pDispenserHT != NULL )
			{
				nRet = pDispenserHT->SetInfo();
				TR_LOG_OUT("pDispenserHT->SetInfo(), nRet(%d) !!!!", nRet);
			}
			else
			{
				TR_LOG_OUT("pDispenserHT == NULL errror !!!!");
				return -1;
			}
			break;
		case 2 :	// 한틀시스템_new
			if( pDispenserNHT != NULL )
			{
				nRet = pDispenserNHT->SetInfo();
				TR_LOG_OUT("pDispenserNHT->SetInfo(), nRet(%d) !!!!", nRet);
			}
			else
			{
				TR_LOG_OUT("pDispenserNHT == NULL errror !!!!");
				return -1;
			}
			break;
		case 3 :
			TR_LOG_OUT("pEnv->nModel(%d), unknown errror !!!!", pEnv->nModel);
			return -2;
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
 * @brief		CDU_Reset
 * @details		지폐방출기 Reset
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int CDU_Reset(void)
{
	int nRet;

	///< 사용유무
	if(pEnv->nUse <= 0)
	{
		TR_LOG_OUT("Dispenser Device not use !!!!\n");
		return -1;
	}

	if(pEnv != NULL)
	{
		///< 모델
		switch(pEnv->nModel)
		{
		case 1 :	// 한틀시스템
			if( pDispenserHT != NULL )
			{
				nRet = pDispenserHT->Initialize(0);
				TR_LOG_OUT("pDispenserHT->Initialize(), nRet(%d) !!!!", nRet);
			}
			else
			{
				TR_LOG_OUT("pDispenserHT == NULL errror !!!!");
				return -1;
			}
			break;
		case 2 :	// 한틀시스템_new
			if( pDispenserNHT != NULL )
			{
				nRet = pDispenserNHT->Initialize(0);
				TR_LOG_OUT("pDispenserNHT->Initialize(), nRet(%d) !!!!", nRet);
			}
			else
			{
				TR_LOG_OUT("pDispenserNHT == NULL errror !!!!");
				return -1;
			}
			break;
		case 3 :
			TR_LOG_OUT("pEnv->nModel(%d), unknown errror !!!!", pEnv->nModel);
			return -2;
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
 * @brief		CDU_Initialize
 * @details		지폐방출기 초기화
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int CDU_Initialize(void)
{
	int nRet;

	pEnv = (PDEV_CFG_T) GetEnvDispenserInfo();
	TR_LOG_OUT("지폐 방출기 Use(%d), Model(%d), Port(%d) ", pEnv->nUse, pEnv->nModel, pEnv->nPort);

	///< 사용유무
	if(pEnv->nUse <= 0)
	{
		TR_LOG_OUT("Dispenser Device not use !!!!\n");
		return -1;
	}

	///< 모델
	switch(pEnv->nModel)
	{
	case 1 :	// 한틀시스템
		if( pDispenserHT != NULL )
		{
			delete pDispenserHT;
			pDispenserHT = NULL;
		}

		pDispenserHT = new CCduHt();
		nRet = pDispenserHT->StartProcess(pEnv->nPort);
		if(nRet < 0)
		{
			TR_LOG_OUT("pDispenserHT->StartProcess() Failure !!!!\n", nRet);
			return nRet;
		}
		break;
	case 2 :	// 한틀시스템_new
		if( pDispenserNHT != NULL )
		{
			delete pDispenserNHT;
			pDispenserNHT = NULL;
		}

		TR_LOG_OUT("지폐방출기 port (%d)..", pEnv->nPort);

		pDispenserNHT = new CCduNHt();
		nRet = pDispenserNHT->StartProcess(pEnv->nPort);
		if(nRet < 0)
		{
			TR_LOG_OUT("pDispenserNHT->StartProcess(), nRet(%d) Failure !!!!\n", nRet);
			return nRet;
		}
		break;
	case 3 :
		break;
	}

	return 0;
}

/**
 * @brief		CDU_Terminate
 * @details		지폐방출기 종료
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int CDU_Terminate(void)
{
	pEnv = (PDEV_CFG_T) GetEnvDispenserInfo();

	TR_LOG_OUT(" use(%d), model_no(%d), start.", pEnv->nUse, pEnv->nModel);

	///< 사용유무
	if(pEnv->nUse <= 0)
	{
		TR_LOG_OUT("Dispenser Device not use !!!!\n");
		return -1;
	}

	///< 모델
	switch(pEnv->nModel)
	{
	case 1 :	// 
		if( pDispenserHT != NULL )
		{
			pDispenserHT->EndProcess();

			delete pDispenserHT;
			pDispenserHT = NULL;
		}
		break;
	case 2 :	// 한틀시스템_new
		if( pDispenserNHT != NULL )
		{
			pDispenserNHT->EndProcess();

			delete pDispenserNHT;
			pDispenserNHT = NULL;
		}
		break;
	case 3 :
		break;
	}

	return 0;
}


/**
 * @brief		CDU_GetStatus
 * @details		지폐방출기 센서 상태값 가져오기
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int CDU_GetStatus(void)
{
	int nRet;

	///< 사용유무
	if(pEnv->nUse <= 0)
	{
		TR_LOG_OUT("Dispenser Device not use !!!!\n");
		return -99;
	}

	if(pEnv != NULL)
	{
		///< 모델
		switch(pEnv->nModel)
		{
		case 1 :	// 한틀시스템
			if( pDispenserHT != NULL )
			{
				nRet = pDispenserHT->GetStatus();
				//TR_LOG_OUT("pDispenserHT->SensorStatus(), nRet(%d) !!!!", nRet);
			}
			else
			{
				TR_LOG_OUT("pDispenserHT == NULL errror !!!!");
				return -99;
			}
			break;
		case 2 :	// 한틀시스템_new
			if( pDispenserNHT != NULL )
			{
				nRet = pDispenserNHT->GetStatus();
				//TR_LOG_OUT("pDispenserNHT->SensorStatus(), nRet(%d) !!!!", nRet);
			}
			else
			{
				TR_LOG_OUT("pDispenserNHT == NULL errror !!!!");
				return -99;
			}
			break;
		case 3 :
			TR_LOG_OUT("pEnv->nModel(%d), unknown errror !!!!", pEnv->nModel);
			return -99;
		}
	}
	else
	{
		TR_LOG_OUT("pEnv == NULL errror !!!!");
		return -99;
	}

	return nRet;
}