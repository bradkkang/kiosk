// 
// 
// dev_cardreader_main.cpp : 신용카드 리더기 MAIN
//

#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "File_Env_ini.h"
#include "dev_cardreader_main.h"
#include "dev_cardreader_kicc.h"
#include "dev_tr_main.h"

//----------------------------------------------------------------------------------------------------------------------

// 한국정보통신(KICC) - ED947
static CCardRD_KICC*	pCardRdKICC = NULL;
static PDEV_CFG_T		pDevice = NULL;

//----------------------------------------------------------------------------------------------------------------------
int CardReader_Polling(void)
{
	int nRet;

	if(pCardRdKICC == NULL)
	{
		return -1;
	}

	nRet = pCardRdKICC->Polling();

	return nRet;
}

/**
 * @brief		CardReader_GetDeviceInfo
 * @details		신용카드 리더기 정보 요청
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int CardReader_GetDeviceInfo(void)
{
	int nRet;

	if(pCardRdKICC == NULL)
	{
		return -1;
	}

	nRet = pCardRdKICC->GetDeviceInfo();

	return nRet;
}

/**
 * @brief		CardReader_GetStatus
 * @details		신용카드 리더기 상태 체크
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int CardReader_GetStatus(void)
{
	int nRet;

	if(pCardRdKICC == NULL)
	{
		return -1;
	}

	nRet = pCardRdKICC->GetStatusInfo();

	return nRet;
}

/**
 * @brief		CardReader_Initialize
 * @details		신용카드 리더기 초기화
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int CardReader_Initialize(void)
{
	int nRet;

	pDevice = (PDEV_CFG_T) GetEnvCardReaderInfo();

	TR_LOG_OUT("카드리더 pDevice->nUse(%d), pDevice->nModel(%d), pDevice->nPort(%d) \n", pDevice->nUse, pDevice->nModel, pDevice->nPort);

	//nRet = GetPayMethod();
	//if(nRet == ONLY_PAY_CASH)
	//{
	//	TR_LOG_OUT("현금 전용이므로 사용안함 !!!!");
	//	return -1;
	//}

	///< 사용유무
	if(pDevice->nUse <= 0)
	{
		TR_LOG_OUT("CardReader Device not use !!!!");
		return -1;
	}

	///< 모델
	switch(pDevice->nModel)
	{
	case 1 :	// 한국정보통신(KICC) - ED-947
		if( pCardRdKICC != NULL )
		{
			delete pCardRdKICC;
			pCardRdKICC = NULL;
		}

		pCardRdKICC = new CCardRD_KICC();
		nRet = pCardRdKICC->StartProcess(pDevice->nPort);
		if(nRet < 0)
		{
			TR_LOG_OUT("pCardRdKICC->StartProcess Failure !!!!\n", nRet);
			return nRet;
		}
		Sleep(300);

		pCardRdKICC->GetDeviceInfo();

		break;
	case 2 :
		break;
	}

	return 0;
}

/**
 * @brief		CardReader_Terminate
 * @details		신용카드 리더기 종료
 * @param		None
 * @return		성공 >= 0, 실패 < 0
 */
int CardReader_Terminate(void)
{
	pDevice = (PDEV_CFG_T) GetEnvCardReaderInfo();

	TR_LOG_OUT(" model_no(%d), start.", pDevice->nModel);

	///< 사용유무
	if(pDevice->nUse <= 0)
	{
		TR_LOG_OUT("CardReader Device not use !!!!\n");
		return -1;
	}

	///< 모델
	switch(pDevice->nModel)
	{
	case 1 :	// 한국정보통신(KICC) - ED-947
		if( pCardRdKICC != NULL )
		{
			pCardRdKICC->EndProcess();

			delete pCardRdKICC;
			pCardRdKICC = NULL;
		}
		break;
	case 2 :
		break;
	}

	return 0;
}



