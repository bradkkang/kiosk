#ifndef WOMRDLL

#define OMR_ERR_NONE				(0)

#define WT_ERR_DEVICEFAIL			4

#define OMR_ERR_IMG_LOCK			(21)
#define OMR_ERR_IMG_LOAD			(22)
#define OMR_ERR_IMG_DATA			(23)
#define OMR_ERR_IMG_BUF				(24)
#define OMR_ERR_IMG_ROTATE			(25)
#define OMR_ERR_IMG_NO_CLOCKMARK	(26)
	
#define OMR_ERR_TMPL_LOAD			(41)
#define OMR_ERR_TMPL_MAKE			(42)
#define OMR_ERR_TMPL_SYMBOL			(43)
#define OMR_ERR_TMPL_NOMATCH		(44)

#define OMR_ERR_BARCODE_INIT		71
#define OMR_ERR_BARCODE_FILE		72
#define OMR_ERR_BARCODE				73

#define BARCODE_CODE128			0x0001
#define BARCODE_CODE39			0x0002
#define BARCODE_CODE25			0x0004
#define BARCODE_EAN8			0x0008
#define BARCODE_EAN13			0x0010
#define BARCODE_UPCA			0x0020
#define BARCODE_UPCE			0x0040
#define BARCODE_PDF417			0x1000
#define BARCODE_QRCODE			0x2000

//#define OS_Windows	0
#define OS_Windows 1

#ifdef OS_Linux

#ifdef __cplusplus
extern "C" {
#endif
int WOMR_SetConfig(char* cfgfile);
char* WOMR_GetOMRVer();
int WOMR_SetThreshold(int threshold);
void WOMR_SetImageDPI(int dpi);
int WOMR_ResultFile(char* filename,int percent,char* slipid,int* count,int* totalrow,int* totalcolumn);
int WOMR_ResultBuf(void* oimage,int percent,char* slipid,int* count,int* totalrow,int* totalcolumn);
int WOMR_GetResultData(int i, char* data, int* p);

int WOMR_SetBarcodeType(int type);
int WOMR_ResultBarcodeBuf(void* img, int* count);
int WOMR_ResultBarcode(char* file, int* count);
int WOMR_GetBarcodeData(int i,char* data, char* position, char* type);

int WOMR_MergeFileImages(char* filename,char* datafile, char* outfile);
int WOMR_MergeImages(void* _sImage, void* _dataImage, void* _outImage);

#ifdef __cplusplus
}
#endif

//for linux end
#endif	//OS_LINUX

#ifdef OS_Windows

	// for windows start
#ifdef WOMRDLL_EXPORTS
#define WOMRDLL_API __declspec(dllexport)
#else
#define WOMRDLL_API __declspec(dllimport)
#endif

	WOMRDLL_API int __stdcall WOMR_SetConfig(char* cfgfile);
	WOMRDLL_API char* __stdcall WOMR_GetOMRVer();
	WOMRDLL_API int __stdcall WOMR_SetThreshold(int threshold);
	WOMRDLL_API void __stdcall WOMR_SetImageDPI(int dpi);
	WOMRDLL_API int __stdcall WOMR_ResultFile(char* filename, int percent, char* slipid, int* count, int* totalrow, int* totalcolumn);
	WOMRDLL_API int __stdcall WOMR_ResultBuf(void* oimage, int percent, char* slipid, int* count, int* totalrow, int* totalcolumn);
	WOMRDLL_API int __stdcall WOMR_GetResultData(int i, char* data, int* p);

	WOMRDLL_API int __stdcall WOMR_SetBarcodeType(int type);
	WOMRDLL_API int __stdcall WOMR_ResultBarcodeBuf(void* img, int* count);
	WOMRDLL_API int __stdcall WOMR_ResultBarcode(char* file, int* count);
	WOMRDLL_API int __stdcall WOMR_GetBarcodeData(int i, char* data, char* position, char* type);

	WOMRDLL_API int __stdcall WOMR_MergeFileImages(char* filename, char* datafile, char* outfile);
	WOMRDLL_API int __stdcall WOMR_MergeImages(void* _sImage, void* _dataImage, void* _outImage);
#endif	//OS_Windows

#endif	//WOMRDLL
