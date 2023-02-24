// data_main.h : 거래 정보 파일 처리
// 
// 
//

#pragma once

#include "MyDefine.h"

//----------------------------------------------------------------------------------------------------------------------

typedef int ( __cdecl *DAMO_CRYPT_AES_EncryptEx)(unsigned char *out, size_t *out_len, const unsigned char *in, 
			size_t in_len, const unsigned char *key, size_t key_len, int alg, int mode, unsigned char *ivec);

//----------------------------------------------------------------------------------------------------------------------

int InitDamoCrypto(void);
int Dukpt(char* pKeyStr, char *pData, int nDataLen, char *encBuf);
void TermDamoCrypto(void);

int GetRandomSeed(int nLen, char *retBuf);
int GetRndChar(int nLen, char *retBuf);
int GetRndLen(int nIndex);
int GetEncSb(char *encDTA, int encLen);


