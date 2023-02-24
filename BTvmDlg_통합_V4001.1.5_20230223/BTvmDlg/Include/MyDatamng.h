// 
// 
// 
//

#pragma once

#include <vector>

using namespace std ;

#define DATA_MAGIC_CODE		0x11223344
#define ACCUM_MAGIC_CODE	0x55667788

#define FILE_FLAG_SAVE		0x33			///> 
#define FILE_FLAG_SEND		0x11			///> 

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#pragma pack(1)

typedef struct 
{
	DWORD			dwVersion;		
	DWORD			dwSerial;		
	DWORD			dwMagic;		
	DWORD			dwTick;			
	DWORD			dwCount;		
	DWORD			dwSend;			
	DWORD			dwSeq;			
} FILE_HEADER_T, *PFILE_HEADER_T;

typedef struct 
{
	DWORD			dwTick;
	DWORD			dwSize;
	DWORD			bSend;
} SEND_HEADER_T, *PSEND_HEADER_T;

//
typedef struct 
{
	char			date[50];		
	DWORD			dwSerial;			
	DWORD			dwTick;			
	DWORD			dwCount;			
	DWORD			dwSend;		
	DWORD			dwSeq;
} DATA_FILE_ENTRY_T, *PDATA_FILE_ENTRY_T;

typedef struct 
{
	DWORD			dwLoad;
	DWORD			dwSave;
} DATA_POINT_T, *PDATA_POINT_T;

#pragma pack()

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


typedef vector<DATA_FILE_ENTRY_T> FILE_VECTOR;
typedef FILE_VECTOR::iterator itrFile;
typedef FILE_VECTOR::reverse_iterator ritrFile;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class NHS_AFX_EXPORT CMyDatamng
{
public:
	CMyDatamng();
	CMyDatamng(int nSave);
	~CMyDatamng();
				  
public:

	HANDLE			m_hAccMutex;
	FILE_VECTOR		m_FileVector;
	DATA_POINT_T	m_tDataPt;
	BYTE			m_Temp[1024 * 4];

	int				m_nMAX_SAVE_DATE;
	CString			m_strPath;

private :
	int Locking(void);
	int UnLocking(void);
	//BOOL SortFunction(DATA_FILE_ENTRY_T a, DATA_FILE_ENTRY_T b);

public:
	int AddData(char *pData, int nDataLen);
	int CheckData(void);
	int ResetDateData(char *pBegDate, char *pEndDate);
	int ReadData(char *pData, int nDataLen);
	int AckData(char *pData, int nDataLen);
	int CreateData(void);
	void SetPath(CString& strPath);
	void SetMaxSaveDate(int nValue);
	int InitData(void);
	int TermData(void);
};



