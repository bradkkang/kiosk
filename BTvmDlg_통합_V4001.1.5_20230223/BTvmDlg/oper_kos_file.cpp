
#include "stdafx.h"
#include <stdio.h>
#include <string.h>

#include "MyUtil.h"
#include "MyFileUtil.h"
#include "svr_ccbus.h"
#include "event_if.h"
#include "dev_tr_main.h"
#include "dev_tr_mem.h"
#include "oper_config.h"
#include "oper_kos_file.h"
#include "File_Env_ini.h"

//----------------------------------------------------------------------------------------------------------------------
static HANDLE	m_hAccMutex = NULL;
static CString	m_strCurrentPath;
//----------------------------------------------------------------------------------------------------------------------
static OPER_FILE_LIST_T op_file_list[] =
{
	{ OPER_FILE_ID_0100					,	(CString)"oper_ccsv_0100.dat"	},
	{ OPER_FILE_ID_0101					,	(CString)"oper_ccsv_0101.dat"	},
	{ OPER_FILE_ID_0102					,	(CString)"oper_ccsv_0102.dat"	},
	{ OPER_FILE_ID_0106					,	(CString)"oper_ccsv_0106.dat"	},
	{ OPER_FILE_ID_0107					,	(CString)"oper_ccsv_0107.dat"	},
	{ OPER_FILE_ID_0108					,	(CString)"oper_ccsv_0108.dat"	},
	{ OPER_FILE_ID_0114					,	(CString)"oper_ccsv_0114.dat"	},
	{ OPER_FILE_ID_0118					,	(CString)"oper_ccsv_0118.dat"	},
	{ OPER_FILE_ID_0119					,	(CString)"oper_ccsv_0119.dat"	},
	{ OPER_FILE_ID_0120					,	(CString)"oper_ccsv_0120.dat"	},
	{ OPER_FILE_ID_0122					,	(CString)"oper_ccsv_0122.dat"	},
	{ OPER_FILE_ID_0124					,	(CString)"oper_ccsv_0124.dat"	},
	{ OPER_FILE_ID_0125					,	(CString)"oper_ccsv_0125.dat"	},
	{ OPER_FILE_ID_0126					,	(CString)"oper_ccsv_0126.dat"	},
	{ OPER_FILE_ID_0217					,	(CString)"oper_ccsv_0217.dat"	},
	{ OPER_FILE_ID_0268					,	(CString)"oper_ccsv_0268.dat"	},

	{ OPER_FILE_ID_0127					,	(CString)"oper_ccsv_0127.dat"	},
	{ OPER_FILE_ID_0128					,	(CString)"oper_ccsv_0128.dat"	},
	{ OPER_FILE_ID_0129					,	(CString)"oper_ccsv_0129.dat"	},
	{ OPER_FILE_ID_0200					,	(CString)"oper_ccsv_0200.dat"	},
	{ OPER_FILE_ID_0201					,	(CString)"oper_ccsv_0201.dat"	},
	{ OPER_FILE_ID_0208					,	(CString)"oper_ccsv_0208.dat"	},
	{ OPER_FILE_ID_0209					,	(CString)"oper_ccsv_0209.dat"	},
	{ OPER_FILE_ID_0130					,	(CString)"oper_ccsv_0130.dat"	},	// 배차 리스트
	{ OPER_FILE_ID_0135					,	(CString)"oper_ccsv_0135.dat"	},	// 발행 내역 조회
	{ OPER_FILE_ID_0157					,	(CString)"oper_ccsv_0157.dat"	},	// 재발행 내역 조회
	{ OPER_FILE_ID_0155					,	(CString)"oper_ccsv_0155.dat"	},	// 환불 내역 조회
	{ OPER_FILE_ID_0999					,	(CString)"CodeConv.xml"			},	// CodeConv.xml

	{ OPER_FILE_ID_CONFIG				,	(CString)"oper_config.dat"		},

	{ OPER_FILE_ID_KO_CM_READMSG		,	(CString)"oper_ko_1001.dat"		},	// (고속버스-코버스) 메시지 코드 조회	
	{ OPER_FILE_ID_KO_CM_READNTC		,	(CString)"oper_ko_1002.dat"		},	// (고속버스-코버스) RTC
	{ OPER_FILE_ID_KO_TK_AUTHCMPT		,	(CString)"oper_ko_1003.dat"		},	// (고속버스-코버스) 접속 컴퓨터 인증 & 터미널 코드 확인(코버스는 skip)
	{ OPER_FILE_ID_KO_CM_READCMNCD		,	(CString)"oper_ko_1004.dat"		},	// (고속버스-코버스) 공통코드 조회
	{ OPER_FILE_ID_KO_TK_READTCKPRTG	,	(CString)"oper_ko_1005.dat"		},	// (고속버스-코버스) 승차권 인쇄정보 조회
	{ OPER_FILE_ID_KO_TK_READOWNRTRML	,	(CString)"oper_ko_1006.dat"		},	// (고속버스-코버스) 자기터미널 정보 조회 (코버스는 skip)
	{ OPER_FILE_ID_KO_CM_READTRML		,	(CString)"oper_ko_1007.dat"		},	// (고속버스-코버스) 터미널 조회(전국터미널)
	{ OPER_FILE_ID_KO_CM_READTRMLINF	,	(CString)"oper_ko_1008.dat"		},	// (고속버스-코버스) 터미널 정보 조회
	{ OPER_FILE_ID_KO_CM_READTRMLSTUP	,	(CString)"oper_ko_1009.dat"		},	// (고속버스-코버스) 터미널환경설정보
	{ OPER_FILE_ID_KO_TM_EWNDINFO		,	(CString)"oper_ko_1010.dat"		},	// (고속버스-코버스) 창구정보 조회
	{ OPER_FILE_ID_KO_CM_READRTRPTRML	,	(CString)"oper_ko_1011.dat"		},	// (고속버스-코버스) 왕복터미널 정보 조회
	{ OPER_FILE_ID_KO_CM_READTCKKND		,	(CString)"oper_ko_1012.dat"		},	// (고속버스-코버스) 승차권 정보 조회 (코버스는 skip)
	{ OPER_FILE_ID_KO_CM_MNGCACM		,	(CString)"oper_ko_1013.dat"		},	// (고속버스-코버스) 버스 정보 조회
	{ OPER_FILE_ID_KO_CM_READROTINF		,	(CString)"oper_ko_1014.dat"		},	// (고속버스-코버스) 노선 정보 조회 (코버스는 skip)
	{ OPER_FILE_ID_KO_CM_RDHMINQR		,	(CString)"oper_ko_1005.dat"		},	// (고속버스-코버스) 승차홈 조회
	{ OPER_FILE_ID_KO_CM_READTHRUTRML	,	(CString)"oper_ko_1016.dat"		},	// (고속버스-코버스) 경유지정보 조회
	{ OPER_FILE_ID_KO_TK_INQPUBPT		,	(CString)"oper_ko_1017.dat"		},	// (고속버스-코버스) 발권내역 조회
	{ OPER_FILE_ID_KO_RD_CORPINF		,	(CString)"oper_ko_corp.txt"		},	// (고속버스-코버스) 로컬 운송회사 정보 조회

	{ OPER_FILE_ID_EZ_CM_READMSG		,	(CString)"oper_texp_2001.dat"	},	// (고속버스-티머니고속) 메시지 코드 조회	
	{ OPER_FILE_ID_EZ_CM_READNTC		,	(CString)"oper_texp_2002.dat"	},	// (고속버스-티머니고속) 공통코드 상세조회
	{ OPER_FILE_ID_EZ_TK_AUTHCMPT		,	(CString)"oper_texp_2003.dat"	},	// (고속버스-티머니고속) 접속 컴퓨터 인증 & 터미널 코드 확인
	{ OPER_FILE_ID_EZ_CM_READCMNCD		,	(CString)"oper_texp_2004.dat"	},	// (고속버스-티머니고속) 공통코드 조회
	{ OPER_FILE_ID_EZ_TK_READTCKPRTG	,	(CString)"oper_texp_2005.dat"	},	// (고속버스-티머니고속) 승차권 인쇄정보 조회
	{ OPER_FILE_ID_EZ_TK_READOWNRTRML	,	(CString)"oper_texp_2006.dat"	},	// (고속버스-티머니고속) 자기터미널 정보 조회 
	{ OPER_FILE_ID_EZ_CM_READTRML		,	(CString)"oper_texp_2007.dat"	},	// (고속버스-티머니고속) 터미널 조회(전국터미널)
	{ OPER_FILE_ID_EZ_CM_READTRMLINF	,	(CString)"oper_texp_2008.dat"	},	// (고속버스-티머니고속) 터미널 정보 조회
	{ OPER_FILE_ID_EZ_CM_READTRMLSTUP	,	(CString)"oper_texp_2009.dat"	},	// (고속버스-티머니고속) 터미널환경설정보
	{ OPER_FILE_ID_EZ_MG_READWND		,	(CString)"oper_texp_2010.dat"	},	// (고속버스-티머니고속) 창구정보 조회
	{ OPER_FILE_ID_EZ_CM_READRTRPTRML	,	(CString)"oper_texp_2011.dat"	},	// (고속버스-티머니고속) 왕복터미널 정보 조회
	{ OPER_FILE_ID_EZ_CM_READTCKKND		,	(CString)"oper_texp_2012.dat"	},	// (고속버스-티머니고속) 승차권 정보 조회
	{ OPER_FILE_ID_EZ_CM_MNGCACM		,	(CString)"oper_texp_2013.dat"	},	// (고속버스-티머니고속) 버스 정보 조회
	{ OPER_FILE_ID_EZ_CM_READROTINF		,	(CString)"oper_texp_2014.dat"	},	// (고속버스-티머니고속) 노선 정보 조회
	{ OPER_FILE_ID_EZ_CM_RDHMINQR		,	(CString)"oper_texp_2015.dat"	},	// (고속버스-티머니고속) 방면 정보 조회
	{ OPER_FILE_ID_EZ_CM_READRYRT		,	(CString)"oper_texp_2016.dat"	},	// (고속버스-티머니고속) 환불율 정보 조회

	{ OPER_FILE_ID_EZ_BUS_CLS			,	(CString)"oper_texp_2017.dat"	},	// (고속버스-티머니고속) 버스등급

	{ OPER_FILE_ID_EZ_TK_INQPUBPT		,	(CString)"oper_texp_2018.dat"	},	// (고속버스-티머니고속) 발권내역 조회
	{ OPER_FILE_ID_EZ_TK_INQREPUBPT		,	(CString)"oper_texp_2019.dat"	},	// (고속버스-티머니고속) 재발행내역 조회
	{ OPER_FILE_ID_EZ_TK_INQCANRYPT		,	(CString)"oper_texp_2019.dat"	},	// (고속버스-티머니고속) 환불내역 조회
	
};

//----------------------------------------------------------------------------------------------------------------------

static TCK_KIND_LIST_T op_tck_list[] =
{
	{ SVR_DVS_CCBUS		,	CCS_IDX_TCK_IU10	,	"IU10",		"대학생10"	  },	
	{ SVR_DVS_CCBUS		,	CCS_IDX_TCK_IV50	,	"IV50",		"보훈50"		  },	
	{ SVR_DVS_CCBUS		,	CCS_IDX_TCK_IS00	,	"IS00",		"군후불"		  },	
	{ SVR_DVS_CCBUS		,	CCS_IDX_TCK_IG00	,	"IG00",		"어른"		  },	
	{ SVR_DVS_CCBUS		,	CCS_IDX_TCK_IC50	,	"IC50",		"아동50"		  },	
	{ SVR_DVS_CCBUS		,	CCS_IDX_TCK_IV30	,	"IV30",		"보훈30"		  },	
	{ SVR_DVS_CCBUS		,	CCS_IDX_TCK_IM20	,	"IM20",		"중고20"		  },	
	{ SVR_DVS_CCBUS		,	CCS_IDX_TCK_IF00	,	"IF00",		"무임"		  },	
	{ SVR_DVS_CCBUS		,	CCS_IDX_TCK_IV70	,	"IV70",		"보훈70"		  },	
	{ SVR_DVS_CCBUS		,	CCS_IDX_TCK_IP20	,	"IP20",		"봉사20%"	  },	
	{ SVR_DVS_CCBUS		,	CCS_IDX_TCK_MT02	,	"MT02",		"월권할증"	  },	
	{ SVR_DVS_CCBUS		,	CCS_IDX_TCK_MT00	,	"MT00",		"월권"		  },	

	/// 2020.03.17 티켓종류 추가
	{ SVR_DVS_CCBUS		,	CCS_IDX_TCK_IR00	,	"IR00",		"어른(교통카드)"},	
	{ SVR_DVS_CCBUS		,	CCS_IDX_TCK_IR70	,	"IR70",		"상주직원RF"	  },	
	{ SVR_DVS_CCBUS		,	CCS_IDX_TCK_IE50	,	"IE50",		"상주직원"	  },	
	/// ~2020.03.17 티켓종류 추가

	// 20221130 ADD~
	{ SVR_DVS_CCBUS		,	CCS_IDX_TCK_IM10	,	"IM10",		"중고10"		  },	
	{ SVR_DVS_CCBUS		,	CCS_IDX_TCK_IM30	,	"IM30",		"중고30"		  },	
	{ SVR_DVS_CCBUS		,	CCS_IDX_TCK_IM50	,	"IM50",		"중고50"		  },	
	{ SVR_DVS_CCBUS		,	CCS_IDX_TCK_IS20	,	"IS20",		"군후20"		  },	
	// 20221130 ~ADD

	// 12
	{ SVR_DVS_KOBUS		,	KOBUS_IDX_TCK_ADULT			,	 "1",		"일반"		  },	
	{ SVR_DVS_KOBUS		,	KOBUS_IDX_TCK_CHILD			,	 "2",		"초등생"		  },	
	{ SVR_DVS_KOBUS		,	KOBUS_IDX_TCK_TRIP_30		,	 "3",		"보훈30"		  },	
	{ SVR_DVS_KOBUS		,	KOBUS_IDX_TCK_TRIP_50		,	 "4",		"보훈50"		  },	
	{ SVR_DVS_KOBUS		,	KOBUS_IDX_TCK_TRIP_70		,	 "0",		"보훈70"		  },	
	{ SVR_DVS_KOBUS		,	KOBUS_IDX_TCK_FREE			,	 "6",		"경로"		  },	
	{ SVR_DVS_KOBUS		,	KOBUS_IDX_TCK_POSTPAY		,	 "5",		"후불권"		  },	
	{ SVR_DVS_KOBUS		,	KOBUS_IDX_TCK_SOLDIER		,	 "7",		"군인20"		  },	
	{ SVR_DVS_KOBUS		,	KOBUS_IDX_TCK_POSTPAY_20	,	 "8",		"후불20"		  },	
	{ SVR_DVS_KOBUS		,	KOBUS_IDX_TCK_STUDENT_DISC	,	 "9",		"학생할인"	  },	
	{ SVR_DVS_KOBUS		,	KOBUS_IDX_TCK_TEEN			,	 "f",		"중고생"		  },	
	{ SVR_DVS_KOBUS		,	KOBUS_IDX_TCK_FOREIGNER		,	 "a",		"할인권"		  },	
	{ SVR_DVS_KOBUS		,	KOBUS_IDX_TCK_FREEPASS		,	 "b",		"프리패스"	  },	
	{ SVR_DVS_KOBUS		,	KOBUS_IDX_TCK_SEASON		,	 "n",		"정기권"		  },	

	// 티머니고속
	{ SVR_DVS_TMEXP		,	TMEXP_IDX_TCK_FREE		,	 "0",		"무임"		  },	
	{ SVR_DVS_TMEXP		,	TMEXP_IDX_TCK_ADULT		,	 "1",		"어른"		  },	
	{ SVR_DVS_TMEXP		,	TMEXP_IDX_TCK_CHILD		,	 "2",		"초등생"		  },	
	{ SVR_DVS_TMEXP		,	TMEXP_IDX_TCK_TRIP_30	,	 "3",		"보훈30"		  },	
	{ SVR_DVS_TMEXP		,	TMEXP_IDX_TCK_TRIP_50	,	 "4",		"보훈50"		  },	
	{ SVR_DVS_TMEXP		,	TMEXP_IDX_TCK_SR_20		,	 "5",		"군인20"		  },	
	{ SVR_DVS_TMEXP		,	TMEXP_IDX_TCK_SH		,	 "6",		"군후"		  },	
	{ SVR_DVS_TMEXP		,	TMEXP_IDX_TCK_SH20		,	 "7",		"군후20"		  },	
	{ SVR_DVS_TMEXP		,	TMEXP_IDX_TCK_UNIV		,	 "8",		"대학생"		  },	
	{ SVR_DVS_TMEXP		,	TMEXP_IDX_TCK_STDU		,	 "9",		"중고생"		  },	
	{ SVR_DVS_TMEXP		,	TMEXP_IDX_TCK_FPASS		,	 "a",		"프리패스"	  },	
	{ SVR_DVS_TMEXP		,	TMEXP_IDX_TCK_FWEEK		,	 "b",		"프리주말"	  },	
	{ SVR_DVS_TMEXP		,	TMEXP_IDX_TCK_HANDI		,	 "d",		"장애인"		  },	
	{ SVR_DVS_TMEXP		,	TMEXP_IDX_TCK_DISC_20	,	 "f",		"할인20"		  },	
	{ SVR_DVS_TMEXP		,	TMEXP_IDX_TCK_STORE_1	,	 "g",		"정액권(우등)" },	
	{ SVR_DVS_TMEXP		,	TMEXP_IDX_TCK_STORE_2	,	 "h",		"정액권(일반)" },	
	{ SVR_DVS_TMEXP		,	TMEXP_IDX_TCK_SEASON_1	,	 "k",		"정기권(우등)" },	
	{ SVR_DVS_TMEXP		,	TMEXP_IDX_TCK_SEASON_2	,	 "l",		"정기권(일반)" },	
	{ SVR_DVS_TMEXP		,	TMEXP_IDX_TCK_SEASON	,	 "n",		"정기권" },	
	{ SVR_DVS_TMEXP		,	TMEXP_IDX_TCK_OLD		,	 "o",		"경로" },	

};

//----------------------------------------------------------------------------------------------------------------------

/**
 * @brief		Locking
 * @details		IPC Lock
 * @param		None
 * @return		항상 : 0
 */
static int Locking(void)
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
static int UnLocking(void)
{
	if(m_hAccMutex != NULL) 
	{
		::ReleaseMutex(m_hAccMutex);
	}

	return 0;	
}

/**
 * @brief		FindFileID
 * @details		File ID 찾기
 * @param		int nID		File ID
 * @return		성공 >= 0, 실패 < 0
 */
static int FindFileID(int nID)
{
	int i;

	for(i = 0; i < sizeof(op_file_list) / sizeof(OPER_FILE_LIST_T); i++)
	{
		if(nID == op_file_list[i].nID)
		{
			return i;
		}
	}

	return -1;
}

/**
 * @brief		OperInitFile
 * @details		초기화
 * @param		None
 * @return		항상 : 0
 */
int OperInitFile(void)
{
	try
	{
		m_hAccMutex = ::CreateMutex(NULL, FALSE, NULL);
		if(m_hAccMutex == NULL) 
		{
			return -1;
		}

		{
			CString strPath;
			char Buffer[256];

			USES_CONVERSION;

			::ZeroMemory(Buffer, sizeof(Buffer));
			Util_GetModulePath(Buffer);

			m_strCurrentPath = (CString) Buffer;

			strPath = m_strCurrentPath + _T("\\files");
			if( MyAccessFile(strPath) < 0 )
			{
				MyCreateDirectory(strPath);
			}

			strPath = m_strCurrentPath + _T("\\Temp");
			if( MyAccessFile(strPath) < 0 )
			{
				MyCreateDirectory(strPath);
			}

			strPath = m_strCurrentPath + _T("\\image_scan");
			if( MyAccessFile(strPath) < 0 )
			{
				MyCreateDirectory(strPath);
			}
		}
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return 0;
}

/**
 * @brief		OperTermFile
 * @details		종료
 * @param		None
 * @return		항상 : 0
 */
void OperTermFile(void)
{
	try
	{
		if(m_hAccMutex != NULL)
		{
			CloseHandle(m_hAccMutex);
			m_hAccMutex = NULL;
		}
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}
}

/**
 * @brief		OperGetFileName
 * @details		get file full name
 * @param		None
 * @return		항상 : 0
 */
int OperGetFileName(int nID, CString& strFileName)
{
	int nIndex;

	try
	{
		nIndex = FindFileID(nID);
		if(nIndex < 0)
		{
			return -1;
		}

		strFileName = m_strCurrentPath + _T("\\files\\") + op_file_list[nIndex].strFileName;
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return 0;
}

/**
 * @brief		OperReadFile
 * @details		Init
 * @param		None
 * @return		항상 : 0
 */
int OperGetFileInfo(int nID, char *pData, int *nFileSize)
{
	int			nIndex = 0;
	CString		strFullName;

	try
	{
		nIndex = FindFileID(nID);
		if(nIndex < 0)
		{
			return -1;
		}

		strFullName = m_strCurrentPath + _T("\\files\\") + op_file_list[nIndex].strFileName;

		*nFileSize = MyGetFileSize(strFullName);
		sprintf(pData, "%s",  LPSTR(LPCTSTR(strFullName)));
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return 0;
}

/**
 * @brief		OperReadFile
 * @details		Init
 * @param		None
 * @return		항상 : 0
 */
int OperRecordReadFile(int nID, char *pData, int nRecodLen)
{
	int i, nRet, nOffset, nCount, nIndex;
	CString strFullName;
	HANDLE hFile;

	try
	{
		Locking();

		i = nRet = nOffset = nCount = nIndex = 0;

		nIndex = FindFileID(nID);
		if(nIndex < 0)
		{
			UnLocking();
			return -1;
		}

		strFullName = m_strCurrentPath + _T("\\files\\") + op_file_list[nIndex].strFileName;
		{
			DWORD dwFileSize = MyGetFileSize(strFullName);

			nCount = dwFileSize / nRecodLen;

			hFile = MyOpenFile(strFullName, FALSE);
			if(hFile == INVALID_HANDLE_VALUE)
			{
				TR_LOG_OUT("MyOpenFile() error");
				UnLocking();
				return -2;
			}

			if(nCount > 0)
			{
				nOffset = 0;
				for(i = 0; i < nCount; i++)
				{
					nRet = MyReadFile(hFile, &pData[nOffset], nRecodLen);
					if(nRet <= 0)
					{
						TR_LOG_OUT("MyReadFile() #2 error, index(%d)", i);
						MyCloseFile(hFile);
						UnLocking();
						return -3;
					} 
					nOffset += nRecodLen;
				}
			}
			else
			{
				MyCloseFile(hFile);
				UnLocking();
				return -4;
			}

			MyCloseFile(hFile);
		}
		UnLocking();
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return 0;
}

/**
 * @brief		OperReadFile
 * @details		Init
 * @param		None
 * @return		항상 : 0
 */
int OperReadFile(int nID, char *pData, int nDataLen)
{
	int nRet, nIndex;
	CString strFullName;
	HANDLE hFile;

	try
	{
		Locking();

		nIndex = FindFileID(nID);
		if(nIndex < 0)
		{
			UnLocking();
			return -1;
		}

		strFullName = m_strCurrentPath + _T("\\files\\") + op_file_list[nIndex].strFileName;
		{
			hFile = MyOpenFile(strFullName, FALSE);
			if(hFile == INVALID_HANDLE_VALUE)
			{
				TR_LOG_OUT("MyOpenFile() error");
				UnLocking();
				return -2;
			}

			nRet = MyReadFile(hFile, pData, nDataLen);
			if(nRet <= 0)
			{
				TR_LOG_OUT("MyReadFile() #2 error, nRet(%d)", nRet);
				MyCloseFile(hFile);
				UnLocking();
				return -3;
			} 

			MyCloseFile(hFile);
		}
		UnLocking();
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return 0;
}

/**
 * @brief		OperWriteFile
 * @details		Write Data
 * @param		None
 * @return		항상 : 0
 */
int OperWriteFile(int nID, char *pData, int nDataLen)
{
#define MAX_OP_BUFF		(1024 * 5)
	int nRet, nIndex;
	int nWrite, nCount, nRest, i;
	CString strFullName;
	HANDLE hFile;

	try
	{
		Locking();

		nIndex = FindFileID(nID);
		if(nIndex < 0)
		{
			UnLocking();
			return -1;
		}

		strFullName = m_strCurrentPath + _T("\\files\\") + op_file_list[nIndex].strFileName;
		{
			hFile = MyOpenFile(strFullName, TRUE);
			if(hFile == INVALID_HANDLE_VALUE)
			{
				TR_LOG_OUT("MyOpenFile() error");
				UnLocking();
				return -2;
			}

			nCount = nDataLen / MAX_OP_BUFF;
			nRest = nDataLen % MAX_OP_BUFF;
			if(nRest > 0)
			{
				nCount++;
			}

			for(i = 0; i < nCount; i++)
			{
				if( i == (nCount - 1) )
				{
					nWrite = nRest;
				}
				else
				{
					nWrite = MAX_OP_BUFF;
				}

				//nRet = MyWriteFile(hFile, pData, nDataLen);
				nRet = MyWriteFile(hFile, pData, nWrite);
				if(nRet <= 0)
				{
					TR_LOG_OUT("MyWriteFile() error");
					MyCloseFile(hFile);
					UnLocking();
					return -3;
				}
			}

			MyCloseFile(hFile);
		}

		UnLocking();
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return 1;
}

// 버스운수사명 찾기
int FindBusCacmName(int nSvrKind, char *bus_cacm_cd, char *retName, char *retBizNo, char *retTelNo)
{
	int nRet, nIndex, i, nCount, nFind;
	CString strFullName;
	HANDLE hFile;

	try
	{
		if( nSvrKind == SVR_DVS_CCBUS )
		{	/// 시외버스 
			rtk_readbuscacm_t Info;
			rtk_readbuscacm_list_t ListInfo;

			nIndex = FindFileID(OPER_FILE_ID_0106);
			if(nIndex < 0)
			{
				return -1;
			}

			::ZeroMemory(&Info, sizeof(rtk_readbuscacm_t));
			nFind = -1;

			strFullName = m_strCurrentPath + _T("\\files\\") + op_file_list[nIndex].strFileName;
			{
				hFile = MyOpenFile(strFullName, FALSE);
				if(hFile == INVALID_HANDLE_VALUE)
				{
					TR_LOG_OUT("MyOpenFile() error");
					return -2;
				}

				nRet = MyReadFile(hFile, Info.rsp_cd, sizeof(Info.rsp_cd));
				nRet = MyReadFile(hFile, Info.bus_cacm_cd_num, sizeof(Info.bus_cacm_cd_num));

				nCount = *(int *) Info.bus_cacm_cd_num;

				for(i = 0; i < nCount; i++)
				{
					::ZeroMemory(&ListInfo, sizeof(rtk_readbuscacm_list_t));

					nRet = MyReadFile(hFile, &ListInfo, sizeof(rtk_readbuscacm_list_t));
					if(nRet <= 0)
					{
						break;
					}

					if( memcmp(bus_cacm_cd, ListInfo.bus_cacm_cd, sizeof(ListInfo.bus_cacm_cd) - 1) == 0 )
					{
						//TR_LOG_OUT("운수사 코드 = [%s], [%s]..", bus_cacm_cd, ListInfo.bus_cacm_cd);
						//TR_LOG_OUT("운수사 정보 - 이름[%s], 전번[%s]..", ListInfo.bus_cacm_nm, ListInfo.bus_cacm_tel_no);
						sprintf(retName, "%s", ListInfo.bus_cacm_nm);
						sprintf(retTelNo, "%s", ListInfo.bus_cacm_tel_no);
						nFind = i;
						break;
					}

				}
				MyCloseFile(hFile);
			}
		}
		else if( nSvrKind == SVR_DVS_KOBUS )
		{	/// 코버스
			rtk_cm_cacminqr_t	Info;
			rtk_cm_cacminqr_list_t ListInfo;

			nIndex = FindFileID(OPER_FILE_ID_KO_CM_MNGCACM);
			if(nIndex < 0)
			{
				return -1;
			}

			::ZeroMemory(&Info, sizeof(rtk_cm_cacminqr_t));
			nFind = -1;

			strFullName = m_strCurrentPath + _T("\\files\\") + op_file_list[nIndex].strFileName;
			{
				hFile = MyOpenFile(strFullName, FALSE);
				if(hFile == INVALID_HANDLE_VALUE)
				{
					TR_LOG_OUT("MyOpenFile() error");
					return -2;
				}

				nRet = MyReadFile(hFile, Info.msg_cd, sizeof(Info.msg_cd));
				nRet = MyReadFile(hFile, Info.msg_dtl_ctt, sizeof(Info.msg_dtl_ctt));
				nRet = MyReadFile(hFile, Info.rec_ncnt1, sizeof(Info.rec_ncnt1));

				nCount = *(int *) Info.rec_ncnt1;

				for(i = 0; i < nCount; i++)
				{
					::ZeroMemory(&ListInfo, sizeof(rtk_cm_cacminqr_list_t));

					nRet = MyReadFile(hFile, &ListInfo, sizeof(rtk_cm_cacminqr_list_t));
					if(nRet <= 0)
					{
						break;
					}

					if( memcmp(bus_cacm_cd, ListInfo.cacm_cd, sizeof(ListInfo.cacm_cd) - 1) == 0 )
					{
						sprintf(retName, "%s", ListInfo.ptrg_prin_nm);
						sprintf(retBizNo, "%.*s-%.*s-%.*s", 3, &ListInfo.bizr_no[0], 2, &ListInfo.bizr_no[3], 5, &ListInfo.bizr_no[5]);
						nFind = i;
						break;
					}

				}
				MyCloseFile(hFile);
			}
		}
		else if( nSvrKind == SVR_DVS_TMEXP )
		{	/// 티머니고속 - 운수사
			rtk_tm_cmm_t			Info;
			rtk_tm_mngcacm_list_t	ListInfo;

			nIndex = FindFileID(OPER_FILE_ID_EZ_CM_MNGCACM);
			if(nIndex < 0)
			{
				return -1;
			}

			::ZeroMemory(&Info, sizeof(rtk_tm_cmm_t));
			nFind = -1;

			strFullName = m_strCurrentPath + _T("\\files\\") + op_file_list[nIndex].strFileName;
			{
				hFile = MyOpenFile(strFullName, FALSE);
				if(hFile == INVALID_HANDLE_VALUE)
				{
					TR_LOG_OUT("MyOpenFile() error");
					return -2;
				}

				nRet = MyReadFile(hFile, Info.rsp_cd, sizeof(Info.rsp_cd));
				nRet = MyReadFile(hFile, Info.rec_num, sizeof(Info.rec_num));

				nCount = *(int *) Info.rec_num;

				for(i = 0; i < nCount; i++)
				{
					::ZeroMemory(&ListInfo, sizeof(rtk_tm_mngcacm_list_t));

					nRet = MyReadFile(hFile, &ListInfo, sizeof(rtk_tm_mngcacm_list_t));
					if(nRet <= 0)
					{
						break;
					}


					if( memcmp(bus_cacm_cd, ListInfo.cacm_cd, strlen(ListInfo.cacm_cd)) == 0 )
					{
						TR_LOG_OUT("#1. 티머니고속 - 운수사 정보(%d), nm(%s), bizr_no(%s)..", i, ListInfo.ptrg_prin_nm, ListInfo.bizr_no);

						sprintf(retName, "%s", ListInfo.ptrg_prin_nm);
						TR_LOG_OUT("#2. 티머니고속(%d) - retName(%s)..", i, retName);
						if(strlen(ListInfo.bizr_no) > 10)
						{
							TR_LOG_OUT("#3. 티머니고속 - 운수사 정보(%d), nm(%s), bizr_no(%s)..", i, ListInfo.ptrg_prin_nm, ListInfo.bizr_no);
							sprintf(retBizNo, "%.*s-%.*s-%.*s", 
								3, &ListInfo.bizr_no[0], 
								2, &ListInfo.bizr_no[3], 
								5, &ListInfo.bizr_no[5]);
						}
						else
						{
							TR_LOG_OUT("#4. 티머니고속 - 운수사 정보(%d), nm(%s), bizr_no(%s)..", i, ListInfo.ptrg_prin_nm, ListInfo.bizr_no);
							if(retBizNo != NULL)
							{
								sprintf(retBizNo, "%s", ListInfo.bizr_no);
							}
						}
						//TR_LOG_OUT("#5. 티머니고속(%d) - retBizNo(%s)..", i, retBizNo);
						nFind = i;
						break;
					}
				}
				MyCloseFile(hFile);
			}
		}

	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nFind;
}

/**
 * @brief		FindBusClsName
 * @details		버스등급명 찾기
 * @param		int nSvrKind			서버종류
 * @param		char *bus_cls_cd		버스 등급 코드
 * @param		char *retName			버스 등급 이름
 * @return		성공 : >= 0, 실패 : < 0
 */
int FindBusClsName(int nSvrKind, char *bus_cls_cd, char *retName)
{
	int nRet, nIndex, i, nCount, nFind;
	CString strFullName;
	HANDLE hFile;

	nRet = nIndex = i = nCount = nFind = 0;
	try
	{
		nFind = -1;

		if( nSvrKind == SVR_DVS_CCBUS )
		{	/// 시외버스
			rtk_readbuscls_t Info;
			rtk_readbuscls_list_t ListInfo;

			nIndex = FindFileID(OPER_FILE_ID_0107);
			if(nIndex < 0)
			{
				return -1;
			}

			::ZeroMemory(&Info, sizeof(rtk_readbuscls_t));

			strFullName = m_strCurrentPath + _T("\\files\\") + op_file_list[nIndex].strFileName;
			{
				hFile = MyOpenFile(strFullName, FALSE);
				if(hFile == INVALID_HANDLE_VALUE)
				{
					TR_LOG_OUT("MyOpenFile() error");
					return -2;
				}

				nRet = MyReadFile(hFile, Info.rsp_cd, sizeof(Info.rsp_cd));
				nRet = MyReadFile(hFile, Info.bus_cls_cd_num, sizeof(Info.bus_cls_cd_num));

				nCount = *(int *) Info.bus_cls_cd_num;

				for(i = 0; i < nCount; i++)
				{
					::ZeroMemory(&ListInfo, sizeof(rtk_readbuscls_list_t));

					nRet = MyReadFile(hFile, &ListInfo, sizeof(rtk_readbuscls_list_t));
					if(nRet <= 0)
					{
						break;
					}

					if( memcmp(bus_cls_cd, ListInfo.bus_cls_cd, sizeof(ListInfo.bus_cls_cd) - 1) == 0 )
					{
						sprintf(retName, "%s", ListInfo.bus_cls_nm);
						TRACE("버스등급명 = %s \n", retName);
						nFind = i;
						break;
					}

				}
				MyCloseFile(hFile);
			}
		}
		else if( nSvrKind == SVR_DVS_KOBUS )
		{	/// 코버스
			KOBUS_BUS_CLS_DT koList[] = 
			{
				{ "1", "우등"			},
				{ "2", "고속"			},
				{ "3", "심야우등"		},
				{ "4", "심야고속"		},
				{ "5", "일반"			},
				{ "6", "심야일반"		},
				{ "7", "프리미엄"		},
				{ "8", "심야프리미엄"		},
			};

			for(i = 0; i < sizeof(koList) / sizeof(KOBUS_BUS_CLS_DT); i++)
			{
				if( memcmp(bus_cls_cd, koList[i].bus_cls_cd, strlen(bus_cls_cd)) == 0 )
				{
					sprintf(retName, "%s", koList[i].bus_cls_nm);
					TRACE("버스등급명 = %s \n", retName);
					nFind = i;
					break;
				}
			}
		}
		else if( nSvrKind == SVR_DVS_TMEXP )
		{
			rtk_tm_readcmncd_list_t ListInfo;

			nIndex = FindFileID(OPER_FILE_ID_EZ_BUS_CLS);
			if(nIndex < 0)
			{
				return -1;
			}

			strFullName = m_strCurrentPath + _T("\\files\\") + op_file_list[nIndex].strFileName;
			{
				hFile = MyOpenFile(strFullName, FALSE);
				if(hFile == INVALID_HANDLE_VALUE)
				{
					TR_LOG_OUT("MyOpenFile() error");
					return -2;
				}

				i = 0;
				while( 1 ) 
				{
					::ZeroMemory(&ListInfo, sizeof(rtk_tm_readcmncd_list_t));

					nRet = MyReadFile(hFile, &ListInfo, sizeof(rtk_tm_readcmncd_list_t));
					if(nRet <= 0)
					{
						break;
					}
					
					//TR_LOG_OUT("티머니고속 - (%d) 버스등급명 비교 데이타 = [%s][%s] \n", i, bus_cls_cd, ListInfo.cmn_cd_val);

					if( (memcmp("C024", ListInfo.cmn_cd, 4) == 0) && 
						(bus_cls_cd[0] == ListInfo.cmn_cd_val[0]) )
					{
						sprintf(retName, "%s", ListInfo.cd_val_nm);
						TR_LOG_OUT("티머니고속 - 버스등급명 = %s \n", retName, bus_cls_cd);
						nFind = i;
						break;
					}
					i++;
				}
				MyCloseFile(hFile);
			}
		}

	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nFind;
}

/** 
 * @brief		FindBusClsShctName 추가_20201028
 * @details		짧은버스등급명 찾기
 * @param		int nSvrKind			서버종류
 * @param		char *bus_cls_shct_nm	버스 등급 코드
 * @param		char *retName			버스 등급 이름
 * @return		성공 : >= 0, 실패 : < 0
 */
int FindBusClsShctName(int nSvrKind, char *bus_cls_cd, char *retName)
{
	int nRet, nIndex, i, nCount, nFind;
	CString strFullName;
	HANDLE hFile;

	nRet = nIndex = i = nCount = nFind = 0;

	try
	{
		nFind = -1;

		if( nSvrKind == SVR_DVS_CCBUS )
		{	/// 시외버스
			rtk_readbuscls_t Info;
			rtk_readbuscls_list_t ListInfo;

			nIndex = FindFileID(OPER_FILE_ID_0107);
			if(nIndex < 0)
			{
				return -1;
			}

			::ZeroMemory(&Info, sizeof(rtk_readbuscls_t));

			strFullName = m_strCurrentPath + _T("\\files\\") + op_file_list[nIndex].strFileName;
			{
				hFile = MyOpenFile(strFullName, FALSE);
				if(hFile == INVALID_HANDLE_VALUE)
				{
					TR_LOG_OUT("MyOpenFile() error");
					return -2;
				}

				nRet = MyReadFile(hFile, Info.rsp_cd, sizeof(Info.rsp_cd));
				nRet = MyReadFile(hFile, Info.bus_cls_cd_num, sizeof(Info.bus_cls_cd_num));

				nCount = *(int *) Info.bus_cls_cd_num;

				for(i = 0; i < nCount; i++)
				{
					::ZeroMemory(&ListInfo, sizeof(rtk_readbuscls_list_t));

					nRet = MyReadFile(hFile, &ListInfo, sizeof(rtk_readbuscls_list_t));
					if(nRet <= 0)
					{
						break;
					}

					if( memcmp(bus_cls_cd, ListInfo.bus_cls_cd, sizeof(ListInfo.bus_cls_cd) - 1) == 0 )
					{
						sprintf(retName, "%s", ListInfo.bus_cls_shct_nm);
						TRACE("버스등급명 = %s \n", retName);
						nFind = i;
						break;
					}

				}
				MyCloseFile(hFile);
			}
		}
		else if( nSvrKind == SVR_DVS_KOBUS )
		{	/// 코버스
			KOBUS_BUS_CLS_DT koList[] = 
			{
				{ "1", "우등"			},
				{ "2", "고속"			},
				{ "3", "심야우등"		},
				{ "4", "심야고속"		},
				{ "5", "일반"			},
				{ "6", "심야일반"		},
				{ "7", "프리미엄"		},
				{ "8", "심야프리미엄"		},
			};

			for(i = 0; i < sizeof(koList) / sizeof(KOBUS_BUS_CLS_DT); i++)
			{
				if( memcmp(bus_cls_cd, koList[i].bus_cls_cd, strlen(bus_cls_cd)) == 0 )
				{
					sprintf(retName, "%s", koList[i].bus_cls_nm);
					TRACE("버스등급명 = %s \n", retName);
					nFind = i;
					break;
				}
			}
		}
		else if( nSvrKind == SVR_DVS_TMEXP )
		{
			rtk_tm_readcmncd_list_t ListInfo;

			nIndex = FindFileID(OPER_FILE_ID_EZ_BUS_CLS);
			if(nIndex < 0)
			{
				return -1;
			}

			strFullName = m_strCurrentPath + _T("\\files\\") + op_file_list[nIndex].strFileName;
			{
				hFile = MyOpenFile(strFullName, FALSE);
				if(hFile == INVALID_HANDLE_VALUE)
				{
					TR_LOG_OUT("MyOpenFile() error");
					return -2;
				}

				while( 1 ) 
				{
					::ZeroMemory(&ListInfo, sizeof(rtk_tm_readcmncd_list_t));

					nRet = MyReadFile(hFile, &ListInfo, sizeof(rtk_tm_readcmncd_list_t));
					if(nRet <= 0)
					{
						break;
					}

					if( memcmp(bus_cls_cd, ListInfo.cmn_cd_val, sizeof(ListInfo.cmn_cd_val) - 1) == 0 )
					{
						sprintf(retName, "%s", ListInfo.cd_val_nm);
						TR_LOG_OUT("티머니고속 - 버스등급명 = %s \n", retName, bus_cls_cd);
						nFind = i;
						break;
					}

				}
				MyCloseFile(hFile);
			}
		}
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nFind;
}

/**
 * @brief		FindBusTckKndName
 * @details		승차권 종류 찾기
 * @param		int nSvrKind			서버종류
 * @param		char *bus_tck_knd_cd	승차권 종류 코드
 * @param		char *retName			승차권 종류 이름
 * @return		성공 : >= 0, 실패 : < 0
 */
int FindBusTckKndName(int nSvrKind, char *bus_tck_knd_cd, char *retName)
{
	int nRet, nIndex, i, nCount, nFind;
	CString strFullName;
	HANDLE hFile;

	nFind = -1;
	try
	{
		//TR_LOG_OUT("#1 SvrKind(%d), bus_tck_knd_cd = (%s)", nSvrKind, bus_tck_knd_cd);

		if( nSvrKind == SVR_DVS_CCBUS )
		{	/// 시외버스
			rtk_readtckknd_t Info;
			rtk_readtckknd_list_t ListInfo;

			nIndex = FindFileID(OPER_FILE_ID_0108);
			if(nIndex < 0)
			{
				return -1;
			}

			::ZeroMemory(&Info, sizeof(rtk_readtckknd_t));

			strFullName = m_strCurrentPath + _T("\\files\\") + op_file_list[nIndex].strFileName;
			{
				hFile = MyOpenFile(strFullName, FALSE);
				if(hFile == INVALID_HANDLE_VALUE)
				{
					TR_LOG_OUT("MyOpenFile() error");
					return -2;
				}

				nRet = MyReadFile(hFile, Info.rsp_cd, sizeof(Info.rsp_cd));
				nRet = MyReadFile(hFile, Info.bus_tck_knd_cd_num, sizeof(Info.bus_tck_knd_cd_num));

				nCount = *(int *) Info.bus_tck_knd_cd_num;

				for(i = 0; i < nCount; i++)
				{
					::ZeroMemory(&ListInfo, sizeof(rtk_readtckknd_list_t));

					nRet = MyReadFile(hFile, &ListInfo, sizeof(rtk_readtckknd_list_t));
					if(nRet <= 0)
					{
						break;
					}

					if( memcmp(bus_tck_knd_cd, ListInfo.bus_tck_knd_cd, sizeof(ListInfo.bus_tck_knd_cd) - 1) == 0 )
					{
						sprintf(retName, "%s", ListInfo.bus_tck_knd_nm);
						//TR_LOG_OUT("#2 SvrKind(%d), bus_tck_knd_cd(%s), name(%s)", nSvrKind, bus_tck_knd_cd, retName);
						nFind = i;
						break;
					}

				}
				MyCloseFile(hFile);
			}
		}
		else if( nSvrKind == SVR_DVS_KOBUS )
		{	/// 코버스 - 승차권 종류
			rtk_cm_readtckknd_t			Info;
			rtk_cm_readtckknd_list_t	ListInfo;

			nIndex = FindFileID(OPER_FILE_ID_KO_CM_READTCKKND);
			if(nIndex < 0)
			{
				return -1;
			}

			strFullName = m_strCurrentPath + _T("\\files\\") + op_file_list[nIndex].strFileName;
			{
				hFile = MyOpenFile(strFullName, FALSE);
				if(hFile == INVALID_HANDLE_VALUE)
				{
					TR_LOG_OUT("MyOpenFile() error");
					return -2;
				}

				i = 0;
				nRet = MyReadFile(hFile, &Info, sizeof(rtk_cm_readtckknd_t));
				while(1)
				{
					::ZeroMemory(&ListInfo, sizeof(rtk_cm_readtckknd_list_t));

					nRet = MyReadFile(hFile, &ListInfo, sizeof(rtk_cm_readtckknd_list_t));
					if(nRet <= 0)
					{
						break;
					}

					if( memcmp(bus_tck_knd_cd, ListInfo.tck_knd_cd, strlen(bus_tck_knd_cd)) == 0 )
					{
						sprintf(retName, "%s", ListInfo.tck_nm_ko);
						TR_LOG_OUT("#3 SvrKind(%d), bus_tck_knd_cd(%s), name(%s)", nSvrKind, bus_tck_knd_cd, retName);
						nFind = i;
						break;
					}
					i++;
				}
				MyCloseFile(hFile);
			}
		}
		else if( nSvrKind == SVR_DVS_TMEXP )
		{	/// 티머니고속 - 승차권 종류
			rtk_tm_cmm_t				Info;
			rtk_tm_readtckknd_list_t	ListInfo;

			nIndex = FindFileID(OPER_FILE_ID_EZ_CM_READTCKKND);
			if(nIndex < 0)
			{
				return -1;
			}

			strFullName = m_strCurrentPath + _T("\\files\\") + op_file_list[nIndex].strFileName;
			{
				hFile = MyOpenFile(strFullName, FALSE);
				if(hFile == INVALID_HANDLE_VALUE)
				{
					TR_LOG_OUT("MyOpenFile() error");
					return -2;
				}

				i = 0;
				nRet = MyReadFile(hFile, &Info, sizeof(rtk_tm_cmm_t));
				while(1)
				{
					::ZeroMemory(&ListInfo, sizeof(rtk_tm_readtckknd_list_t));

					nRet = MyReadFile(hFile, &ListInfo, sizeof(rtk_tm_readtckknd_list_t));
					if(nRet <= 0)
					{
						break;
					}

					if( memcmp(bus_tck_knd_cd, ListInfo.tck_knd_cd, strlen(bus_tck_knd_cd)) == 0 )
					{
						sprintf(retName, "%s", ListInfo.ptrg_prin_nm);
						TR_LOG_OUT("#3 SvrKind(%d), bus_tck_knd_cd(%s), name(%s)", nSvrKind, bus_tck_knd_cd, retName);
						nFind = i;
						break;
					}
					i++;
				}
				MyCloseFile(hFile);
			}
		}
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nFind;
}

/**
 * @brief		FindTerminalName
 * @details		터미널명 찾기
 * @param		int nLanguage		언어종류		
 * @param		char *trml_cd		터미널코드
 * @param		char *retName		터미널 이름
 * @return		성공 : >= 0, 실패 : < 0
 */
int FindTerminalName(int nLanguage, char *trml_cd, char *retName)
{
	int						nRet, nIndex, i, nCount, nFind;
	CString					strFullName;
	HANDLE					hFile;
	rtk_readtrmlcd_t		Info;
	rtk_readtrmlcd_list_t	ListInfo;

	nFind = -1;

	try
	{
		Locking();

		nIndex = FindFileID(OPER_FILE_ID_0120);
		if(nIndex < 0)
		{
			UnLocking();
			return -1;
		}
		UnLocking();

		::ZeroMemory(&Info, sizeof(rtk_readtrmlcd_t));

		strFullName = m_strCurrentPath + _T("\\files\\") + op_file_list[nIndex].strFileName;
		{
			hFile = MyOpenFile(strFullName, FALSE);
			if(hFile == INVALID_HANDLE_VALUE)
			{
				TR_LOG_OUT("MyOpenFile() error");
				return -2;
			}

			nRet = MyReadFile(hFile, Info.rsp_cd, sizeof(Info.rsp_cd));
			nRet = MyReadFile(hFile, Info.trml_cd_num, sizeof(Info.trml_cd_num));

			nCount = *(int *) Info.trml_cd_num;

			for(i = 0; i < nCount; i++)
			{
				::ZeroMemory(&ListInfo, sizeof(rtk_readtrmlcd_list_t));

				nRet = MyReadFile(hFile, &ListInfo, sizeof(rtk_readtrmlcd_list_t));
				if(nRet <= 0)
				{
					break;
				}

				if( memcmp(trml_cd, ListInfo.trml_cd, sizeof(ListInfo.trml_cd) - 1) == 0 )
				{
					if(nLanguage == LANG_KOR)
					{
						sprintf(retName, "%s", ListInfo.trml_nm);
					}
					else
					{
						sprintf(retName, "%s", ListInfo.trml_eng_nm);
					}
					nFind = i;
					break;
				}
				else if( memcmp(trml_cd, GetTrmlCode(SVR_DVS_CCBUS), strlen(trml_cd) - 1) == 0 )
				{
					sprintf(retName, "%s", GetTrmlName());
					nFind = i;
					break;
				}

			}
			MyCloseFile(hFile);
		}
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nFind;
}

/**
 * @brief		Find_KobusTrmlName
 * @details		[코버스] 터미널명 찾기
 * @param		int nLanguage		언어종류		
 * @param		char *trml_cd		터미널코드
 * @param		char *retName		터미널 이름
 * @return		성공 : >= 0, 실패 : < 0
 */
int Find_KobusTrmlName(int nLanguage, char *trml_cd, char *retName)
{
	int						nRet, nIndex, i, nCount, nFind;
	char					cmpLang[10];
	CString					strFullName;
	HANDLE					hFile;
	rtk_tm_etrmlinfo_t		Info;
	rtk_tm_etrmlinfo_list_t	ListInfo;

	nFind = -1;

	switch(nLanguage)
	{
	case LANG_KOR:
		sprintf(cmpLang, "KO");
		break;
	case LANG_ENG:
		sprintf(cmpLang, "EN");
		break;
	case LANG_JPN:
		sprintf(cmpLang, "JP");
		break;
	case LANG_CHINA:
		sprintf(cmpLang, "CN");
		break;
	}

	try
	{
		Locking();

		nIndex = FindFileID(OPER_FILE_ID_KO_CM_READTRMLINF);
		if(nIndex < 0)
		{
			UnLocking();
			return -1;
		}
		UnLocking();

		::ZeroMemory(&Info, sizeof(rtk_tm_etrmlinfo_t));

		strFullName = m_strCurrentPath + _T("\\files\\") + op_file_list[nIndex].strFileName;
		{
			hFile = MyOpenFile(strFullName, FALSE);
			if(hFile == INVALID_HANDLE_VALUE)
			{
				TR_LOG_OUT("MyOpenFile() error");
				return -2;
			}

			/// 4개국어 정보
			for(int k = 0; k < 4; k++)
			{
				::ZeroMemory(&Info, sizeof(rtk_tm_etrmlinfo_t));

				/// (01). 
				nRet = MyReadFile(hFile, Info.msg_cd, sizeof(Info.msg_cd));
				/// (02). 
				nRet = MyReadFile(hFile, Info.msg_dtl_ctt, sizeof(Info.msg_dtl_ctt));
				/// (03). 
				nRet = MyReadFile(hFile, Info.rec_ncnt1, sizeof(Info.rec_ncnt1));

				nCount = *(int *) Info.rec_ncnt1;

				for(i = 0; i < nCount; i++)
				{
					char lng_cd				[2+1]	;	///< 언어구분

					::ZeroMemory(&lng_cd, sizeof(lng_cd));
					::ZeroMemory(&ListInfo, sizeof(rtk_tm_etrmlinfo_list_t));

					/// (04). 
					nRet = MyReadFile(hFile, &lng_cd, sizeof(lng_cd));
					if(nRet <= 0)
					{
						break;
					}

					/// (05). 
					nRet = MyReadFile(hFile, &ListInfo, sizeof(rtk_tm_etrmlinfo_list_t));
					if(nRet <= 0)
					{
						break;
					}

					if( (memcmp(cmpLang, lng_cd, 2) == 0) && (memcmp(trml_cd, ListInfo.trml_no, sizeof(ListInfo.trml_no) - 1) == 0) )
					{
						if(nLanguage == LANG_KOR)
						{
							sprintf(retName, "%s", ListInfo.trml_abrv_nm);
//							sprintf(retName, "%s", ListInfo.trml_nm);
						}
						else
						{
							sprintf(retName, "%s", ListInfo.trml_eng_abrv_nm);
//							sprintf(retName, "%s", ListInfo.trml_eng_nm);
						}
						
						nFind = i;
						break;
					}
				}
			}
//end_proc:
			if(nFind < 0)
			{
				TR_LOG_OUT("[코버스] 터미널정보 없음. trml_cd = [%s]..", trml_cd);
			}
			else
			{
				TR_LOG_OUT("[코버스] 터미널정보 찾음. trml_cd = [%s][%s]..", trml_cd, retName);
			}
			MyCloseFile(hFile);
		}
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nFind;
}

/**
 * @brief		Find_TmExpTrmlName
 * @details		[티머니고속] 터미널명 찾기
 * @param		int nLanguage		언어종류		
 * @param		char *trml_cd		터미널코드
 * @param		char *retName		터미널 이름
 * @return		성공 : >= 0, 실패 : < 0
 */
int Find_TmExpTrmlName(int nLanguage, char *trml_cd, char *retName)
{
	int						nRet, nIndex, i, nCount, nFind;
	char					cmpLang[10];
	CString					strFullName;
	HANDLE					hFile;
	rtk_tm_readtrml_list_t	ListInfo;

	nFind = -1;

	switch(nLanguage)
	{
	case LANG_KOR:
		sprintf(cmpLang, "KO");
		break;
	case LANG_ENG:
		sprintf(cmpLang, "EN");
		break;
	case LANG_JPN:
		sprintf(cmpLang, "JP");
		break;
	case LANG_CHINA:
		sprintf(cmpLang, "CN");
		break;
	}

	try
	{
		Locking();

		nIndex = FindFileID(OPER_FILE_ID_EZ_CM_READTRML);
		if(nIndex < 0)
		{
			UnLocking();
			return -1;
		}
		UnLocking();

		strFullName = m_strCurrentPath + _T("\\files\\") + op_file_list[nIndex].strFileName;
		{
			hFile = MyOpenFile(strFullName, FALSE);
			if(hFile == INVALID_HANDLE_VALUE)
			{
				TR_LOG_OUT("MyOpenFile() error");
				return -2;
			}

			/// 4개국어 정보
			for(int k = 0; k < 4; k++)
			{
				char rsp_cd				[6		+1]	;		///< 응답코드
				char rec_num			[4		+1]	;		///< 정보 갯수

				::ZeroMemory(rsp_cd, sizeof(rsp_cd));
				::ZeroMemory(rec_num, sizeof(rec_num));

				/// (01). 
				nRet = MyReadFile(hFile, rsp_cd, sizeof(rsp_cd));
				/// (02). 
				nRet = MyReadFile(hFile, rec_num, sizeof(rec_num));

				nCount = *(int *) rec_num;

				for(i = 0; i < nCount; i++)
				{
					char lng_cd				[2+1]	;	///< 언어구분

					::ZeroMemory(&lng_cd, sizeof(lng_cd));
					::ZeroMemory(&ListInfo, sizeof(rtk_tm_readtrml_list_t));

					/// (03). 
					nRet = MyReadFile(hFile, &ListInfo, sizeof(rtk_tm_readtrml_list_t));
					if(nRet <= 0)
					{
						break;
					}

					//TR_LOG_OUT("[티머니고속] 터미널정보(%2d), lang(%s, %s), trml_no(%s, %s)..", i, cmpLang, ListInfo.lng_cd, trml_cd, ListInfo.trml_no);

					if( (memcmp(cmpLang, ListInfo.lng_cd, 2) == 0) && (memcmp(trml_cd, ListInfo.trml_no, strlen(ListInfo.trml_no)) == 0) )
					{
						if(nLanguage == LANG_KOR)
						{
							sprintf(retName, "%s", ListInfo.trml_abrv_nm);
						}
						else
						{
							sprintf(retName, "%s", ListInfo.trml_eng_abrv_nm);
//							sprintf(retName, "%s", ListInfo.trml_eng_nm);
						}
						
						nFind = i;
						goto end_proc;
					}
				}
			}
end_proc:
			if(nFind < 0)
			{
				TR_LOG_OUT("[티머니고속] 터미널정보 없음. trml_cd = [%s]..", trml_cd);
			}
			else
			{
				TR_LOG_OUT("[티머니고속] 터미널정보 찾음. trml_cd = [%s][%s]..", trml_cd, retName);
			}
			MyCloseFile(hFile);
		}
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return nFind;
}

/**
 * @brief		FindTicketKindData
 * @details		승차권 종류 찾기
 * @param		int n_bus_dvs		서버 종류
 * @param		char *tck_knd		승차권 종류
 * @param		char *retBuf		승차권 종류 리스트
 * @return		성공 : >= 0, 실패 : < 0
 */
int FindTicketKindData(int n_bus_dvs, char *tck_knd, char *retBuf)
{
	int i, nLen;

	try
	{
		nLen = strlen(tck_knd);
		if(nLen <= 0)
		{
			return -2;
		}

		for(i = 0; i < sizeof(op_tck_list) / sizeof(TCK_KIND_LIST_T); i++)
		{
			if( (n_bus_dvs == op_tck_list[i].n_bus_dvs) && (memcmp(tck_knd, op_tck_list[i].tck_kind, nLen) == 0) )
			{
				// 			if( n_bus_dvs == SVR_DVS_CCBUS )
				// 			{
				// 				TR_LOG_OUT("시외_티켓종류 정보, indx(%d), code(%s : %s)...", i, tck_knd, op_tck_list[i].str_nm);
				// 			}
				// 			else
				// 			{
				// 				TR_LOG_OUT("고속_티켓종류 정보, indx(%d), code(%s : %s)...", i, tck_knd, op_tck_list[i].str_nm);
				// 			}
				::CopyMemory(retBuf, &op_tck_list[i], sizeof(TCK_KIND_LIST_T));
				//return i;
				return op_tck_list[i].n_tck_index;
			}
		}
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return -1;
}

/**
 * @brief		GetTckkndListData
 * @details		Index로 승차권 종류 찾기
 * @param		int n_bus_dvs		서버 종류
 * @param		int n_tck_idx		Index
 * @param		char *retBuf		승차권 종류 리스트
 * @return		성공 : >= 0, 실패 : < 0
 */
int GetTckkndListData(int n_bus_dvs, int n_tck_idx, char *retBuf)
{
	int i;

	try
	{
		for(i = 0; i < sizeof(op_tck_list) / sizeof(TCK_KIND_LIST_T); i++)
		{
			if( (n_bus_dvs == op_tck_list[i].n_bus_dvs) && (n_tck_idx == op_tck_list[i].n_tck_index) )
			{
// 				if( n_bus_dvs == SVR_DVS_CCBUS )
// 				{
// 					TR_LOG_OUT("시외_티켓종류 정보, indx(%d), code(%s : %s)...", i, op_tck_list[i].tck_kind, op_tck_list[i].str_nm);
// 				}
// 				else
// 				{
// 					TR_LOG_OUT("고속_티켓종류 정보, indx(%d), code(%s : %s)...", i, op_tck_list[i].tck_kind, op_tck_list[i].str_nm);
// 				}
				::CopyMemory(retBuf, &op_tck_list[i], sizeof(TCK_KIND_LIST_T));
				return i;
			}
		}
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	return -1;
}

/**
 * @brief		Find_CCS_RotID
 * @details		경유지 정보 찾기
 * @param		char *pSTrmlCode	터미널 코드
 * @param		char *pRotID		노선ID
 * @return		성공 : >= 0, 실패 : < 0
 */
int SaveThruInfo(char *pSTrmlCode)
{
	int						nRet, nIndex, i, nCount, nFind;
	CString					strFullName;
	HANDLE					hFile;
	rtk_readrotdtl_t		Info;
	rtk_readrotdtl_list_t	ListInfo;
	CConfigTkMem*			pCfgMem;
	
	pCfgMem = CConfigTkMem::GetInstance();

	::ZeroMemory(pCfgMem->m_tThruInfo, sizeof(pCfgMem->m_tThruInfo));

	nFind = -1;

	if(GetEnvOperCorp() != KUMHO_OPER_CORP)
	{	/// 금호터미널이 아니면..
		return -1;
	}

	TR_LOG_OUT("경유지 찾기, 터미널코드(%s) ", pSTrmlCode);

	try
	{
		/// 노선상세 정보
		nIndex = FindFileID(OPER_FILE_ID_0125);
		if(nIndex < 0)
		{
			return -1;
		}

		::ZeroMemory(&Info, sizeof(rtk_readrotdtl_t));

		strFullName = m_strCurrentPath + _T("\\files\\") + op_file_list[nIndex].strFileName;
		{
			hFile = MyOpenFile(strFullName, FALSE);
			if(hFile == INVALID_HANDLE_VALUE)
			{
				TR_LOG_OUT("MyOpenFile() error");
				return -2;
			}

			nRet = MyReadFile(hFile, Info.rsp_cd, sizeof(Info.rsp_cd));
			nRet = MyReadFile(hFile, Info.rot_dtl_num, sizeof(Info.rot_dtl_num));

			nCount = *(int *) Info.rot_dtl_num;

			TR_LOG_OUT("경유지 찾기, 노선 갯수(%d) ", nCount);

			nIndex = 0;
			for(i = 0; i < nCount; i++)
			{
				::ZeroMemory(&ListInfo, sizeof(rtk_readrotdtl_list_t));

				nRet = MyReadFile(hFile, &ListInfo, sizeof(rtk_readrotdtl_list_t));
				if(nRet <= 0)
				{
					break;
				}

				if( memcmp(pSTrmlCode, "5864201", 7) == 0 )
				{	/// 목포 터미널 경우
					//if( memcmp(ListInfo.rot_id, pRotID, strlen(pRotID)) == 0 )
					{
						if( memcmp(ListInfo.trml_cd, "5856701", 7 ) == 0 )
						{	/// 남악 경유..
							::CopyMemory(pCfgMem->m_tThruInfo[nIndex].rot_id, ListInfo.rot_id, sizeof(pCfgMem->m_tThruInfo[nIndex].rot_id) - 1);
							::CopyMemory(pCfgMem->m_tThruInfo[nIndex].trml_cd, ListInfo.trml_cd, sizeof(pCfgMem->m_tThruInfo[nIndex].trml_cd) - 1);
							TR_LOG_OUT("남악경유_%02d, 노선ID(%s), 터미널코드(%s)..", nIndex, ListInfo.rot_id, ListInfo.trml_cd);
							nIndex++;
							nFind = 0;
							//break;
						}
					}
				}
				else if( memcmp(pSTrmlCode, "5971501", 7) == 0 )
				{	/// 여수 터미널 경우
					//if( memcmp(ListInfo.rot_id, pRotID, strlen(pRotID)) == 0 )
					{
						if( memcmp(ListInfo.trml_cd, "6119902", 7 ) == 0 )
						{	/// 문화동 경유
							::CopyMemory(pCfgMem->m_tThruInfo[nIndex].rot_id, ListInfo.rot_id, sizeof(pCfgMem->m_tThruInfo[nIndex].rot_id) - 1);
							::CopyMemory(pCfgMem->m_tThruInfo[nIndex].trml_cd, ListInfo.trml_cd, sizeof(pCfgMem->m_tThruInfo[nIndex].trml_cd) - 1);
							TR_LOG_OUT("문화동경유_%02d, 노선ID(%s), 터미널코드(%s)..", nIndex, ListInfo.rot_id, ListInfo.trml_cd);
							nIndex++;
							nFind = 1;
// 							break;
						}
						if( memcmp(ListInfo.trml_cd, "6111601", 7 ) == 0 )
						{	/// 운암동 경유
							::CopyMemory(pCfgMem->m_tThruInfo[nIndex].rot_id, ListInfo.rot_id, sizeof(pCfgMem->m_tThruInfo[nIndex].rot_id) - 1);
							::CopyMemory(pCfgMem->m_tThruInfo[nIndex].trml_cd, ListInfo.trml_cd, sizeof(pCfgMem->m_tThruInfo[nIndex].trml_cd) - 1);
							TR_LOG_OUT("운암동경유_%02d, 노선ID(%s), 터미널코드(%s)..", nIndex, ListInfo.rot_id, ListInfo.trml_cd);
							nIndex++;
							nFind = 2;
// 							break;
						}
					}
				}
				else if( memcmp(pSTrmlCode, "6193701", 7) == 0 )
				{	/// 광주 터미널 경우
					/**
					if( memcmp(ListInfo.trml_cd, "6119902", 7 ) == 0 )
					{	/// 문화동 경유
						::CopyMemory(pCfgMem->m_tThruInfo[nIndex].rot_id, ListInfo.rot_id, sizeof(pCfgMem->m_tThruInfo[nIndex].rot_id) - 1);
						::CopyMemory(pCfgMem->m_tThruInfo[nIndex].trml_cd, ListInfo.trml_cd, sizeof(pCfgMem->m_tThruInfo[nIndex].trml_cd) - 1);
						TR_LOG_OUT("문화동경유_%02d, 노선ID(%s), 터미널코드(%s)..", nIndex, ListInfo.rot_id, ListInfo.trml_cd);
						nIndex++;
						nFind = 1;
						// 							break;
					}
					if( memcmp(ListInfo.trml_cd, "6111601", 7 ) == 0 )
					{	/// 운암동 경유
						::CopyMemory(pCfgMem->m_tThruInfo[nIndex].rot_id, ListInfo.rot_id, sizeof(pCfgMem->m_tThruInfo[nIndex].rot_id) - 1);
						::CopyMemory(pCfgMem->m_tThruInfo[nIndex].trml_cd, ListInfo.trml_cd, sizeof(pCfgMem->m_tThruInfo[nIndex].trml_cd) - 1);
						TR_LOG_OUT("운암동경유_%02d, 노선ID(%s), 터미널코드(%s)..", nIndex, ListInfo.rot_id, ListInfo.trml_cd);
						nIndex++;
						nFind = 2;
						// 							break;
					}
					**/
				}
				else
				{
					nFind = -1;
					break;
				}
			}
			MyCloseFile(hFile);
		}
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	TR_LOG_OUT("경유지 찾기 Result = %d !!!", nIndex);

	return nIndex;
}

/**
 * @brief		Find_CCS_RotID
 * @details		경유지 정보 찾기
 * @param		char *pSTrmlCode	터미널 코드
 * @param		char *pRotID		노선ID
 * @return		성공 : >= 0, 실패 : < 0
 */
int Find_CCS_RotID(char *pSTrmlCode, char *pRotID)
{
	int						nRet, nIndex, i, nCount, nFind;
	CString					strFullName;
	HANDLE					hFile;
	rtk_readrotdtl_t		Info;
	rtk_readrotdtl_list_t	ListInfo;
	CConfigTkMem*			pCfgMem;
	
	pCfgMem = CConfigTkMem::GetInstance();

	::ZeroMemory(pCfgMem->m_tThruInfo, sizeof(pCfgMem->m_tThruInfo));

	nFind = -1;

	if(GetEnvOperCorp() != KUMHO_OPER_CORP)
	{	/// 금호터미널이 아니면..
		return -1;
	}

	TR_LOG_OUT("경유지 찾기, 터미널코드(%s), 노선ID(%s) ", pSTrmlCode, pRotID);

	try
	{
		/// 노선상세 정보
		nIndex = FindFileID(OPER_FILE_ID_0125);
		if(nIndex < 0)
		{
			return -1;
		}

		::ZeroMemory(&Info, sizeof(rtk_readrotdtl_t));

		strFullName = m_strCurrentPath + _T("\\files\\") + op_file_list[nIndex].strFileName;
		{
			hFile = MyOpenFile(strFullName, FALSE);
			if(hFile == INVALID_HANDLE_VALUE)
			{
				TR_LOG_OUT("MyOpenFile() error");
				return -2;
			}

			nRet = MyReadFile(hFile, Info.rsp_cd, sizeof(Info.rsp_cd));
			nRet = MyReadFile(hFile, Info.rot_dtl_num, sizeof(Info.rot_dtl_num));

			nCount = *(int *) Info.rot_dtl_num;

			TR_LOG_OUT("경유지 찾기, 노선 갯수(%d) ", nCount);

			nIndex = 0;
			for(i = 0; i < nCount; i++)
			{
				::ZeroMemory(&ListInfo, sizeof(rtk_readrotdtl_list_t));

				nRet = MyReadFile(hFile, &ListInfo, sizeof(rtk_readrotdtl_list_t));
				if(nRet <= 0)
				{
					break;
				}

				if( memcmp(pSTrmlCode, "5864201", 7) == 0 )
				{	/// 목포 터미널 경우
					if( memcmp(ListInfo.rot_id, pRotID, strlen(pRotID)) == 0 )
					{
						if( memcmp(ListInfo.trml_cd, "5856701", 7 ) == 0 )
						{	/// 남악 경유..
							::CopyMemory(pCfgMem->m_tThruInfo[nIndex].rot_id, pRotID, sizeof(pCfgMem->m_tThruInfo[nIndex].rot_id) - 1);
							::CopyMemory(pCfgMem->m_tThruInfo[nIndex].trml_cd, ListInfo.trml_cd, sizeof(pCfgMem->m_tThruInfo[nIndex].trml_cd) - 1);
							nIndex++;
							nFind = 0;
							//break;
						}
					}
				}
				else if( memcmp(pSTrmlCode, "5971501", 7) == 0 )
				{	/// 여수 터미널 경우
					if( memcmp(ListInfo.rot_id, pRotID, strlen(pRotID)) == 0 )
					{
						if( memcmp(ListInfo.trml_cd, "6119902", 7 ) == 0 )
						{	/// 문화동 경유
							::CopyMemory(pCfgMem->m_tThruInfo[nIndex].rot_id, pRotID, sizeof(pCfgMem->m_tThruInfo[nIndex].rot_id) - 1);
							::CopyMemory(pCfgMem->m_tThruInfo[nIndex].trml_cd, ListInfo.trml_cd, sizeof(pCfgMem->m_tThruInfo[nIndex].trml_cd) - 1);
							nIndex++;
							nFind = 1;
// 							break;
						}
						if( memcmp(ListInfo.trml_cd, "6111601", 7 ) == 0 )
						{	/// 운암동 경유
							::CopyMemory(pCfgMem->m_tThruInfo[nIndex].rot_id, pRotID, sizeof(pCfgMem->m_tThruInfo[nIndex].rot_id) - 1);
							::CopyMemory(pCfgMem->m_tThruInfo[nIndex].trml_cd, ListInfo.trml_cd, sizeof(pCfgMem->m_tThruInfo[nIndex].trml_cd) - 1);
							nIndex++;
							nFind = 2;
// 							break;
						}
					}
				}
				else
				{
					nFind = -1;
					break;
				}
			}
			MyCloseFile(hFile);
		}
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	TR_LOG_OUT("경유지 찾기 Result = %d !!!", nIndex);

	return nFind;
}

/**
 * @brief		Find_CCS_RotName
 * @details		노선명 찾기
 * @param		char *pRotID		노선ID
 * @param		char *retNm			노선명
 * @return		성공 : >= 0, 실패 : < 0
 */
int Find_CCS_RotName(char *pRotID, char *retNm)
{
	int						nRet, nIndex, i, nCount, nFind;
	CString					strFullName;
	HANDLE					hFile;
	rtk_readrotbsc_t		Info;
	rtk_readrotbsc_list_t	ListInfo;
	//CConfigTkMem*			pCfgMem;
	
	nFind = -1;

	if(GetEnvOperCorp() != IIAC_OPER_CORP)
	{	/// 인천공항이 아니면..
		return -1;
	}

	//TR_LOG_OUT("노선명 찾기, 노선ID(%s) ", pRotID);

	try
	{
		/// 노선상세 정보
		nIndex = FindFileID(OPER_FILE_ID_0124);
		if(nIndex < 0)
		{
			return -1;
		}

		::ZeroMemory(&Info, sizeof(rtk_readrotbsc_t));

		strFullName = m_strCurrentPath + _T("\\files\\") + op_file_list[nIndex].strFileName;
		{
			hFile = MyOpenFile(strFullName, FALSE);
			if(hFile == INVALID_HANDLE_VALUE)
			{
				TR_LOG_OUT("MyOpenFile() error");
				return -2;
			}

			nRet = MyReadFile(hFile, Info.rsp_cd, sizeof(Info.rsp_cd));
			nRet = MyReadFile(hFile, Info.rot_bsc_num, sizeof(Info.rot_bsc_num));

			nCount = *(int *) Info.rot_bsc_num;

			//TR_LOG_OUT("노선ID 찾기, 노선 갯수(%d) ", nCount);

			nIndex = 0;
			for(i = 0; i < nCount; i++)
			{
				::ZeroMemory(&ListInfo, sizeof(rtk_readrotbsc_list_t));

				nRet = MyReadFile(hFile, &ListInfo, sizeof(rtk_readrotbsc_list_t));
				if(nRet <= 0)
				{
					break;
				}

				if( memcmp(pRotID, ListInfo.rot_id, strlen(ListInfo.rot_id)) == 0 )
				{	///
					//char Buffer[10];
					//int nRotNum;
					int k, j;

					nFind = i;
					//TR_LOG_OUT("노선ID 찾기 - index(%d), id(%s) name(%s) !!!", i, pRotID, ListInfo.rot_nm);

					for(j = 0, k = 0; k < (int)strlen(ListInfo.rot_nm); k++)
					{
						//if( (ListInfo.rot_nm[k] >= 0x30) && (ListInfo.rot_nm[k] <= 0x39) )	// 20220429 DEL : N7000 or 6009-1 표시안됨
						// 20220429 MOD~
						if( (ListInfo.rot_nm[k] >= 0x30) && (ListInfo.rot_nm[k] <= 0x39) 
						 || (ListInfo.rot_nm[k] >= 0x41) && (ListInfo.rot_nm[k] <= 0x5A) 
						 || (ListInfo.rot_nm[k] == 0x2D) )
						// 20220429 ~MOD
						{
							retNm[j++] = ListInfo.rot_nm[k];
							retNm[j] = 0;
						}
						else
						{
							break;
						}
					}
					//TR_LOG_OUT("노선명 - retNm(%s), rot_name(%s) !!!", retNm, ListInfo.rot_nm);
					break;
				}
			}
			MyCloseFile(hFile);
		}
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	TR_LOG_OUT("노선ID 찾기 Result = %d !!!", nFind);

	return nFind;
}

/**
 * @brief		ConvertDCRT
 * @details		할인율 정보 가져오기
 * @param		char *dcrt			공통코드 할인율 정보
 * @param		char *retBuff		공통코드 할인율 string
 * @return		성공 : >= 0, 실패 : < 0
 */
int ConvertDCRT(char *dcrt, char *retBuff)
{
	int						nRet, nIndex, i, nCount, nFind;
	CString					strFullName;
	HANDLE					hFile;
	rtk_cmncddtl_t			Info;
	rtk_cmncddtl_list_t		ListInfo;
	//CConfigTkMem*			pCfgMem;
	
	nFind = -1;

	TR_LOG_OUT("할인율 정보, 코드값(%s) ", dcrt);

	try
	{
		/// 공통코드상세 정보
		nIndex = FindFileID(OPER_FILE_ID_0102);
		if(nIndex < 0)
		{
			return -1;
		}

		::ZeroMemory(&Info, sizeof(rtk_cmncddtl_t));

		strFullName = m_strCurrentPath + _T("\\files\\") + op_file_list[nIndex].strFileName;
		{
			hFile = MyOpenFile(strFullName, FALSE);
			if(hFile == INVALID_HANDLE_VALUE)
			{
				TR_LOG_OUT("MyOpenFile() error");
				return -2;
			}

			nRet = MyReadFile(hFile, Info.rsp_cd, sizeof(Info.rsp_cd));
			nRet = MyReadFile(hFile, Info.cmn_dtl_num, sizeof(Info.cmn_dtl_num));

			nCount = *(int *) Info.cmn_dtl_num;

			TR_LOG_OUT("공통코드 상세정보, 갯수(%d) ", nCount);

			nIndex = 0;
			for(i = 0; i < nCount; i++)
			{
				::ZeroMemory(&ListInfo, sizeof(rtk_cmncddtl_list_t));

				nRet = MyReadFile(hFile, &ListInfo, sizeof(rtk_cmncddtl_list_t));
				if(nRet <= 0)
				{
					break;
				}

				if( (memcmp(ListInfo.cmn_cd_id, "Z51", 3) == 0) && (memcmp(dcrt, ListInfo.cmn_cd_val, strlen(dcrt)) == 0) )
				{	///
					sprintf(retBuff, "%s", ListInfo.cd_rfrn_val_1);
					nFind = i;
					break;
				}
			}
			MyCloseFile(hFile);
		}
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	TR_LOG_OUT("공통코드 상세정보, 할인율 정보 Result = (%s), (%d) !!!", retBuff, nFind);

	return nFind;
}

/**
 * @brief		ConvertDCKnd
 * @details		할인종류 정보 가져오기
 * @param		char *knd			할인종류 정보
 * @param		char *retBuff		할인종류 string
 * @return		성공 : >= 0, 실패 : < 0
 */
int ConvertDCKnd(char *knd, char *retBuff)
{
	switch(knd[0])
	{
	case 'A':	/// 사전
		sprintf(retBuff, "%s", "사전");
		break;
	case 'B':	/// 왕복
		sprintf(retBuff, "%s", "왕복");
		break;
	case 'C':	/// 단체
		sprintf(retBuff, "%s", "단체");
		break;
	case 'D':	/// 기피
		sprintf(retBuff, "%s", "기피");
		break;
	case 'Z':	/// 할인없음
		sprintf(retBuff, "%s", "할인없음");
		break;
	default:	/// Unknown
		sprintf(retBuff, "%s", "Unknown");
		break;
	}

	return 0;
}

/**
 * @brief		Oper_GetTmExpRefundCode
 * @details		(티머니고속) 환불율 정보 가져오기
 * @param		char *knd			할인종류 정보
 * @param		char *retBuff		할인종류 string
 * @return		성공 : >= 0, 실패 : < 0
 */
int Oper_GetTmExpRefundCode(int nRate, char *retBuf)
{
	//
	int						nRet, nIndex, i, nCount, nFind, nInfoRate;
	CString					strFullName;
	HANDLE					hFile;
	char					Buffer[100] = {0, };
	rtk_tm_read_ryrt_t		Info;
	rtk_tm_read_ryrt_list_t	ListInfo;
	//CConfigTkMem*			pCfgMem;

	nRet = nIndex = i = nCount = nFind = nInfoRate = 0;
	nFind = -1;

	TR_LOG_OUT("터미널 환불율 정보, 코드값(%d) ", nRate);

	try
	{
		/// (티머니고속) 터미널 환불율 정보
		nIndex = FindFileID(OPER_FILE_ID_EZ_CM_READRYRT);
		if(nIndex < 0)
		{
			return -1;
		}

		::ZeroMemory(&Info, sizeof(rtk_tm_read_ryrt_t));

		strFullName = m_strCurrentPath + _T("\\files\\") + op_file_list[nIndex].strFileName;
		{
			hFile = MyOpenFile(strFullName, FALSE);
			if(hFile == INVALID_HANDLE_VALUE)
			{
				TR_LOG_OUT("MyOpenFile() error");
				return -2;
			}

			nRet = MyReadFile(hFile, Info.rsp_cd, sizeof(Info.rsp_cd));
			nRet = MyReadFile(hFile, Info.rec_num, sizeof(Info.rec_num));

			nCount = *(int *) Info.rec_num;

			TR_LOG_OUT("터미널 환불율 정보, 갯수(%d) ", nCount);

			nIndex = 0;
			for(i = 0; i < nCount; i++)
			{
				char *cp, *tp;

				::ZeroMemory(&ListInfo, sizeof(rtk_tm_read_ryrt_list_t));

				nRet = MyReadFile(hFile, &ListInfo, sizeof(rtk_tm_read_ryrt_list_t));
				if(nRet <= 0)
				{
					break;
				}

				TR_LOG_OUT("터미널 환불율 정보(%d), (%s, %s) ", i, ListInfo.cd_val_nm, ListInfo.ry_knd_cd);

				tp = ListInfo.cd_val_nm;
				cp = strstr(tp, "%");
				if( cp )
				{
					::ZeroMemory(Buffer, sizeof(Buffer));
					memcpy(Buffer, tp, cp - tp);
					nInfoRate = Util_Ascii2Long(Buffer, cp - tp);

					TR_LOG_OUT("터미널 환불율 정보, 퍼센트(%d, %d) ", nRate, nInfoRate);

					if(nRate == nInfoRate)
					{
						/// 환불율 종류 코드
						//sprintf(retBuf, "%s", ListInfo.ry_knd_cd);
						retBuf[0] = ListInfo.ry_knd_cd[0];
						nFind = i;
						break;
					}
				}
			}
			MyCloseFile(hFile);
		}
	}
	catch ( ... )
	{
		TR_LOG_OUT("EXCEPTION error = %d !!!", ::GetLastError());
	}

	TR_LOG_OUT("터미널 환불율 정보, 할인율 정보 Result = (%s), (%d) !!!", retBuf, nFind);

	return nFind;
}
