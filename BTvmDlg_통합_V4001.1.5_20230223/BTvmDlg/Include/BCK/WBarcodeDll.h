
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the WBARCODEDLL_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// WBARCODEDLL_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef WBARCODEDLL_EXPORTS
#define WBARCODEDLL_API __declspec(dllexport)
#else
#define WBARCODEDLL_API __declspec(dllimport)
#endif

// barcode type
#define	BAR_CODE39				0x0001	// "CODE39"
#define	BAR_CODE128				0x0002	// "CODE128"
#define	BAR_CODE25NI			0x0004	// "CODE-2-of-5 Non-Interleaved"
#define	BAR_CODE25				0x0008	// "CODE-2-of-5 Interleaved"
#define	BAR_CODDBAR				0x0010	// "CODABAR"
#define	BAR_EAN8				0x0020	// "EAN8"
#define	BAR_EAN13				0x0040	// "EAN13"
#define	BAR_UPCA				0x0080	// "UPCA"
#define	BAR_UPCE				0x0100	// "UPCE"
#define	BAR_PATCHCODES			0x0200	// "PATCH"
#define	BAR_EXTENDEDCODE39		0x0400	// 
#define	BAR_PDF417				0x0800	// 2D

// barcode direction
#define	BAR_DIR_LEFTTORIGHT				0x0001	// Left to Right
#define	BAR_DIR_TOPTOBOTTOM				0x0002	// Top to Bottom
#define	BAR_DIR_RIGHTTOLEFT				0x0004	// Right To Left
#define	BAR_DIR_BOTTOMTOTOP				0x0008	// Bottom to Top
#define	BAR_DIR_TOPLEFTTOBOTTOMRIGHT	0x0010	// Top Left to Bottom Right 
#define	BAR_DIR_TOPRIGHTTOBOTTOMLEFT	0x0020	// Top Right to Bottom Left
#define	BAR_DIR_BOTTOMRIGHTTOTOPLEFT	0x0040	// Bottom Right to Top Left
#define	BAR_DIR_BOTTOMLEFTTOTOPRIGHT	0x0080	// Bottom Left to Top Right


#define	W_OK					0
#define	W_FAIL_FILE				-1	// Error opening file 
#define	W_FAIL_MULTIPLAN		-2	// File is Multi Plane 
#define	W_FAIL_BPS				-3	// Invalid number of bits per sample 
#define	W_FAIL_MEMORY			-4	// Memory allocation error 
#define	W_FAIL_TIF				-5	// Invalid TIF photometric value  
#define	W_FAIL_LOCK				-6	// demo version 
#define	W_FAIL_NODATA			-7
#define	W_FAIL					-8	//  

#define	W_DEGREE_5			0 //  up to 5 degrees  
#define	W_DEGREE_13		1 // 13 degrees 
#define	W_DEGREE_21		2 // 21 degrees  
#define	W_DEGREE_29		3 // 29 degrees 
#define	W_DEGREE_37		4 // 37 degrees 
#define	W_DEGREE_45		5 // 45 degrees

//WBARCODEDLL_API int __stdcall WScanBarCodeFromDIB(long hDIB,int* count); 
//WBARCODEDLL_API int __stdcall WScanBarCodeFromBitmap(long hBitmap,int* count); 
WBARCODEDLL_API int __stdcall WScanBarCodeFromFile(char* filename,int* count);
WBARCODEDLL_API int __stdcall WSetScanRect(long TopLeftX, long TopLeftY, long BottomRightX, long BottomRightY, short nMappingMode); 
WBARCODEDLL_API int __stdcall WGetBarString(short index,char* data); 
WBARCODEDLL_API int __stdcall WGetBarStringPos(short index, long* pTopLeftX, long* pTopLeftY, long* pBotRightX, long* pBotRightY); 
WBARCODEDLL_API int __stdcall WGetBarStringType(short index,char* type); 
WBARCODEDLL_API int __stdcall WGetBarStringDirection(short index,int* direction); 

WBARCODEDLL_API int __stdcall WSetReadBarcodeType(int type);
WBARCODEDLL_API int __stdcall WGetReadBarcodeType(int* type);
WBARCODEDLL_API int __stdcall WSetBarcodeDirection(int direction); 
WBARCODEDLL_API int __stdcall WGetBarcodeDirection(int* direction); 
WBARCODEDLL_API int __stdcall WSetSkewTolerance(int tolerance); 
WBARCODEDLL_API int __stdcall WGetSkewTolerance(int* tolerance); 
WBARCODEDLL_API int __stdcall WSetColorThreshold(int threshold);
WBARCODEDLL_API int __stdcall WGetColorThreshold(int* threshold);
WBARCODEDLL_API int __stdcall WSetShowCheckDigit(int show);
WBARCODEDLL_API int __stdcall WGetShowCheckDigit(int* show);
WBARCODEDLL_API int __stdcall WSetMultipleRead(int multiple);
WBARCODEDLL_API int __stdcall WGetMultipleRead(int* multiple);
WBARCODEDLL_API int __stdcall WSetCode39Checksum(int Checksum);
WBARCODEDLL_API int __stdcall WGetCode39Checksum(int* Checksum);
WBARCODEDLL_API char* __stdcall WGetLibVer();
WBARCODEDLL_API int __stdcall WSetMinLength(int length);	// 20140731
WBARCODEDLL_API int __stdcall WGetMinLength(int* length);	// 20140731
WBARCODEDLL_API int __stdcall WSetMaxLength(int length);	// 20140731
WBARCODEDLL_API int __stdcall WGetMaLength(int* length);	// 20140731
