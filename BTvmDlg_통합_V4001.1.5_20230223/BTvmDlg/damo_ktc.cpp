
#include "stdafx.h"
#include <stdio.h>
#include <string.h>

#include "MyUtil.h"
#include "damo_ktc.h"
#include "dev_tr_main.h"

//----------------------------------------------------------------------------------------------------------------------

static char			s_Tmp[1024];
static HINSTANCE	hDll_Damo = NULL;
static DAMO_CRYPT_AES_EncryptEx		m_fnDAMO_CRYPT_AES_EncryptEx = NULL;

//----------------------------------------------------------------------------------------------------------------------

/**
 * @brief		InitDamoCrypto
 * @details		D'amo 암호화 모듈 초기화
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int InitDamoCrypto(void)
{
#define MAX_LOOP_CNT	10
	int  i;
	char szBasePath[256];
	char szFullName[256];

	::ZeroMemory(szBasePath, sizeof(szBasePath));
	Util_GetModulePath(szBasePath);
	sprintf(szFullName, "%s\\dukptClientDll.dll", szBasePath);

	if(hDll_Damo != NULL)
	{
		TermDamoCrypto();
	}

	for(i = 0; i < MAX_LOOP_CNT; i++)
	{
		hDll_Damo = LoadLibrary(szFullName);
		if(hDll_Damo == NULL)
		{
			Sleep(100);
			TR_LOG_OUT("LoadLibrary(), err, (%s)..", szFullName);
			continue;
			//return -1;
		}
		break;
	}

	if( i >= MAX_LOOP_CNT )
	{
		TR_LOG_OUT("InitDamoCrypto() #1 error ...");
		return -1;
	}

	for(i = 0; i < MAX_LOOP_CNT; i++)
	{
		m_fnDAMO_CRYPT_AES_EncryptEx = (DAMO_CRYPT_AES_EncryptEx) GetProcAddress(hDll_Damo, "DAMO_CRYPT_AES_EncryptEx");
		if(m_fnDAMO_CRYPT_AES_EncryptEx == NULL)
		{
			TR_LOG_OUT("m_fnDAMO_CRYPT_AES_EncryptEx == NULL");
			continue;
			//return -1;
		}
		break;
	}

	if( i >= MAX_LOOP_CNT )
	{
		TR_LOG_OUT("InitDamoCrypto() #2 error ...");
		return -2;
	}

	return 0;
}

/**
 * @brief		Dukpt
 * @details		D'amo 암호화 Encryption 
 * @param		None
 * @return		성공 : >= 0, 실패 : < 0
 */
int Dukpt(char* pKeyStr, char *pData, int nDataLen, char *encBuf)
{
	int nRet, nLen;
	int encLen, encKeyLen;
	BYTE encBinKey[1024];

	nLen = strlen(pKeyStr);
	TR_LOG_OUT("#1 pKeyStr = %s, len(%d)..", pKeyStr, nLen);
	::ZeroMemory(encBinKey, sizeof(encBinKey));
	Util_Ascii2BCD(pKeyStr, nLen, encBinKey);
	TR_LOG_HEXA("#2 encBinKey data", encBinKey, nLen / 2);

	if(m_fnDAMO_CRYPT_AES_EncryptEx == NULL)
	{
		TR_LOG_OUT("Dukpt() 함수 NULL 에러...");
		return -1;
	}

	encLen = 0;
	//encKeyLen = strlen((char *)encAscKey);
	encKeyLen = nLen / 2;
	TR_LOG_OUT("#3 encKeyLen = %d ..", encKeyLen);

	nRet = m_fnDAMO_CRYPT_AES_EncryptEx((BYTE *)encBuf, (size_t *)&encLen, (BYTE *)pData, nDataLen, encBinKey, encKeyLen, 0, 0, NULL);
//	TR_LOG_OUT("#4 m_fnDAMO_CRYPT() nRet(%d), encBuf(%s)..", nRet, encBuf);
	TR_LOG_OUT("#4 m_fnDAMO_CRYPT() nRet(%d), encLen(%d)..", nRet, encLen);

	return encLen;
}

void TermDamoCrypto(void)
{
	if(hDll_Damo != NULL)
	{
		FreeLibrary(hDll_Damo);
		hDll_Damo = NULL;
		TR_LOG_OUT("TermDamoCrypto() 성공...");
	}
	else
	{
		TR_LOG_OUT("TermDamoCrypto() 없음...");
	}
}

int GetRandomSeed(int nLen, char *retBuf)
{
	int i, k;
	char *pStr = "0123456789";

	srand((unsigned int)time(NULL));

	for(i = 0; i < nLen; i++)
	{
		k = rand() % 10;
		retBuf[i] = pStr[k];
	}

	return 0;
}

int GetRndChar(int nLen, char *retBuf)
{
	int i, nCount, input_len;
	SYSTEMTIME st;
	char *input = "abcdefghijklmnopqrstuvwxyz!@#$%^&*()_+|<>?.,ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";

	GetLocalTime(&st);

	srand((unsigned int)st.wMilliseconds + nLen);

	nCount = 0;
	input_len = strlen(input);
	for(i = 0; i < nLen; i++)
	{
		i = rand() % input_len;
		retBuf[nCount++] = input[i];
	}

	return nCount;
}

// 신용카드 번호 암호화
int GetEncSb(char *encDTA, int encLen)
{
	int i, rndLen, nRet, nOffset;
	char rndString[100];
	char retString[100];

	nOffset = 0;
	for(i = 0; i < encLen; i++)
	{
		::ZeroMemory(rndString, sizeof(rndString));
		::ZeroMemory(retString, sizeof(retString));

		srand((unsigned int) i);
		rndLen = rand() % 5 + 2;
		nRet = GetRndChar(rndLen, rndString);

		retString[nOffset++] = rndString[0];
		retString[nOffset++] = encDTA[i];
		::CopyMemory(&retString[nOffset], &rndString[1], rndLen - 1);

		//Insert(ENC_DTA, retString);
	}

	return 0;
}

int GetRndLen(int nIndex)
{
	int nRet;

	srand((unsigned int)nIndex);

	nRet = (rand() % 3) + 2;

	return nRet;
}

// int GetTicketSecurityCode(char *pTckData)
// {
// 
// }

