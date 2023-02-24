// AtecScanLib.h : AtecScanLib 의 기본 헤더 파일입니다.
//
#pragma once


extern "C" __declspec(dllexport) int ATEC_DeviceOpen(BOOL IsDebugMode);
extern "C" __declspec(dllexport) int ATEC_DeviceClose();
extern "C" __declspec(dllexport) int ATEC_IsDevice();

extern "C" __declspec(dllexport) int ATEC_SaveBMP(int Width, int Height, unsigned char *RawData, char *Path, int barcodeType);

extern "C" __declspec(dllexport) int ATEC_Inject(unsigned char dir);
extern "C" __declspec(dllexport) int ATEC_Reject();

extern "C" __declspec(dllexport) int ATEC_LEDOn();
extern "C" __declspec(dllexport) int ATEC_LEDOff();

extern "C" __declspec(dllexport) int ATEC_GetStatus(unsigned char *status);

extern "C" __declspec(dllexport) int ATEC_SetParam(unsigned char *data, int data_len);			// only engineer function
extern "C" __declspec(dllexport) int ATEC_GetParam(unsigned char *data);	// only engineer function

extern "C" __declspec(dllexport) int ATEC_GetFirmwareVersion(unsigned char *ver);

extern "C" __declspec(dllexport) int ATEC_ScannerOn(unsigned int IsOMR);
extern "C" __declspec(dllexport) int ATEC_ScannerOff();
extern "C" __declspec(dllexport) int ATEC_ScanBuf(int Scan_Len, unsigned char *FrontRawBuf, unsigned char *RearRawBuf);

extern "C" __declspec(dllexport) int ATEC_MotorStart(char identifier, char dir);
extern "C" __declspec(dllexport) int ATEC_MotorStop(char identifier);

extern "C" __declspec(dllexport) int ATEC_Reset(int mode);

extern "C" __declspec(dllexport) int ATEC_TPHBrandInfo(int TPH_Data_Len);		// only brander option	
extern "C" __declspec(dllexport) int ATEC_TPHBrandData(unsigned char *TPH_Data, int Len);
extern "C" __declspec(dllexport) int ATEC_TPHBranding(BYTE dir, unsigned short PositionY);

extern "C" __declspec(dllexport) int ATEC_GetSerial(unsigned char *serial);
extern "C" __declspec(dllexport) int ATEC_SetSerial(unsigned char *serial);

extern "C" __declspec(dllexport) int ATEC_Calibration(unsigned char step);

extern "C" __declspec(dllexport) int ATEC_Barcode(unsigned char *rawfile, int Width, int Height, char* barcode_data, unsigned short *posY);

extern "C" __declspec(dllexport) int ATEC_SendOMR(unsigned char *pBitMap);

extern "C" __declspec(dllexport) int ATEC_Barcode_BMP(char* FileName, char* barcode_data, unsigned short* posY);