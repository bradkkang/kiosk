// 
// 
// dev_bill_onep.cpp : 지폐인식기 (원플러스)
//

#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "MyDefine.h"
#include "MyUtil.h"
#include "File_Env_ini.h"
#include "event_if.h"
#include "dev_bill_onep.h"

//----------------------------------------------------------------------------------------------------------------------

#define LOG_OUT(fmt, ...)		{ CBillONEP::m_clsLog.LogOut("[%s:%d] " fmt " - ", __FUNCTION__, __LINE__, __VA_ARGS__ );  }
#define LOG_HEXA(x,y,z)			{ CBillONEP::m_clsLog.HexaDump(x, y, z); }

//----------------------------------------------------------------------------------------------------------------------

/**
 * @brief		CBillONEP
 * @details		생성자
 */
CBillONEP::CBillONEP()
{
	m_hThread = NULL;
	m_hAccMutex = NULL;
	m_bConnected = FALSE;

	m_n1k = m_n5k = m_n10k = m_n50k = m_nTotalFare = 0;
	m_dwStatus = 0L;

	m_nDebug = TRUE;
	m_bStacking = FALSE;
	m_bEnable = FALSE;
}

/**
 * @brief		~CBillONEP
 * @details		소멸자
 */
CBillONEP::~CBillONEP()
{
	LOG_OUT(" start !! ");
	EndProcess();
}

/**
 * @brief		LOG_INIT
 * @details		LOG 파일 초기화
 * @param		None
 * @return		None
 */
void CBillONEP::LOG_INIT(void)
{
	//m_clsLog.SetData(30, "\\Log\\BillICT");
	m_clsLog.SetData(30, "\\Log\\Bill\\ONEP");
	m_clsLog.Initialize();
	m_clsLog.Delete();
}

/**
 * @brief		Locking
 * @details		IPC Lock
 * @param		None
 * @return		항상 : 0
 */
int CBillONEP::Locking(void)
{
	if(m_hAccMutex != NULL) 
	{
		::WaitForSingleObject(m_hAccMutex, INFINITE);
	}

	return 0;	
}

/**
 * @brief		UnLocking
 * @details		IPC UnLock
 * @param		None
 * @return		항상 : 0
 */
int CBillONEP::UnLocking(void)
{
	if(m_hAccMutex != NULL) 
	{
		::ReleaseMutex(m_hAccMutex);
	}

	return 0;	
}

/**
 * @brief		SetStatusValue
 * @details		1 Byte 전송 data
 * @param		BYTE byData		전송 데이타
 * @return		항상 : 0
 */
int CBillONEP::SetStatusValue(BYTE byData)
{
	int i, nFind;
	BILLONEP_STATUS_T stData[] = {
		{  0, "RESET_WAIT"			, "Reset후 Initial 동작중"					},
		{  1, "WAIT"				, "Initial후 대기 중(입수금지상태)"			},
		{  2, "STARTWAIT"			, "입수 가능 상태"							},
		{  3, "RECOGNITION_RETURN"	, "인식작업 중 오류로 인한 반환 작업 중"		},
		{  4, "RECOGNITION_WAIT"	, "인식작업 중"								},
		{  5, "RECOGNITION_END"		, "인식완료 후 대기 중(입수금지상태)"			},
		{  6, "RETURN_START"		, "인식완료 대기상태에서 반환 명령"			},
		{  7, "RETURN_WAIT"			, "반환동작 중"								},
		{  8, "RETURN_END"			, "반환동작 완료 후 대기 중(입수금지상태)"	},
		{  9, "STACK_START"			, "인식완료 대기상태에서 Stack 명령"			},
		{ 10, "STACK_WAIT"			, "Stack 동작 중"							},
		{ 11, "STACK_END"			, "Stack 동작 완료 후 대기 중(입수금지상태)"	},
		{ 12, "ERROR_WAIT"			, "동작 Error로 인한 대기 중(입수금지상태)"	},
		{ 13, "INSERT_ENABLE"		, "입수금지 상태에서 입수 가능 명령"			},
		{ 14, "INSERT_DISABLE"		, "입수가능 상태에서 입수 금지 명령"			},
		{ 16, "STACK_OPENED"		, "Stack이 Open된 상태(입수금지상태)"			},
		{ 17, "ForceStack_WAIT"		, "강제 입수 동작 중"							},
		{ 18, "ForceStack_END"		, "강제 입수 완료 후 대기 중(입수금지 상태)"	},
	};

	nFind = -1;
	for(i = 0; i < sizeof(stData) / sizeof(BILLONEP_STATUS_T); i++)
	{
		if(stData[i].byStatus == byData)
		{
			nFind = i;
			LOG_OUT("ActiveStatus = 0x%02X, [%s] [%s] ...", stData[i].byStatus & 0xFF, stData[i].szShtNm, stData[i].pStr);
			break;
		}
	}

	if(nFind < 0)
	{
		return -1;
	}

	return 0;
}

/**
 * @brief		SendPacket
 * @details		패킷 데이타 전송
 * @param		BYTE *pData		데이타 버퍼
 * @param		int nDataLen	데이타 길이
 * @return		전송 데이타 길이
 */
int CBillONEP::SendPacket(BYTE *pData, int nDataLen)
{
	int nOffset = 0, i;
	BYTE bySum = 0;

	/// STX
	m_szTxBuf[nOffset++] = '$';
	
	/// DATA
	nOffset += Util_MyCopyMemory(&m_szTxBuf[nOffset], pData, nDataLen);
	
	/// CheckSum
	for(i = 0; i < nDataLen; i++)
	{
		bySum += pData[i];
	}
	m_szTxBuf[nOffset++] = bySum;

	LOG_HEXA("SendPacket", m_szTxBuf, nOffset);

	return m_clsComm.SendData(m_szTxBuf, nOffset);
}

/**
 * @brief		GetPacket
 * @details		데이타 수신 처리
 * @param		BYTE *retBuf		수신 데이타
 * @return		항상 : 0
 */
int CBillONEP::GetPacket(BYTE *retBuf)
{
	int ch, nCount, nState, i;
	DWORD dwTick;
	BYTE byCalcSum = 0;

	nCount = nState = 0;
	dwTick = ::GetTickCount();
	while( Util_CheckExpire(dwTick) < 200 )	
	{
		ch = m_clsComm.ReadData();
		if(ch < 0) 
		{
			continue;
		}

		ch = ch & 0xFF;

		//if( m_nDebug == TRUE )
		//	LOG_OUT("[RX] = %02X ", ch);

		switch(nState)
		{
		case 0 :	/// stx
			if(ch == '$')
			{
				nState++;
				nCount = 0;
			}
			break;
		case 1 :	/// data 1
		case 2 :	/// data 2
		case 3 :	/// data 3
			retBuf[nCount++] = ch;
			nState++;
			break;
		case 4 :	/// check sum
			retBuf[nCount++] = ch;
			goto got_packet;
		}
	}

got_packet:

	byCalcSum = 0;
	for(i = 0; i < nCount - 1; i++)
	{
		byCalcSum += retBuf[i];
	}

	if( byCalcSum != retBuf[nCount - 1] )
	{
		LOG_OUT("Checksum error, (%02X, %02X) !!!", byCalcSum, retBuf[nCount - 1]);
		return -1;
	}

	LOG_HEXA("GetPacket", retBuf, nCount);
	return nCount;
}

/**
 * @brief		Reset
 * @details		초기화 명령 처리
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CBillONEP::Reset(void)
{
	Locking();

	SetCheckEventCode(EC_BILL_SENSOR_ERR, FALSE);

	UnLocking();

	return 0;
}

/**
 * @brief		doEventProcess
 * @details		Event TX 수신 처리
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CBillONEP::doEventProcess(void)
{
	int		i, nRet;
	BYTE	byValue;
	BYTE	sBuffer[100];
	BILLONEP_EVENT_T evtBill[] = {
		{	0x01, 	"Start Sensor관련 Error"					},
		{	0x02, 	"Shutter Sensor관련 Error"				},
		{	0x03, 	"End Sensor관련 Error"					},
		{	0x04, 	"이송 모터 관련 Error"					},
		{	0x05, 	"Stack 모터 관련 Error"					},
		{	0x09, 	"불손한 의도가 시도되어 1분간 입수금지"	},
		{	0x0B, 	"인식 Sensor1 관련 Error"				},
		{	0x0C, 	"인식 Sensor2 관련 Error"				},
		{	0x0D, 	"인식 Sensor3 관련 Error"				},
		{	0x0E, 	"인식 Sensor4 관련 Error"				},
		{	0x0F, 	"인식 Sensor5 관련 Error"				},
		{	0x10, 	"인식 Sensor6 관련 Error"				},
		{	0x11, 	"인식 Sensor7 관련 Error"				},
		{	0x12, 	"인식 Sensor8 관련 Error"				},
	};

	i = nRet = byValue = 0;
	::ZeroMemory(sBuffer, sizeof(sBuffer));

	if( memcmp(m_szRxBuf, "ES", 2) == 0 )
	{
		byValue = m_szRxBuf[2] & 0xFF;

		sBuffer[0] = 'e';
		sBuffer[1] = 's';
		sBuffer[2] = byValue;
		nRet = SendPacket(sBuffer, 3);

		if( (m_szRxBuf[2] & 0xFF) > 0 )
		{
			for(i = 0; i < sizeof(evtBill) / sizeof(BILLONEP_EVENT_T); i++)
			{
				if(evtBill[i].byEvent == byValue)
				{
					LOG_OUT("에러발생, [0x%02X][%s]....", evtBill[i].byEvent & 0xFF, evtBill[i].pStr);
					break;
				}
			}
			SetCheckEventCode(EC_BILL_SENSOR_ERR, TRUE);
		}
		else
		{
			SetCheckEventCode(EC_BILL_SENSOR_ERR, FALSE);
		}
		return 0;
	}

	return -1;
}

/**
 * @brief		GetStatus
 * @details		상태 명령 처리
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CBillONEP::GetStatus(void)
{
	int		nRet, nValue;
	int		n1k, n5k, n10k, n50k;
	DWORD	dwTick;
	BYTE	sBuffer[] = "\x47\x42\x3F";		/// "GB?"

	nRet = nValue = 0;
	n1k = n5k = n10k = n50k = 0;

	Locking();

	LOG_OUT("start..");

	m_bStacking = FALSE;

	nRet = SendPacket(sBuffer, 3);

	dwTick = ::GetTickCount();
	while( Util_CheckExpire(dwTick) <= 1000 )
	{
		nRet = GetPacket(m_szRxBuf);
		if(nRet > 0)
		{
			if( memcmp(m_szRxBuf, (BYTE *)"gb", 2) == 0 )
			{
				nValue = (m_szRxBuf[2] & 0xFF) * 1000;

				//LOG_OUT("raw 투입 정보 = [0x%02X], 투입금액(%d)..", m_szRxBuf[2] & 0xFF, nValue);

				switch(nValue)
				{
				case 1000 :
					n1k = 1;
					LOG_OUT("1,000원 투입");
					break;
				case 5000 :
					n5k = 1;
					LOG_OUT("5,000원 투입");
					break;
				case 10000 :
					n10k = 1;
					LOG_OUT("10,000원 투입");
					break;
				case 50000 :
					n50k = 1;
					LOG_OUT("50,000원 투입");
					break;
				}

				m_n1k += n1k;
				m_n5k += n5k;
				m_n10k += n10k;
				m_n50k += n50k;
				m_nTotalFare = BILL_SUM(m_n1k, m_n5k, m_n10k, m_n50k);
				
				//LOG_OUT("총 투입 정보 = 1k(%d), 5k(%d), 10k(%d), 50k(%d), 총투입금액(%d)..", m_n1k, m_n5k, m_n10k, m_n50k, m_nTotalFare);
				UnLocking();

				if(m_bEnable == TRUE)
				{
					Enable();
				}

				return nRet;
			}
			else
			{
				nRet = doEventProcess();
				if(nRet < 0)
				{
					LOG_OUT("response error = %02X, %02X, %02X ", m_szRxBuf[0] & 0xFF, m_szRxBuf[1] & 0xFF, m_szRxBuf[2] & 0xFF);
				}

			}
		}
	}

	UnLocking();

	return -1;
}

/**
 * @brief		GetDevStatus
 * @details		상태 명령 처리
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CBillONEP::GetDevStatus(void)
{
	int		nRet, nValue, i;
	DWORD	dwTick;
	BYTE	sBuffer[] = "\x47\x41\x3F";		/// "GA?"
	BILLONEP_STATUS_T stValues[] = {
		{  0, "RESET_WAIT"			, "Reset후 Initial 동작중"					},
		{  1, "WAIT"				, "Initial후 대기 중(입수금지상태)"			},
		{  2, "STARTWAIT"			, "입수 가능 상태"							},
		{  3, "RECOGNITION_RETURN"	, "인식작업 중 오류로 인한 반환 작업 중"		},
		{  4, "RECOGNITION_WAIT"	, "인식작업 중"								},
		{  5, "RECOGNITION_END"		, "인식완료 후 대기 중(입수금지상태)"			},
		{  6, "RETURN_START"		, "인식완료 대기상태에서 반환 명령"			},
		{  7, "RETURN_WAIT"			, "반환동작 중"								},
		{  8, "RETURN_END"			, "반환동작 완료 후 대기 중(입수금지상태)"	},
		{  9, "STACK_START"			, "인식완료 대기상태에서 Stack 명령"			},
		{ 10, "STACK_WAIT"			, "Stack 동작 중"							},
		{ 11, "STACK_END"			, "Stack 동작 완료 후 대기 중(입수금지상태)"	},
		{ 12, "ERROR_WAIT"			, "동작 Error로 인한 대기 중(입수금지상태)"	},
		{ 13, "INSERT_ENABLE"		, "입수금지 상태에서 입수 가능 명령"			},
		{ 14, "INSERT_DISABLE"		, "입수가능 상태에서 입수 금지 명령"			},
		{ 16, "STACK_OPENED"		, "Stack이 Open된 상태(입수금지상태)"			},
		{ 17, "ForceStack_WAIT"		, "강제 입수 동작 중"						},
		{ 18, "ForceStack_END"		, "강제 입수 완료 후 대기 중(입수금지 상태)"	},
	};
	static BYTE prevStatus = 0xFF;

	nRet = nValue = 0;

	Locking();

	LOG_OUT("start..");

	nRet = SendPacket(sBuffer, 3);

	dwTick = ::GetTickCount();
	while( Util_CheckExpire(dwTick) <= 1000 )
	{
		nRet = GetPacket(m_szRxBuf);
		if(nRet > 0)
		{
			if( memcmp(m_szRxBuf, (BYTE *)"ga", 2) == 0 )
			{
				SetStatusValue(m_szRxBuf[2]);

				for(i = 0; i < sizeof(stValues)/sizeof(BILLONEP_STATUS_T); i++)
				{
					if( stValues[i].byStatus == (m_szRxBuf[2] & 0xFF) )
					{
						if(prevStatus != (m_szRxBuf[2] & 0xFF))
						{
							LOG_OUT("ActiveStatus = 0x%02X, [%s] [%s] ...", stValues[i].byStatus & 0xFF, stValues[i].szShtNm, stValues[i].pStr);
							prevStatus = (m_szRxBuf[2] & 0xFF);
						}
						break;
					}
				}

				if(m_szRxBuf[3] == 0x0C)
				{  /// error 상태

				}

				UnLocking();
				return m_szRxBuf[3] & 0xFF;
			}
			else
			{
				nRet = doEventProcess();
				if(nRet < 0)
				{
					LOG_OUT("response error = %02X, %02X, %02X ", m_szRxBuf[0] & 0xFF, m_szRxBuf[1] & 0xFF, m_szRxBuf[2] & 0xFF);
				}
			}
		}
	}

	UnLocking();

	return -1;
}

/**
 * @brief		Enable
 * @details		지폐투입 허가 
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CBillONEP::Enable(void)
{
	int		nRet;
	DWORD	dwTick;
	BYTE	sBuffer[] = "\x53\x41\x0D";		/// S,A,0x0D

	Locking();

	LOG_OUT("start..");

	nRet = SendPacket(sBuffer, 3);

	dwTick = ::GetTickCount();
	while( Util_CheckExpire(dwTick) <= 1000 )
	{
		nRet = GetPacket(m_szRxBuf);
		if(nRet > 0)
		{
			if( memcmp(m_szRxBuf, "OKa", 3) == 0 )
			{
				m_bEnable = TRUE;
				UnLocking();
				return nRet;
			}
			else
			{
				LOG_OUT("response error = %02X, %02X, %02X ", m_szRxBuf[0] & 0xFF, m_szRxBuf[1] & 0xFF, m_szRxBuf[2] & 0xFF);
			}
		}
	}

	UnLocking();

	return -1;
}

/**
 * @brief		Inhibit
 * @details		지폐투입 금지 
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CBillONEP::Inhibit(void)
{
	int		nRet;
	DWORD	dwTick;
	BYTE	sBuffer[] = "\x53\x41\x0E";		/// S, A, 0x0E

	Locking();

	LOG_OUT("start..");

	nRet = SendPacket(sBuffer, 3);

	dwTick = ::GetTickCount();
	while( Util_CheckExpire(dwTick) <= 1000 )
	{
		nRet = GetPacket(m_szRxBuf);
		if(nRet > 0)
		{
			if( memcmp(m_szRxBuf, "OKa", 3) == 0 )
			{
				m_bEnable = FALSE;

				UnLocking();
				return nRet;
			}
			else
			{
				LOG_OUT("response error = %02X, %02X, %02X ", m_szRxBuf[0] & 0xFF, m_szRxBuf[1] & 0xFF, m_szRxBuf[2] & 0xFF);
			}
		}
	}

	UnLocking();

	return -1;
}

/**
 * @brief		GetConfig
 * @details		Config 데이타 읽기
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CBillONEP::GetConfig(void)
{
	int		nRet;
	BYTE	byData;
	DWORD	dwTick;
	BYTE	sBuffer[] = "\x47\x43\x3F";		/// G, C, ?

	Locking();

	LOG_OUT("start..");

	nRet = SendPacket(sBuffer, 3);

	dwTick = ::GetTickCount();
	while( Util_CheckExpire(dwTick) <= 1000 )
	{
		nRet = GetPacket(m_szRxBuf);
		if(nRet > 0)
		{
			if( memcmp(m_szRxBuf, "gc", 2) == 0 )
			{
				byData = m_szRxBuf[2] & 0xFF;

				LOG_OUT("%s", (byData & 0x01) ? "1,000원권 입수 가능"  : "1,000원권 입수 불가" );
				LOG_OUT("%s", (byData & 0x02) ? "5,000원권 입수 가능"  : "5,000원권 입수 불가" );
				LOG_OUT("%s", (byData & 0x04) ? "10,000원권 입수 가능" : "10,000원권 입수 불가" );
				LOG_OUT("%s", (byData & 0x08) ? "50,000원권 입수 가능" : "50,000원권 입수 불가" );
				LOG_OUT("%s", (byData & 0x10) ? "지폐입수 후 자동 STACK ON" : "지폐입수 후 자동 STACK OFF" );

				UnLocking();
				return nRet;
			}
			else
			{
				LOG_OUT("response error = %02X, %02X, %02X ", m_szRxBuf[0] & 0xFF, m_szRxBuf[1] & 0xFF, m_szRxBuf[2] & 0xFF);
			}
		}
	}

	UnLocking();

	return -1;
}

/**
 * @brief		GetErrorInfo
 * @details		에러 상태 체크
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CBillONEP::GetErrorInfo(void)
{
	int		nRet;
	BYTE	byData;
	DWORD	dwTick;
	BYTE	sBuffer[] = "\x47\x45\x3F";		/// G, E, ?

	Locking();

	LOG_OUT("start..");

	nRet = SendPacket(sBuffer, 3);

	dwTick = ::GetTickCount();
	while( Util_CheckExpire(dwTick) <= 1000 )
	{
		nRet = GetPacket(m_szRxBuf);
		if(nRet > 0)
		{
			if( memcmp(m_szRxBuf, "ge", 2) == 0 )
			{
				byData = m_szRxBuf[2] & 0xFF;
				switch(byData)
				{
				case 0 : 	LOG_OUT("error_code = %d, <에러 없음>",								byData);			break;
				case 1 : 	LOG_OUT("error_code = %d, <Start Sensor관련 Error>",					byData);			break;
				case 2 : 	LOG_OUT("error_code = %d, <Shutter Sensor관련 Error>",				byData);			break;
				case 3 : 	LOG_OUT("error_code = %d, <End Sensor 관련 Error>",					byData);			break;
				case 4 : 	LOG_OUT("error_code = %d, <이송 Motor관련 Error>",					byData);			break;
				case 5 : 	LOG_OUT("error_code = %d, <Stack Motor관련 Error>",					byData);			break;
				case 9 : 	LOG_OUT("error_code = %d, <불손한 의도가 시도되어 1분간 입수금지>",	byData);			break;
				case 11: 	LOG_OUT("error_code = %d, <인식 Sensor1 관련 Error>",				byData);			break;
				case 12: 	LOG_OUT("error_code = %d, <인식 Sensor2 관련 Error>",				byData);			break;
				case 13: 	LOG_OUT("error_code = %d, <인식 Sensor3 관련 Error>",				byData);			break;
				case 14: 	LOG_OUT("error_code = %d, <인식 Sensor4 관련 Error>",				byData);			break;
				case 15: 	LOG_OUT("error_code = %d, <인식 Sensor5 관련 Error>",				byData);			break;
				case 16: 	LOG_OUT("error_code = %d, <인식 Sensor6 관련 Error>",				byData);			break;
				case 17: 	LOG_OUT("error_code = %d, <인식 Sensor7 관련 Error>",				byData);			break;
				case 18: 	LOG_OUT("error_code = %d, <인식 Sensor8 관련 Error>",				byData);			break;
				default:	LOG_OUT("error_code = %d, <unknown>",								byData);			break;
				}
				UnLocking();
				return nRet;
			}
			else
			{
				LOG_OUT("response error = %02X, %02X, %02X ", m_szRxBuf[0] & 0xFF, m_szRxBuf[1] & 0xFF, m_szRxBuf[2] & 0xFF);
			}
		}
	}

	UnLocking();

	return -1;
}

/**
 * @brief		SetConfig
 * @details		지폐인식기 설정 
 * @param		BOOL b1k		1,000원권 사용유무
 * @param		BOOL b5k		5,000원권 사용유무
 * @param		BOOL b10k		10,000원권 사용유무
 * @param		BOOL b50k		50,000원권 사용유무
 * @return		성공 : > 0, 실패 : < 0
 */
int CBillONEP::SetConfig(BOOL b1k, BOOL b5k, BOOL b10k, BOOL b50k)
{
	int		nRet;
	DWORD	dwTick;
	BYTE	byConfig = 0;
	BYTE	sBuffer[] = "\x53\x43\x00";		/// S, C, Config_value

	Locking();

	LOG_OUT("start..");

	byConfig = 0;
	if(b1k == TRUE)
	{
		byConfig |= 0x01;
	}
	if(b5k == TRUE)
	{
		byConfig |= 0x02;
	}
	if(b10k == TRUE)
	{
		byConfig |= 0x04;
	}
	if(b50k == TRUE)
	{
		byConfig |= 0x08;
	}
	byConfig |= 0x10;		/// 자동 stack

	sBuffer[2] = byConfig;

	nRet = SendPacket(sBuffer, 3);

	dwTick = ::GetTickCount();
	while( Util_CheckExpire(dwTick) <= 1000 )
	{
		nRet = GetPacket(m_szRxBuf);
		if(nRet > 0)
		{
			if( memcmp(m_szRxBuf, "OKc", 3) == 0 )
			{
				UnLocking();
				return nRet;
			}
		}
	}

	UnLocking();

	return -1;
}

/**
 * @brief		Alive
 * @details		통신 체크
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CBillONEP::Alive(void)
{
	int		nRet;
	DWORD	dwTick;
	BYTE	sBuffer[] = "\x48\x69\x3F";		/// H, i, ?

	Locking();

	LOG_OUT("start..");

	nRet = SendPacket(sBuffer, 3);

	dwTick = ::GetTickCount();
	while( Util_CheckExpire(dwTick) <= 1000 )
	{
		nRet = GetPacket(m_szRxBuf);
		if(nRet > 0)
		{
			if( memcmp(m_szRxBuf, "me!", 3) == 0 )
			{
				UnLocking();
				return nRet;
			}
			else
			{
				LOG_OUT("response error = %02X, %02X, %02X ", m_szRxBuf[0] & 0xFF, m_szRxBuf[1] & 0xFF, m_szRxBuf[2] & 0xFF);
			}
		}
	}

	UnLocking();

	return -1;
}

/**
 * @brief		GetVersion
 * @details		버젼 체크
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CBillONEP::GetVersion(void)
{
	int		nRet;
	DWORD	dwTick;
	BYTE	sBuffer[] = "\x47\x56\x3F";		/// G, V, ?

	Locking();

	LOG_OUT("start..");

	nRet = SendPacket(sBuffer, 3);

	dwTick = ::GetTickCount();
	while( Util_CheckExpire(dwTick) <= 1000 )
	{
		nRet = GetPacket(m_szRxBuf);
		if(nRet > 0)
		{
			if( memcmp(m_szRxBuf, "v", 1) == 0 )
			{
				LOG_OUT("get_version = %02X.%02X", m_szRxBuf[1] & 0xFF, m_szRxBuf[2] & 0xFF);
				UnLocking();
				return nRet;
			}
			else
			{
				LOG_OUT("response error = %02X, %02X, %02X ", m_szRxBuf[0] & 0xFF, m_szRxBuf[1] & 0xFF, m_szRxBuf[2] & 0xFF);
			}
		}
	}

	UnLocking();

	return -1;
}

/**
 * @brief		GetMoney
 * @details		지폐투입 금액 가져오기 
 * @param		int *n1k		1,000원 데이타 갯수
 * @param		int *n5k		5,000원 데이타 갯수
 * @param		int *n10k		10,000원 데이타 갯수
 * @param		int *n50k		50,000원 데이타 갯수
 * @return		성공 : > 0, 실패 : < 0
 */
int CBillONEP::GetMoney(int *n1k, int *n5k, int *n10k, int *n50k)
{
	if( (*n1k != m_n1k) && (*n5k != m_n5k) && (*n10k != m_n10k) && (*n50k != m_n50k) )
	{
		LOG_OUT("1k[%02d], 5k[%02d], 10k[%02d], total[%10d]\n", m_n1k, m_n5k, m_n10k, m_nTotalFare);
	}

	*n1k = m_n1k;
	*n5k = m_n5k;
	*n10k = m_n10k;
	*n50k = m_n50k;

	return m_nTotalFare;
}

/**
 * @brief		TotalEnd
 * @details		거래 종료 
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CBillONEP::TotalEnd(void)
{
	Locking();

	LOG_OUT("start..");

	m_n1k = m_n5k = m_n10k = m_n50k = m_nTotalFare = 0;

	UnLocking();

	return 0;
}

/**
 * @brief		StartProcess
 * @details		Start
 * @param		None
 * @return		성공 : > 0, 실패 : < 0
 */
int CBillONEP::StartProcess(int nCommIdx)
{
	int		nRet;
	DWORD dwThreadID;

	LOG_INIT();

	m_bConnected = FALSE;

	LOG_OUT("start !!!");

	EndProcess();

	nRet = m_clsComm.Open(nCommIdx, CBR_9600, 8, NOPARITY, ONESTOPBIT);
	if(nRet < 0) 
	{
		LOG_OUT("m_clsComm.Open() Failure, nRet(%d) ..", nRet);
		EndProcess();
		return -1;
	}

	m_hAccMutex = ::CreateMutex(NULL, FALSE, NULL);
	if(m_hAccMutex == NULL) 
	{
		LOG_OUT("m_clsComm.Open() Failure, nRet(%d) ..", nRet);
		EndProcess();
		return -2;
	}

	m_bConnected = TRUE;

	m_hThread = ::CreateThread(NULL, 0, RunThread, this, CREATE_SUSPENDED, &dwThreadID);
	if(NULL == m_hThread)
	{
		LOG_OUT("CreateThread() failure !![%d]", ::GetLastError());
		EndProcess();
		return -3;
	}

	::ResumeThread(m_hThread);

	return 0;
}

/**
 * @brief		EndProcess
 * @details		Start
 * @param		int nCommIdx		COM
 * @return		성공 : > 0, 실패 : < 0
 */
int CBillONEP::EndProcess(void)
{
	LOG_OUT("start !!!");
	
	m_bConnected = FALSE;

	if(NULL != m_hThread)
	{
		::WaitForSingleObject(m_hThread, 500);
		::CloseHandle(m_hThread);

		m_hThread = NULL;
	}

	m_clsComm.Close();

	if(m_hAccMutex != NULL)
	{
		CloseHandle(m_hAccMutex);
		m_hAccMutex = NULL;
	}

	return 0;
}

/**
 * @brief		RunThread
 * @details		Start
 * @param		LPVOID lParam		CBillONEP instance
 * @return		항상 : 0
 */
DWORD CBillONEP::RunThread(LPVOID lParam)
{
	int		nRet, nRetry;
	DWORD	dwTick;
	BOOL	b1k, b5k, b10k, b50k;
	CBillONEP *pBill = (CBillONEP *)lParam;

	nRetry = 0;
	b1k = b5k = b10k = b50k = TRUE;

	nRet = pBill->GetConfig();
	nRet = pBill->GetDevStatus();
	nRet = pBill->GetStatus();
	
	/// 금액 설정
	if( INI_UseCashIn1K() <= 0 )
	{
		b1k = FALSE;
	}
	if( INI_UseCashIn5K() <= 0 )
	{
		b5k = FALSE;
	}
	if( INI_UseCashIn10K() <= 0 )
	{
		b10k = FALSE;
	}
	if( INI_UseCashIn50K() <= 0 )
	{
		b50k = FALSE;
	}

	//nRet = pBill->SetConfig(TRUE, TRUE, TRUE, TRUE);
	nRet = pBill->SetConfig(b1k, b5k, b10k, b50k);

	dwTick = ::GetTickCount();
	while(pBill->m_bConnected)
	{
		Sleep(10);

		if(dwTick == 0L)
		{
			dwTick = ::GetTickCount();
		}

		if( Util_CheckExpire(dwTick) >= 500 )	
		{
			nRet = pBill->GetDevStatus();
			if(nRet < 0)
			{
				if(++nRetry >= 3)
				{
					SetCheckEventCode(EC_BILL_COMM_ERR, TRUE);
				}
			}
			else
			{
				if(nRetry > 0)
				{
					nRetry = 0;
				}
				SetCheckEventCode(EC_BILL_COMM_ERR, FALSE);
			}

			switch(nRet)
			{
			case 0x01:
			case 0x0B:
				/// enable
				if(pBill->m_bStacking == TRUE)
				{
					nRet = pBill->GetStatus();
				}
				else
				{
					if(pBill->m_bEnable == TRUE)
					{
						nRet = pBill->Enable();
					}
				}
				break;
			case 0x02:	/// 입수가능 상태
				break;
			case 0x04:	/// 인식작업중
				pBill->m_bStacking = TRUE;
				break;
			case 0x0A:	/// Stack 동작 중
				pBill->m_bStacking = TRUE;
				break;
			case 0x03: /// 반환 동작 중
				break;
			case 0x0C: /// 에러
				nRet = pBill->GetErrorInfo();
				break;
			default:
				nRet = pBill->GetStatus();
				break;
			}
			dwTick = ::GetTickCount();
		}		
	}

	return 0;
}



