#pragma once

#include <ddraw.h>

#define ISDXTEX(tex) (tex == D3DFMT_DXT1 || tex == D3DFMT_DXT2 || tex == D3DFMT_DXT3 || tex == D3DFMT_DXT4 || tex == D3DFMT_DXT5)

#define D3DFMT_B8G8R8 (D3DFORMAT)19
#define D3DFMT_YV12   MAKEFOURCC('Y','V','1','2')

#define D3DFMT_R5G6B5_TO_X8R8G8B8(w) \
	((((DWORD)((w>>11)&0x1f)*8)<<16)+(((DWORD)((w>>5)&0x3f)*4)<<8)+((DWORD)(w&0x1f)*8))
#define D3DFMT_A8R8G8B8_TO_A4R4G4B4(w) \
	(WORD)(((((w&0xFF000000)>>24)/17)<<12)+((((w&0xFF0000)>>16)/17)<<8)+((((w&0xFF00)>>8)/17)<<4)+(((w&0xFF)/17)))
#define D3DFMT_X8R8G8B8_TO_B8G8R8(w) \
	(((w&0xFF)<<16)+(w&0xFF00)+((w&0xFF0000)>>16))
#define D3DFMT_A8R8G8B8_TO_A8B8G8R8(w) \
	((w&0xFF000000)+((w&0xFF)<<16)+(w&0xFF00)+((w&0xFF0000)>>16))

static constexpr DWORD FourCCTypes[] =
{
	//0x026CFB68, // MAKEFOURCC('N', 'V', '1', '2')
	//0x026CFB6C, // MAKEFOURCC('P', '0', '1', '0')
	//0x026CFB70, // MAKEFOURCC('N', 'V', '2', '4')
	0x026CFB74, // MAKEFOURCC('Y', 'V', '1', '2')
	0x026CFB78, // MAKEFOURCC('U', 'Y', 'V', 'Y')
	0x026CFB7C, // MAKEFOURCC('Y', 'U', 'Y', '2')
	//0x026CFB80, // MAKEFOURCC('A', 'I', '4', '4')
	//0x026CFB84, // MAKEFOURCC('A', 'Y', 'U', 'V')
	//0x026CFB88, // MAKEFOURCC('A', 'I', 'P', '8')
	//0x026CFB8C, // MAKEFOURCC('A', 'V', '1', '2')
	//0x026CFB90, // MAKEFOURCC('P', 'L', 'F', 'F')
	//0x026CFB94, // MAKEFOURCC('N', 'V', 'M', 'D')
	//0x026CFB98, // MAKEFOURCC('N', 'V', 'D', 'P')
	//0x026CFB9C, // MAKEFOURCC('N', 'V', 'D', 'B')
	//0x026CFBA0, // MAKEFOURCC('S', 'S', 'A', 'A')
	//0x026CFBA4, // MAKEFOURCC('A', 'T', 'O', 'C')
	//0x026CFBA8, // MAKEFOURCC('3', 'x', '1', '1')
	//0x026CFBAC, // MAKEFOURCC('3', 'x', '1', '6')
};
static constexpr int NumFourCCs = (sizeof(FourCCTypes) / sizeof(*FourCCTypes));

struct DDS_PIXELFORMAT {
	DWORD dwSize;
	DWORD dwFlags;
	DWORD dwFourCC;
	DWORD dwRGBBitCount;
	DWORD dwRBitMask;
	DWORD dwGBitMask;
	DWORD dwBBitMask;
	DWORD dwABitMask;
};

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

struct DDS_BUFFER
{
	DWORD               dwMagic;
	DDS_HEADER          header;
#pragma warning (suppress : 4200)
	BYTE bdata[];
};

static constexpr DWORD DDS_MAGIC				= 0x20534444; // "DDS "
static constexpr DWORD DDS_HEADER_SIZE			= sizeof(DWORD) + sizeof(DDS_HEADER);
static constexpr DWORD DDS_HEADER_FLAGS_TEXTURE	= 0x00001007; // DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT 
static constexpr DWORD DDS_HEADER_FLAGS_PITCH	= 0x00000008;

inline DWORD ComputePitch(DWORD Width, DWORD BitCount)
{
	return ((((Width * BitCount) + 31) & ~31) >> 3);	// Use Surface Stride for pitch
}

void ConvertColorControl(DDCOLORCONTROL &ColorControl, DDCOLORCONTROL &ColorControl2);
void ConvertGammaRamp(DDGAMMARAMP &RampData, DDGAMMARAMP &RampData2);
void ConvertSurfaceDesc(DDSURFACEDESC &Desc, DDSURFACEDESC &Desc2);
void ConvertSurfaceDesc(DDSURFACEDESC2 &Desc, DDSURFACEDESC2 &Desc2);
void ConvertSurfaceDesc(DDSURFACEDESC &Desc, DDSURFACEDESC2 &Desc2);
void ConvertSurfaceDesc(DDSURFACEDESC2 &Desc2, DDSURFACEDESC &Desc);
void ConvertPixelFormat(DDPIXELFORMAT& Format, DDS_PIXELFORMAT &Format2);
void ConvertPixelFormat(DDPIXELFORMAT &Format, DDPIXELFORMAT &Format2);
void ConvertDeviceIdentifier(DDDEVICEIDENTIFIER2 &DeviceID, DDDEVICEIDENTIFIER2 &DeviceID2);
void ConvertDeviceIdentifier(DDDEVICEIDENTIFIER &DeviceID, DDDEVICEIDENTIFIER2 &DeviceID2);
void ConvertDeviceIdentifier(DDDEVICEIDENTIFIER2 &DeviceID2, DDDEVICEIDENTIFIER &DeviceID);
void ConvertDeviceIdentifier(DDDEVICEIDENTIFIER2 &DeviceID, D3DADAPTER_IDENTIFIER9 &Identifier9);
void ConvertCaps(DDSCAPS2 &Caps, DDSCAPS2 &Caps2);
void ConvertCaps(DDSCAPS &Caps, DDSCAPS2 &Caps2);
void ConvertCaps(DDSCAPS2 &Caps2, DDSCAPS &Caps);
void ConvertCaps(DDCAPS &Caps, DDCAPS &Caps2);
void ConvertCaps(DDCAPS &Caps7, D3DCAPS9 &Caps9);
DWORD GetByteAlignedWidth(DWORD Width, DWORD BitCount);
DWORD GetBitCount(DDPIXELFORMAT ddpfPixelFormat);
DWORD GetBitCount(D3DFORMAT Format);
D3DFORMAT GetDisplayFormat(DDPIXELFORMAT ddpfPixelFormat);
void SetPixelDisplayFormat(D3DFORMAT Format, DDPIXELFORMAT &lpPixelFormat);
HRESULT SetDisplayFormat(DDPIXELFORMAT &ddpfPixelFormat, DWORD BPP);
