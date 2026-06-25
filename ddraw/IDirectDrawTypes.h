#pragma once

#include "ddraw.h"

class m_IDirectDrawX;

static constexpr DWORD MaxVidMemory     = 2047 * 1024 * 1024;	// 2 GBs
static constexpr DWORD MinUsedVidMemory = 8 * 1024;				// 8 KBs

static constexpr D3DFORMAT D9DisplayFormat = D3DFMT_X8R8G8B8;

static constexpr D3DMULTISAMPLE_TYPE D9SampleType = D3DMULTISAMPLE_4_SAMPLES;

static constexpr DWORD MaxPaletteSize = 256;

static constexpr DWORD DXW_ALL_SURFACE_LEVELS = 0xFFFF;

// Indicates surface was created using CreateSurface()
#define DDSCAPS4_CREATESURFACE  0x0001

// Indicates surface is a child of a complex surface
#define DDSCAPS4_COMPLEXCHILD   0x0002

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
#define D3DFMT_AYUV   (D3DFORMAT)MAKEFOURCC('A', 'Y', 'U', 'V')
#define D3DFMT_YV12   (D3DFORMAT)MAKEFOURCC('Y', 'V', '1', '2')
#define D3DFMT_NV12   (D3DFORMAT)MAKEFOURCC('N', 'V', '1', '2')

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
	(D3DFORMAT)MAKEFOURCC('3', 'x', '1', '6'),
	D3DFMT_YUY2,
	D3DFMT_UYVY,
	D3DFMT_AYUV,
};

// Mouse hook
struct MOUSEHOOK
{
	HANDLE ghWriteEvent = nullptr;
	HANDLE threadID = nullptr;
	HHOOK m_hook = nullptr;
	bool bChange = false;
	POINT Pos = {};
};

struct DRAWSTATEBACKUP
{
	DWORD ssMagFilter = 0;
	DWORD ss1addressU = 0;
	DWORD ss1addressV = 0;
	DWORD tsColorOP = 0;
	DWORD tsColorArg1 = 0;
	DWORD tsColorArg2 = 0;
	DWORD tsAlphaOP = 0;
	DWORD rsLighting = 0;
	DWORD rsAlphaBlendEnable = 0;
	DWORD rsAlphaTestEnable = 0;
	DWORD rsFogEnable = 0;
	DWORD rsZEnable = 0;
	DWORD rsZWriteEnable = 0;
	DWORD rsStencilEnable = 0;
	DWORD rsCullMode = 0;
	DWORD rsClipping = 0;
	D3DVIEWPORT9 ViewPort = {};
	D3DMATRIX WorldMatrix = {};
	D3DMATRIX ViewMatrix = {};
	D3DMATRIX ProjectionMatrix = {};
};

struct DISPLAYSETTINGS
{
	HWND hWnd;
	HDC DC;
	m_IDirectDrawX* SetBy;
	DWORD Width;
	DWORD Height;
	DWORD BPP;
	DWORD RefreshRate;
};

struct DEVICESETTINGS
{
	bool IsWindowed;
	bool AntiAliasing;
	bool MultiThreaded;
	bool FPUPreserve;
	bool NoWindowChanges;
	DWORD Width;
	DWORD Height;
	DWORD RefreshRate;
};

struct HIGHRESCOUNTER
{
	LARGE_INTEGER Frequency = {};
	LARGE_INTEGER LastPresentTime = {};
	LONGLONG LastFrameTime = 0;
	DWORD FrameCounter = 0;
	DWORD FrameSkipCounter = 0;
	double PerFrameMS = 1000.0 / 60.0;
};

struct PRESENTTHREAD
{
	const bool& ExitFlag = Config.Exiting;
	bool IsInitialized = false;
	HANDLE workerThread = {};
	HANDLE exitEvent = {};
	LARGE_INTEGER LastPresentTime = {};
};

// Emulated surface
struct EMUSURFACE
{
	HDC DC = nullptr;
	HDC GameDC = nullptr;
	bool UsingGameDC = false;
	DWORD Size = 0;
	D3DFORMAT Format = D3DFMT_UNKNOWN;
	void* pBits = nullptr;
	DWORD Pitch = 0;
	HBITMAP bitmap = nullptr;
	BYTE bmiMemory[(sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256)] = {};
	PBITMAPINFO bmi = (PBITMAPINFO)bmiMemory;
	HGDIOBJ OldDCObject = nullptr;
	HGDIOBJ OldGameDCObject = nullptr;
	DWORD LastPaletteUSN = 0;
};

// Used for 24-bit surfaces
struct TRIBYTE
{
	BYTE first;
	BYTE second;
	BYTE third;

	// Constructor from DWORD
	TRIBYTE(DWORD value)
		: first(BYTE(value & 0xFF)),
		second(BYTE((value >> 8) & 0xFF)),
		third(BYTE((value >> 16) & 0xFF))
	{
	}

	// Default constructor
	TRIBYTE() : first(0), second(0), third(0) {}

	// Conversion operator from TRIBYTE to DWORD
	operator DWORD() const {
		return (DWORD(first) | (DWORD(second) << 8) | (DWORD(third) << 16));
	}

	// Equality operator
	bool operator==(const TRIBYTE& other) const {
		return first == other.first && second == other.second && third == other.third;
	}

	// Inequality operator
	bool operator!=(const TRIBYTE& other) const {
		return !(*this == other);
	}
};

struct ZFormatEntry
{
	DWORD bitDepthFlag;
	std::initializer_list<D3DFORMAT> formats;
};

const ZFormatEntry zFormats[] = {
	{ DDBD_16, { D3DFMT_D16 } },
	{ DDBD_24, { D3DFMT_D24S8, D3DFMT_D24X8 } },
	{ DDBD_32, { D3DFMT_D32 } },
};

void AddDisplayResolution(DWORD Width, DWORD Height);
bool IsDisplayResolution(DWORD Width, DWORD Height);
template <typename T>
void SimpleColorKeyCopy(T ColorKey, T ColorKeyMask, BYTE* SrcBuffer, BYTE* DestBuffer, INT SrcPitch, INT DestPitch, LONG DestRectWidth, LONG DestRectHeight, bool IsColorKey, bool IsMirrorLeftRight);
template <typename T>
void ComplexCopy(T ColorKey, T ColorKeyMask, D3DLOCKED_RECT SrcLockRect, D3DLOCKED_RECT DestLockRect, LONG SrcRectWidth, LONG SrcRectHeight, LONG DestRectWidth, LONG DestRectHeight, bool IsColorKey, bool IsMirrorUpDown, bool IsMirrorLeftRight);
D3DCOLOR ConvertPixelColor(D3DCOLOR PixelColor, const DDPIXELFORMAT& ddpfPixelFormat);
bool HasStencil(D3DFORMAT Format);
DWORD GetDepthColor(float DepthValue, D3DFORMAT Format, DWORD& BPP);
template <typename T>
HRESULT ComplexZBufferCopy(IDirect3DDevice9* d3d9Device, IDirect3DSurface9* pSourceSurfaceD9, RECT SrcRect, RECT DestRect, D3DFORMAT Format);
bool DoRectsMatch(const RECT& lhs, const RECT& rhs);
bool GetOverlappingRect(const RECT& rect1, const RECT& rect2, RECT& outOverlapRect);
bool ClipRectToBounds(RECT* r, LONG width, LONG height);
void ConvertSurfaceDesc(DDSURFACEDESC& Desc, const DDSURFACEDESC2& Desc2);
void ConvertSurfaceDesc(DDSURFACEDESC2& Desc2, const DDSURFACEDESC& Desc);
void ClearUnusedValues(DDSURFACEDESC2& Desc2);
void ConvertDeviceIdentifier(DDDEVICEIDENTIFIER& DeviceID, const DDDEVICEIDENTIFIER2& DeviceID2);
void ConvertDeviceIdentifier(DDDEVICEIDENTIFIER2& DeviceID2, const DDDEVICEIDENTIFIER& DeviceID);
void ConvertDeviceIdentifier(DDDEVICEIDENTIFIER2& DeviceID, const D3DADAPTER_IDENTIFIER9& Identifier9);
void ConvertCaps(DDSCAPS& Caps, const DDSCAPS2& Caps2);
void ConvertCaps(DDSCAPS2& Caps2, const DDSCAPS& Caps);
void ConvertCaps(DDCAPS& Caps, const DDCAPS& Caps2);
void ConvertCaps(DDCAPS& Caps7, D3DCAPS9& Caps9);
void AdjustVidMemory(LPDWORD lpdwTotal, LPDWORD lpdwFree);
DWORD GetByteAlignedWidth(DWORD Width, DWORD BitCount);
DWORD GetMaxMipMapLevel(DWORD Width, DWORD Height);
DWORD GetBitCount(const DDPIXELFORMAT& ddpfPixelFormat);
DWORD GetBitCount(D3DFORMAT Format);
DWORD GetUsedPixelBitsMask(D3DFORMAT Format, DWORD BitCount);
DWORD ComputePitch(D3DFORMAT Format, DWORD Width, DWORD Height);
DWORD GetSurfaceSize(D3DFORMAT Format, DWORD Width, DWORD Height, INT Pitch);
DWORD GetARGBColorKey(DWORD ColorKey, const DDPIXELFORMAT& ddpfPixelFormat);
void GetColorKeyArray(float(&lowColorKey)[4], float(&highColorKey)[4], DWORD lowColorSpace, DWORD highColorSpace, const DDPIXELFORMAT& pixelFormat);
bool IsPixelFormatRGB(const DDPIXELFORMAT& ddpfPixelFormat);
bool IsPixelFormatPalette(const DDPIXELFORMAT& ddpfPixelFormat);
D3DFORMAT ConvertSurfaceFormat(D3DFORMAT Format);
bool IsUnsupportedFormat(D3DFORMAT Format);
D3DFORMAT GetFailoverFormat(D3DFORMAT Format);
D3DFORMAT GetStencilEmulatedFormat(DWORD BitCount);
D3DFORMAT GetDisplayFormat(const DDPIXELFORMAT& ddpfPixelFormat);
void SetPixelDisplayFormat(D3DFORMAT Format, DDPIXELFORMAT& ddpfPixelFormat);
D3DFORMAT SetDisplayFormat(DDPIXELFORMAT& ddpfPixelFormat, DWORD BPP);
