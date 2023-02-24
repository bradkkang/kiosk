// AtecScanLib.h : AtecScanLib 의 기본 헤더 파일입니다.
//

#pragma once

#pragma pack(push, 1)

typedef struct {
	int	width;		// image width
	int	height;		// image height
	int info;		// bit-count (1,4,8 bit)
	unsigned char *pbuf;	// image buffer
}IMAGE_INFO;	// defines the image information
#pragma pack(pop)


/***
extern "C" __declspec(dllexport) int ATEC_DeviceOpen(BOOL IsDebugMode);
extern "C" __declspec(dllexport) int ATEC_DeviceClose();
extern "C" __declspec(dllexport) int ATEC_IsDevice();
extern "C" __declspec(dllexport) char* ATEC_GetLibVer();

extern "C" __declspec(dllexport) int ATEC_SaveBMP(int height, char *RawData, char *FileName);

extern "C" __declspec(dllexport) int ATEC_SetPaperOutSpeed(int speed);	//	20141204
extern "C" __declspec(dllexport) int ATEC_Inject(unsigned char dir);
extern "C" __declspec(dllexport) int ATEC_Reject();
extern "C" __declspec(dllexport) int ATEC_IsPaper(int *paper1);

extern "C" __declspec(dllexport) int ATEC_LEDOn();
extern "C" __declspec(dllexport) int ATEC_LEDOff();

extern "C" __declspec(dllexport) int ATEC_GetStatus(unsigned char *status);

extern "C" __declspec(dllexport) int ATEC_SetAdcConfig(unsigned char rlevel, unsigned char speed,
	unsigned char threshold0, unsigned char threshold1);	// only engineer function

extern "C" __declspec(dllexport) int ATEC_SetParam(unsigned char *data, int data_len);			// only engineer function

extern "C" __declspec(dllexport) int ATEC_SetAdcParam1(unsigned char gain, unsigned char offset,
	unsigned char bright, unsigned char contrast);			// only engineer function, only support double CIS scanner

extern "C" __declspec(dllexport) int ATEC_GetAdcConfig(unsigned char *rlevel, unsigned char *speed,
	unsigned char *threshold0, unsigned char *threshold1);	// only engineer function

extern "C" __declspec(dllexport) int ATEC_GetParam(unsigned char *data);	// only engineer function

extern "C" __declspec(dllexport) int ATEC_GetAdcParam1(unsigned char *gain, unsigned char *offset,
	unsigned char *bright, unsigned char *contrast);		// only engineer function, only support double CIS scanner

extern "C" __declspec(dllexport) int ATEC_GetFirmwareVersion(unsigned char *ver);
extern "C" __declspec(dllexport) int ATEC_GetUSBSpeed(unsigned char *speed);

extern "C" __declspec(dllexport) int ATEC_ScannerOn();
extern "C" __declspec(dllexport) int ATEC_ScannerOff();
extern "C" __declspec(dllexport) int ATEC_ScanBuf(int Scan_Len, char *FrontRawBuf, char *RearRawBuf);
extern "C" __declspec(dllexport) int ATEC_Scan(char* filename);
extern "C" __declspec(dllexport) int ATEC_ScanSelectBuf(IMAGE_INFO *simage_down,IMAGE_INFO *simage_up,int select);

extern "C" __declspec(dllexport) int ATEC_SetPixelDataSize(int pixel);
extern "C" __declspec(dllexport) int ATEC_GetPixelDataSize(int *pixel);

extern "C" __declspec(dllexport) int ATEC_SetLineNumber(int LineNumber);
extern "C" __declspec(dllexport) int ATEC_GetLineNumber(int *LineNumber);

extern "C" __declspec(dllexport) int ATEC_SetMotorDir(int dir);
extern "C" __declspec(dllexport) int ATEC_GetMotorDir(int *dir);
extern "C" __declspec(dllexport) int ATEC_MotorStart(char identifier, char dir);
extern "C" __declspec(dllexport) int ATEC_MotorStop(char identifier);

extern "C" __declspec(dllexport) int ATEC_FirmwareUpdate(char *filename);
extern "C" __declspec(dllexport) int ATEC_Reset(int mode);

extern "C" __declspec(dllexport) int ATEC_SetUseHold(int nHold);		
extern "C" __declspec(dllexport) int ATEC_GetUseHold(int *nHold);		

extern "C" __declspec(dllexport) int ATEC_TPHPrint(int IsTestMode, unsigned char Dir, short PositionY, unsigned char *TPH_Data, int TPH_Data_Len);		// only brander option	
extern "C" __declspec(dllexport) int ATEC_TPHPrintDirection(int mode, int pos, int print_dir);		// only brander option	
extern "C" __declspec(dllexport) int ATEC_TPHSaveROM(char* filename);		// only brander option
extern "C" __declspec(dllexport) int ATEC_TPHSaveRAM(char* filename);		// only brander option
extern "C" __declspec(dllexport) int ATEC_TPHSaveRAMBuf(IMAGE_INFO* img);		// only brander option
extern "C" __declspec(dllexport) int ATEC_TPHUp();							// only brander option
extern "C" __declspec(dllexport) int ATEC_TPHDown();						// only brander option

extern "C" __declspec(dllexport) int ATEC_GetSerial(unsigned char *serial);
extern "C" __declspec(dllexport) int ATEC_SetSerial(unsigned char *serial);

extern "C" __declspec(dllexport) int ATEC_GetFirmwareMajorVersion(unsigned char* ver);	// only engineer function
extern "C" __declspec(dllexport) int ATEC_GetFirmwareMinorVersion(unsigned char* ver);	// only engineer function

extern "C" __declspec(dllexport) int ATEC_Calibration();

extern "C" __declspec(dllexport) int ATEC_IRSensorCalibration(unsigned int paper_number, unsigned int paper_tickness, int* error_state);               
extern "C" __declspec(dllexport) int ATEC_SetUseOption(unsigned int use_option0, unsigned int use_option1);
extern "C" __declspec(dllexport) int ATEC_GetUseOption(unsigned int* use_option0, unsigned int* use_option1);

extern "C" __declspec(dllexport) int ATEC_Reject2();
extern "C" __declspec(dllexport) int ATEC_Eject2(unsigned char dir);
extern "C" __declspec(dllexport) int ATEC_SetScanSpeed(int speed);
extern "C" __declspec(dllexport) int ATEC_Barcode(char *rawfile, int height, char* barcode_data, unsigned short *posY);
extern "C" __declspec(dllexport) int ATEC_SolenoidUp(char Type);
extern "C" __declspec(dllexport) int ATEC_SolenoidDown(char Type);
**/

extern "C" __declspec(dllexport) int ATEC_DeviceOpen(BOOL IsDebugMode);
extern "C" __declspec(dllexport) int ATEC_DeviceClose();
extern "C" __declspec(dllexport) int ATEC_IsDevice();
extern "C" __declspec(dllexport) char* ATEC_GetLibVer();

extern "C" __declspec(dllexport) int ATEC_SaveBMP(int width, int height, char *RawData, char *FileName);

extern "C" __declspec(dllexport) int ATEC_SetPaperOutSpeed(int speed);	//	20141204
extern "C" __declspec(dllexport) int ATEC_Inject(unsigned char dir);
extern "C" __declspec(dllexport) int ATEC_Reject();
extern "C" __declspec(dllexport) int ATEC_IsPaper(int *paper1);

extern "C" __declspec(dllexport) int ATEC_LEDOn();
extern "C" __declspec(dllexport) int ATEC_LEDOff();

extern "C" __declspec(dllexport) int ATEC_GetStatus(unsigned char *status);

extern "C" __declspec(dllexport) int ATEC_SetAdcConfig(unsigned char rlevel, unsigned char speed,
	unsigned char threshold0, unsigned char threshold1);	// only engineer function

extern "C" __declspec(dllexport) int ATEC_SetParam(unsigned char *data, int data_len);			// only engineer function

extern "C" __declspec(dllexport) int ATEC_SetAdcParam1(unsigned char gain, unsigned char offset,
	unsigned char bright, unsigned char contrast);			// only engineer function, only support double CIS scanner

extern "C" __declspec(dllexport) int ATEC_GetAdcConfig(unsigned char *rlevel, unsigned char *speed,
	unsigned char *threshold0, unsigned char *threshold1);	// only engineer function

extern "C" __declspec(dllexport) int ATEC_GetParam(unsigned char *data);	// only engineer function

extern "C" __declspec(dllexport) int ATEC_GetAdcParam1(unsigned char *gain, unsigned char *offset,
	unsigned char *bright, unsigned char *contrast);		// only engineer function, only support double CIS scanner

extern "C" __declspec(dllexport) int ATEC_GetFirmwareVersion(unsigned char *ver);
extern "C" __declspec(dllexport) int ATEC_GetUSBSpeed(unsigned char *speed);

extern "C" __declspec(dllexport) int ATEC_ScannerOn();
extern "C" __declspec(dllexport) int ATEC_ScannerOff();
extern "C" __declspec(dllexport) int ATEC_ScanBuf(int Scan_Len, char *FrontRawBuf, char *RearRawBuf);
extern "C" __declspec(dllexport) int ATEC_Scan(char* filename);
extern "C" __declspec(dllexport) int ATEC_ScanSelectBuf(IMAGE_INFO *simage_down,IMAGE_INFO *simage_up,int select);

extern "C" __declspec(dllexport) int ATEC_SetPixelDataSize(int pixel);
extern "C" __declspec(dllexport) int ATEC_GetPixelDataSize(int *pixel);

extern "C" __declspec(dllexport) int ATEC_SetLineNumber(int LineNumber);
extern "C" __declspec(dllexport) int ATEC_GetLineNumber(int *LineNumber);

extern "C" __declspec(dllexport) int ATEC_SetMotorDir(int dir);
extern "C" __declspec(dllexport) int ATEC_GetMotorDir(int *dir);
extern "C" __declspec(dllexport) int ATEC_MotorStart(char identifier, char dir);
extern "C" __declspec(dllexport) int ATEC_MotorStop(char identifier);

extern "C" __declspec(dllexport) int ATEC_FirmwareUpdate(char *filename);
extern "C" __declspec(dllexport) int ATEC_Reset(int mode);

extern "C" __declspec(dllexport) int ATEC_SetUseHold(int nHold);		
extern "C" __declspec(dllexport) int ATEC_GetUseHold(int *nHold);		

extern "C" __declspec(dllexport) int ATEC_TPHPrint(int IsTestMode, unsigned char Dir, short PositionY, unsigned char *TPH_Data, int TPH_Data_Len);		// only brander option	
extern "C" __declspec(dllexport) int ATEC_TPHPrintDirection(int mode, int pos, int print_dir);		// only brander option	
extern "C" __declspec(dllexport) int ATEC_TPHSaveROM(char* filename);		// only brander option
extern "C" __declspec(dllexport) int ATEC_TPHSaveRAM(char* filename);		// only brander option
extern "C" __declspec(dllexport) int ATEC_TPHSaveRAMBuf(IMAGE_INFO* img);		// only brander option
extern "C" __declspec(dllexport) int ATEC_TPHUp();							// only brander option
extern "C" __declspec(dllexport) int ATEC_TPHDown();						// only brander option

extern "C" __declspec(dllexport) int ATEC_GetSerial(unsigned char *serial);
extern "C" __declspec(dllexport) int ATEC_SetSerial(unsigned char *serial);

extern "C" __declspec(dllexport) int ATEC_GetFirmwareMajorVersion(unsigned char* ver);	// only engineer function
extern "C" __declspec(dllexport) int ATEC_GetFirmwareMinorVersion(unsigned char* ver);	// only engineer function

extern "C" __declspec(dllexport) int ATEC_Calibration();

extern "C" __declspec(dllexport) int ATEC_IRSensorCalibration(unsigned int paper_number, unsigned int paper_tickness, int* error_state);               
extern "C" __declspec(dllexport) int ATEC_SetUseOption(unsigned int use_option0, unsigned int use_option1);
extern "C" __declspec(dllexport) int ATEC_GetUseOption(unsigned int* use_option0, unsigned int* use_option1);

extern "C" __declspec(dllexport) int ATEC_Reject2();
extern "C" __declspec(dllexport) int ATEC_Eject2(unsigned char dir);
extern "C" __declspec(dllexport) int ATEC_SetScanSpeed(int speed);
extern "C" __declspec(dllexport) int ATEC_Barcode(char *rawfile, int width, int height, char* barcode_data, unsigned short *posY);
extern "C" __declspec(dllexport) int ATEC_SolenoidUp(char Type);
extern "C" __declspec(dllexport) int ATEC_SolenoidDown(char Type);


