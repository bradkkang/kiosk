// 
// 
// cmn_util.cpp : 공통유틸 소스 파일
//

#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "MyDefine.h"
#include "cmn_util.h"

/**
 * @brief		IsHangul
 * @details		버퍼에 한글이 깨졌는지 확인하는 함수
 * @param		char *pData			데이타
 * @param		int nDataLen		데이타 길이
 * @return		한글 완성 = 0, 한글 미완성 < 0
 */
int IsHangul(char *pData, int nDataLen)
{
	int i;
	BOOL bComplete;

	bComplete = TRUE;

	for(i = 0; i < nDataLen; i++)
	{
		if(pData[i] == 0)
			break;

		if(pData[i] & 0x80)
		{
			bComplete = !bComplete;
		}
	}

	if( bComplete == FALSE )
	{
		return -1;
	}

	return 0;
}

/**
 * @brief		GetLanguageStr
 * @details		메시지 정보 조회
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
char* GetLanguageStr(int nIndex)
{
	static char *szLangStr[] = {
		"KO", "EN", "CN", "JP",
	};

	if( (nIndex < LANG_DVS_KO) || (nIndex >= LANG_DVS_MAX) )
	{
		return (char *)NULL;
	}

	return szLangStr[nIndex];
}

/**
 * @brief		KTC_MemClear
 * @details		메모리 초기화
 * @param		void *pData			초기화할 메모리 버퍼
 * @param		int nSize			길이
 * @return		항상 0
 */
int KTC_MemClear(void *pData, int nSize)
{
	memset(pData, 0x00, nSize);
	memset(pData, 0xFF, nSize);
	memset(pData, 0x00, nSize);

	return 0;
}

/**
 * @brief		CheckMinusValue
 * @details		음수 데이타 체크
 * @param		int nValue			데이타 값
 * @param		int nSubValue		뺄값
 * @return		뺄셈 값
 */
int SubtractValue(int nValue, int nSubValue)
{
	int nRet;

	nRet = nValue - nSubValue;
	if(nRet < 0)
	{
		return 0;
	}
	return nRet;
}

/**
 * @brief		CMN_MakePasswdChars
 * @details		구매자정보 암호화 문자 만들기
 * @param		char *retBuf		암호화 문자 버퍼
 * @param		char *pSrc			원본 데이타
 * @return		None
 */
void CMN_MakePasswdChars(char *retBuf, char *pSrc)
{
	int nLen;

	/// 구매자 정보 (개인:핸드폰번호, 법인:사업자번호)
	nLen = strlen(pSrc);
	if(nLen <= 0)
	{
		return;
	}

	if(nLen < 4)
	{
		sprintf(retBuf, "****");
	}
	else
	{
		for(int i = 0; i < nLen; i++)
		{
			if( i < 3 )
			{
				retBuf[i] = pSrc[i];
			}
			else if( i >= (nLen - 4) )
			{
				retBuf[i] = pSrc[i];
			}
			else
			{
				retBuf[i] = '*';
			}
		}
		// ~2020.03.30 modify
	}
}


