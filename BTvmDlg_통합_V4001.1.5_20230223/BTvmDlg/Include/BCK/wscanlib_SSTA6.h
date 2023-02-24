#ifndef WSCANLIB_H
#define WSCANLIB_H

#define WT_STATUS_SENSOR1	0x01
#define WT_STATUS_SENSOR2	0x02

#define WT_STATUS_SCAN_START_OK		0x40
#define WT_STATUS_SCAN_START_OK0	0x80
#define WT_STATUS_SCAN_START_OK1	0x100

#define WT_STATUS_JAM				0x200

#define WT_SUCCESS					0
#define WT_ERR_MUTEX				1
#define WT_ERR_UNKOWNDEVICE			2
#define WT_ERR_DEVICEALREADY		3
#define WT_ERR_DEVICEFAIL			4
#define WT_ERR_UNKOWNSPEED			5
#define WT_ERR_ENABLESCANNER		6
#define WT_ERR_NOTSUPPORT			7
#define WT_ERR_PARAMETER			8
#define WT_ERR_FIRMWAREFILE			9
#define WT_ERR_IMAGEFILE			10
#define WT_ERR_IMAGEBUF				11
#define WT_ERR_IMAGEDATA			12
#define WT_ERR_CALIBRATIONGAIN		13
#define WT_ERR_CALIBRATIONOFFSET	14
#define WT_ERR_NOPAPER				15
#define WT_ERR_MOTTORISWORKING		16
#define WT_ERR_IRCALIBRATION		17



#ifndef MY_IMAGE_STRUCT
#define MY_IMAGE_STRUCT 
typedef struct {
	int	width;		// image width
	int	height;		// image height
	int info;		// bit-count (1,4,8 bit)
	unsigned char *pbuf;	// image buffer
}MY_IMAGE;	// defines the image information
#endif

#define OS_Windows 	1
//#define OS_Linux	1

#ifdef OS_Windows

// for windows start
#ifdef WNSCANDLL_EXPORTS
#define WNSCANDLL_API  __declspec(dllexport)
#else
#define WNSCANDLL_API  __declspec(dllimport)
#endif


#ifdef __cplusplus
extern "C" {
#endif

WNSCANDLL_API int __stdcall WT_DeviceOpen();
WNSCANDLL_API int __stdcall WT_DeviceClose();
WNSCANDLL_API int __stdcall WT_IsDevice();
WNSCANDLL_API char* __stdcall WT_GetLibVer();

WNSCANDLL_API int __stdcall WT_SaveBMP(char *filename, MY_IMAGE *simage);

WNSCANDLL_API int __stdcall WT_SetPaperOutSpeed(int speed);	//	20141204
WNSCANDLL_API int __stdcall WT_Eject();
WNSCANDLL_API int __stdcall WT_Reject();
WNSCANDLL_API int __stdcall WT_IsPaper(int *paper1);

WNSCANDLL_API int __stdcall WT_LEDOn();
WNSCANDLL_API int __stdcall WT_LEDOff();

WNSCANDLL_API int __stdcall WT_GetStatus(int* status);	

WNSCANDLL_API int __stdcall WT_SetAdcConfig(unsigned char rlevel, unsigned char speed,
					unsigned char threshold0, unsigned char threshold1);	// only engineer function

WNSCANDLL_API int __stdcall WT_SetAdcParam0(unsigned char gain, unsigned char offset, 
					unsigned char bright, unsigned char contrast);			// only engineer function

WNSCANDLL_API int __stdcall WT_SetAdcParam1(unsigned char gain, unsigned char offset,
					unsigned char bright, unsigned char contrast);			// only engineer function, only support double CIS scanner
			
WNSCANDLL_API int __stdcall WT_GetAdcConfig(unsigned char *rlevel, unsigned char *speed,
					unsigned char *threshold0, unsigned char *threshold1);	// only engineer function

WNSCANDLL_API int __stdcall WT_GetAdcParam0(unsigned char *gain,	 unsigned char *offset, 
					unsigned char *bright,  unsigned char *contrast);	// only engineer function

WNSCANDLL_API int __stdcall WT_GetAdcParam1(unsigned char *gain, unsigned char *offset,
					unsigned char *bright, unsigned char *contrast);		// only engineer function, only support double CIS scanner

WNSCANDLL_API int __stdcall WT_GetFirmwareVersion(unsigned char *ver);
WNSCANDLL_API int __stdcall WT_GetUSBSpeed(unsigned char *speed);

WNSCANDLL_API int __stdcall WT_ScannerOn();
WNSCANDLL_API int __stdcall WT_ScannerOff();
WNSCANDLL_API int __stdcall WT_ScanBuf(MY_IMAGE *simage);
WNSCANDLL_API int __stdcall WT_Scan(char* filename);
WNSCANDLL_API int __stdcall WT_ScanSelectBuf(MY_IMAGE *simage_down,MY_IMAGE *simage_up,int select);

WNSCANDLL_API int __stdcall WT_SetPixelDataSize(int pixel);
WNSCANDLL_API int __stdcall WT_GetPixelDataSize(int *pixel);

WNSCANDLL_API int __stdcall WT_SetLineNumber(int LineNumber);
WNSCANDLL_API int __stdcall WT_GetLineNumber(int *LineNumber);

WNSCANDLL_API int __stdcall WT_SetMotorDir(int dir);
WNSCANDLL_API int __stdcall WT_GetMotorDir(int *dir);
WNSCANDLL_API int __stdcall WT_MotorStart();
WNSCANDLL_API int __stdcall WT_MotorStop();

WNSCANDLL_API int __stdcall WT_FirmwareUpdate(char *filename);
WNSCANDLL_API int __stdcall WT_Reset();		

WNSCANDLL_API int __stdcall WT_SetUseHold(int nHold);		
WNSCANDLL_API int __stdcall WT_GetUseHold(int *nHold);		

WNSCANDLL_API int __stdcall WT_TPHPrint(int mode, int pos);		// only brander option	
WNSCANDLL_API int __stdcall WT_TPHPrintDirection(int mode, int pos, int print_dir);		// only brander option	
WNSCANDLL_API int __stdcall WT_TPHSaveROM(char* filename);		// only brander option
WNSCANDLL_API int __stdcall WT_TPHSaveRAM(char* filename);		// only brander option
WNSCANDLL_API int __stdcall WT_TPHSaveRAMBuf(MY_IMAGE* img);		// only brander option
WNSCANDLL_API int __stdcall WT_TPHUp();							// only brander option
WNSCANDLL_API int __stdcall WT_TPHDown();						// only brander option

WNSCANDLL_API int __stdcall WT_GetSerial(unsigned char *serial);
WNSCANDLL_API int __stdcall WT_SetSerial(unsigned char *serial);

WNSCANDLL_API int __stdcall WT_GetFirmwareMajorVersion(unsigned char* ver);	// only engineer function
WNSCANDLL_API int __stdcall WT_GetFirmwareMinorVersion(unsigned char* ver);	// only engineer function

WNSCANDLL_API int __stdcall  WT_Calibration();

//20141217
WNSCANDLL_API int __stdcall WT_IRSensorCalibration(unsigned int paper_number, unsigned int paper_tickness, int* error_state);               
WNSCANDLL_API int __stdcall WT_SetUseOption(unsigned int use_option0, unsigned int use_option1);
WNSCANDLL_API int __stdcall WT_GetUseOption(unsigned int* use_option0, unsigned int* use_option1);

//20150916
WNSCANDLL_API int __stdcall WT_Reject2(unsigned char hold_onoff);
WNSCANDLL_API int __stdcall WT_Eject2(unsigned char hold_onoff);
// 20151209
WNSCANDLL_API int __stdcall WT_SetScanSpeed(int speed);

#ifdef __cplusplus
}
#endif

#else

int WT_DeviceOpen();
int WT_DeviceClose();
int WT_IsDevice();
char* WT_GetLibVer();

int WT_SaveBMP(char *filename, MY_IMAGE *simage);

int WT_SetPaperOutSpeed(int speed);	//	20141204
int WT_Eject();
int WT_Reject();
int WT_IsPaper(int *paper1);

int WT_LEDOn();
int WT_LEDOff();

int WT_GetStatus(int* status);	

int WT_SetAdcConfig(unsigned char rlevel, unsigned char speed,
					unsigned char threshold0, unsigned char threshold1);	// only engineer function

int WT_SetAdcParam0(unsigned char gain, unsigned char offset, 
					unsigned char bright, unsigned char contrast);		// only engineer function

int WT_SetAdcParam1(unsigned char gain, unsigned char offset,
					unsigned char bright, unsigned char contrast);		// only engineer function, only support double CIS scanner
			
int WT_GetAdcConfig(unsigned char *rlevel, unsigned char *speed,
					unsigned char *threshold0, unsigned char *threshold1);	// only engineer function

int WT_GetAdcParam0(unsigned char *gain,	 unsigned char *offset, 
					unsigned char *bright,  unsigned char *contrast);	// only engineer function

int WT_GetAdcParam1(unsigned char *gain, unsigned char *offset,
					unsigned char *bright, unsigned char *contrast);	// only engineer function, only support double CIS scanner

int WT_GetFirmwareVersion(unsigned char *ver);
int WT_GetUSBSpeed(unsigned char *speed);

int WT_ScannerOn();
int WT_ScannerOff();
int WT_ScanBuf(MY_IMAGE *simage);
int WT_Scan(char* filename);
int WT_ScanSelectBuf(MY_IMAGE *simage_down,MY_IMAGE *simage_up,int select);

int WT_SetPixelDataSize(int pixel);
int WT_GetPixelDataSize(int *pixel);

int WT_SetLineNumber(int LineNumber);
int WT_GetLineNumber(int *LineNumber);

int WT_SetMotorDir(int dir);
int WT_GetMotorDir(int *dir);
int WT_MotorStart();
int WT_MotorStop();

int WT_FirmwareUpdate(char *filename);
int WT_Reset();		

int WT_SetUseHold(int nHold);			
int WT_GetUseHold(int *nHold);			

int WT_TPHPrint(int mode, int pos);		// only brander option	
int WT_TPHPrintDirection(int mode, int pos, int print_dir);		// only brander option	
int WT_TPHSaveROM(char* filename);		// only brander option
int WT_TPHSaveRAM(char* filename);		// only brander option
int WT_TPHSaveRAMBuf(MY_IMAGE* img);		// only brander option
int WT_TPHUp();							// only brander option
int WT_TPHDown();						// only brander option

int WT_GetSerial(unsigned char *serial);
int WT_SetSerial(unsigned char *serial);

int WT_GetFirmwareMajorVersion(unsigned char* ver);	// only engineer function
int WT_GetFirmwareMinorVersion(unsigned char* ver);	// only engineer function

int  WT_Calibration();

//20141217
int WT_IRSensorCalibration(unsigned int paper_number, unsigned int paper_tickness, int* error_state);              
int WT_SetUseOption(unsigned int use_option0, unsigned int use_option1);
int WT_GetUseOption(unsigned int* use_option0, unsigned int* use_option1);

//20150916
int WT_Reject2(unsigned char hold_onoff);
int WT_Eject2(unsigned char hold_onoff);
// 20151209
int WT_SetScanSpeed(int speed);

#endif
#endif
