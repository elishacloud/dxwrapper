#pragma once

#include <ddraw.h>

#define BLT_MIRRORLEFTRIGHT		0x00000002l
#define BLT_MIRRORUPDOWN		0x00000004l
#define BLT_COLORKEY			0x00002000l

#define ISDXTEX(tex) (tex == D3DFMT_DXT1 || tex == D3DFMT_DXT2 || tex == D3DFMT_DXT3 || tex == D3DFMT_DXT4 || tex == D3DFMT_DXT5)

// Dummy mipmap needs to have only high bits set
#define DXW_IS_MIPMAP_DUMMY		0xFFFF0000

#define D3DCOLOR_GETALPHA(c)      (((c) >> 24) & 0xFF)
#define D3DCOLOR_GETRED(c)        (((c) >> 16) & 0xFF)
#define D3DCOLOR_GETGREEN(c)      (((c) >> 8) & 0xFF)
#define D3DCOLOR_GETBLUE(c)       ((c) & 0xFF)

#define D3DFMT_B8G8R8 (D3DFORMAT)19
#define D3DFMT_YV12   (D3DFORMAT)MAKEFOURCC('Y','V','1','2')
#define D3DFMT_AYUV   (D3DFORMAT)MAKEFOURCC('A', 'Y', 'U', 'V')

#define D3DFMT_R5G6B5_TO_X8R8G8B8(w) \
	((((DWORD)((w>>11)&0x1f)*8)<<16)+(((DWORD)((w>>5)&0x3f)*4)<<8)+((DWORD)(w&0x1f)*8))
#define D3DFMT_A8R8G8B8_TO_A4R4G4B4(w) \
	(WORD)(((((w&0xFF000000)>>24)/17)<<12)+((((w&0xFF0000)>>16)/17)<<8)+((((w&0xFF00)>>8)/17)<<4)+(((w&0xFF)/17)))
#define D3DFMT_X8R8G8B8_TO_B8G8R8(w) \
	(((w&0xFF)<<16)+(w&0xFF00)+((w&0xFF0000)>>16))
#define D3DFMT_A8R8G8B8_TO_A8B8G8R8(w) \
	((w&0xFF000000)+((w&0xFF)<<16)+(w&0xFF00)+((w&0xFF0000)>>16))

static constexpr D3DFORMAT FourCCTypes[] =
{
	(D3DFORMAT)MAKEFOURCC('N', 'V', '1', '2'),
	(D3DFORMAT)MAKEFOURCC('P', '0', '1', '0'),
	(D3DFORMAT)MAKEFOURCC('N', 'V', '2', '4'),
	(D3DFORMAT)MAKEFOURCC('Y', 'V', '1', '2'),
	(D3DFORMAT)MAKEFOURCC('U', 'Y', 'V', 'Y'),
	(D3DFORMAT)MAKEFOURCC('Y', 'U', 'Y', '2'),
	(D3DFORMAT)MAKEFOURCC('A', 'I', '4', '4'),
	(D3DFORMAT)MAKEFOURCC('A', 'Y', 'U', 'V'),
	(D3DFORMAT)MAKEFOURCC('A', 'I', 'P', '8'),
	(D3DFORMAT)MAKEFOURCC('A', 'V', '1', '2'),
	(D3DFORMAT)MAKEFOURCC('P', 'V', 'R', 'C'),
	(D3DFORMAT)MAKEFOURCC('P', 'L', 'F', 'F'),
	(D3DFORMAT)MAKEFOURCC('N', 'V', 'M', 'D'),
	(D3DFORMAT)MAKEFOURCC('N', 'V', 'D', 'P'),
	(D3DFORMAT)MAKEFOURCC('N', 'V', 'D', 'B'),
	(D3DFORMAT)MAKEFOURCC('N', 'V', 'C', 'S'),
	(D3DFORMAT)MAKEFOURCC('N', 'V', 'H', 'U'),
	(D3DFORMAT)MAKEFOURCC('N', 'V', 'H', 'S'),
	(D3DFORMAT)MAKEFOURCC('N', 'U', 'L', 'L'),
	(D3DFORMAT)MAKEFOURCC('I', 'N', 'T', 'Z'),
	(D3DFORMAT)MAKEFOURCC('S', 'S', 'A', 'A'),
	(D3DFORMAT)MAKEFOURCC('A', 'T', 'O', 'C'),
	(D3DFORMAT)MAKEFOURCC('A', 'T', 'I', '1'),
	(D3DFORMAT)MAKEFOURCC('A', 'T', 'I', '2'),
	(D3DFORMAT)MAKEFOURCC('3', 'x', '1', '1'),
	(D3DFORMAT)MAKEFOURCC('3', 'x', '1', '6')
};

typedef struct {
	DWORD dwSize;
	DWORD dwFlags;
	DWORD dwFourCC;
	DWORD dwRGBBitCount;
	DWORD dwRBitMask;
	DWORD dwGBitMask;
	DWORD dwBBitMask;
	DWORD dwABitMask;
} DDS_PIXELFORMAT;

typedef struct {
	DWORD           dwSize;
	DWORD           dwFlags;
	DWORD           dwHeight;
	DWORD           dwWidth;
	DWORD           dwPitchOrLinearSize;
	DWORD           dwDepth;
	DWORD           dwMipMapCount;
	DWORD           dwReserved1[11];
	DDS_PIXELFORMAT ddspf;
	DWORD           dwCaps;
	DWORD           dwCaps2;
	DWORD           dwCaps3;
	DWORD           dwCaps4;
	DWORD           dwReserved2;
} DDS_HEADER;

typedef struct {
	DWORD               dwMagic;
	DDS_HEADER          header;
#pragma warning (suppress : 4200)
	BYTE bdata[];
} DDS_BUFFER;

// Used for 24-bit surfaces
struct TRIBYTE
{
	BYTE first;
	BYTE second;
	BYTE third;
};

static constexpr DWORD DDS_MAGIC				= 0x20534444; // "DDS "
static constexpr DWORD DDS_HEADER_SIZE			= sizeof(DWORD) + sizeof(DDS_HEADER);
static constexpr DWORD DDS_HEADER_FLAGS_TEXTURE	= 0x00001007; // DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT 
static constexpr DWORD DDS_HEADER_FLAGS_PITCH	= 0x00000008;

static constexpr DWORD MaxPaletteSize = 256;

inline DWORD ComputePitch(DWORD Width, DWORD BitCount)
{
	return ((((Width * BitCount) + 31) & ~31) >> 3);	// Use Surface Stride for pitch
}

void ConvertSurfaceDesc(DDSURFACEDESC &Desc, DDSURFACEDESC2 &Desc2);
void ConvertSurfaceDesc(DDSURFACEDESC2 &Desc2, DDSURFACEDESC &Desc);
void ConvertPixelFormat(DDPIXELFORMAT& Format, DDS_PIXELFORMAT &Format2);
void ConvertDeviceIdentifier(DDDEVICEIDENTIFIER &DeviceID, DDDEVICEIDENTIFIER2 &DeviceID2);
void ConvertDeviceIdentifier(DDDEVICEIDENTIFIER2 &DeviceID2, DDDEVICEIDENTIFIER &DeviceID);
void ConvertDeviceIdentifier(DDDEVICEIDENTIFIER2 &DeviceID, D3DADAPTER_IDENTIFIER9 &Identifier9);
void ConvertCaps(DDSCAPS &Caps, DDSCAPS2 &Caps2);
void ConvertCaps(DDSCAPS2 &Caps2, DDSCAPS &Caps);
void ConvertCaps(DDCAPS &Caps, DDCAPS &Caps2);
void ConvertCaps(DDCAPS &Caps7, D3DCAPS9 &Caps9);
DWORD GetByteAlignedWidth(DWORD Width, DWORD BitCount);
DWORD GetMaxMipMapLevel(DWORD Width, DWORD Height);
DWORD GetBitCount(DDPIXELFORMAT ddpfPixelFormat);
DWORD GetBitCount(D3DFORMAT Format);
float ConvertDepthValue(DWORD dwFillDepth, D3DFORMAT Format);
DWORD GetSurfaceSize(D3DFORMAT Format, DWORD Width, DWORD Height, INT Pitch);
DWORD GetARGBColorKey(DWORD ColorKey, DDPIXELFORMAT& pixelFormat);
void GetColorKeyArray(float(&lowColorKey)[4], float(&highColorKey)[4], DWORD lowColorSpace, DWORD highColorSpace, DDPIXELFORMAT& pixelFormat);
D3DFORMAT ConvertSurfaceFormat(D3DFORMAT Format);
D3DFORMAT GetFailoverFormat(D3DFORMAT Format);
D3DFORMAT GetDisplayFormat(DDPIXELFORMAT ddpfPixelFormat);
void SetPixelDisplayFormat(D3DFORMAT Format, DDPIXELFORMAT &lpPixelFormat);
HRESULT SetDisplayFormat(DDPIXELFORMAT &ddpfPixelFormat, DWORD BPP);
