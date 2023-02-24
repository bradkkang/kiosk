#ifndef __BMPSTRUCT__
#define __BMPSTRUCT__

#define ISBITMAP 0x4d42		// bitmap signature is 0x4d42 : 'BM' => 4d:M 42:M

#ifndef _BYTE_
typedef unsigned char	BYTE;
#endif
#ifndef _WORD_
typedef unsigned short	WORD;
#endif
#ifndef _DWORD_
typedef unsigned long	DWORD;
#endif

typedef struct{
	WORD	bfType;			// bitmap signature 'BM' = 0x4d42
	DWORD	bfSize;			// size of bitmap file in bytes
	WORD	bfReserved1;	// must be set to 0
	WORD	bfReserved2;	// must be set to 0
	DWORD	bfOffBits;		// offset to actual bitmap bit
} BMPFILEHEADER;			// bitmap file header

typedef struct{
	DWORD	biSize;			// size of present structure in bytes
	DWORD	biWidth;		// width of bitmap in pixels
	DWORD	biHeight;		// height of bitmap in pixels
							// (+) for bottom-up, (-) for top-down bitmap
	WORD	biPlanes;		// number of planes, must be set to 1
	WORD	biBitCount;		// bits per pixels : 1, 4, 8, 16, 24, 32
	DWORD	biCompression;	// compression type for bottom-up bitmap
							// 0 : uncompressed  1 : 8bit-RLE  2 : 4bit-RLE
	DWORD	biSizeImage;	// size of uncompressed bitmap 
							// without header information in bytes
	DWORD	biXPelsPerMeter;	// horizontal resolution : in pixels per meter
	DWORD	biYPelsPerMeter;	// vertical resolution   : in pizels per meter
	DWORD	biClrUsed;		// colors used
	DWORD	biClrImportant;	// important color
} BMPINFOHEADER;			// bitmap file info header

// for 1bit, 4bit, and 8bit bitmap
typedef struct{
	BYTE	rgbBlue;		// intensity of blue color
	BYTE	rgbGreen;		// intensity of green color
	BYTE	rgbRed;			// intensity of red color
	BYTE	rgbReserved;	// must be set to 0
} RGBQUAD_;

// for 24bit bitmap ; don't use palette
typedef struct{
	BYTE	rgbBlue;		// intensity of blue color
	BYTE	rgbGreen;		// intensity of green color
	BYTE	rgbRed;			// intensity of red color
} RGB;

typedef struct{
	BMPFILEHEADER	BmpFileHeader;
	BMPINFOHEADER	BmpInfoHeader;

	RGBQUAD_	*pRgbQuad;	// for 1, 4, and 8bit bitmap
	BYTE		*pIndex;

	RGB		*pRgb;		// for 24bit bitmap
} BMPSTRUCT;


#define IMAGE_WIDTH	864

#endif
