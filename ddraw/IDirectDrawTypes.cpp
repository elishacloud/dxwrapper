/**
* Copyright (C) 2019 Elisha Riedlinger
*
* This software is  provided 'as-is', without any express  or implied  warranty. In no event will the
* authors be held liable for any damages arising from the use of this software.
* Permission  is granted  to anyone  to use  this software  for  any  purpose,  including  commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*   1. The origin of this software must not be misrepresented; you must not claim that you  wrote the
*      original  software. If you use this  software  in a product, an  acknowledgment in the product
*      documentation would be appreciated but is not required.
*   2. Altered source versions must  be plainly  marked as such, and  must not be  misrepresented  as
*      being the original software.
*   3. This notice may not be removed or altered from any source distribution.
*/

#include "ddraw.h"

void ConvertColorControl(DDCOLORCONTROL &ColorControl, DDCOLORCONTROL &ColorControl2)
{
	DWORD dwSize = min(sizeof(DDCOLORCONTROL), ColorControl.dwSize);
	ZeroMemory(&ColorControl, dwSize);
	CopyMemory(&ColorControl, &ColorControl2, min(dwSize, ColorControl2.dwSize));
	ColorControl.dwSize = dwSize;
}

void ConvertGammaRamp(DDGAMMARAMP &RampData, DDGAMMARAMP &RampData2)
{
	CopyMemory(&RampData, &RampData2, sizeof(DDGAMMARAMP));
}

void ConvertSurfaceDesc(DDSURFACEDESC &Desc, DDSURFACEDESC &Desc2)
{
	DWORD dwSize = min(sizeof(DDSURFACEDESC), Desc.dwSize);
	DWORD ddpfPixelFormat_dwSize = (dwSize > 88) ? Desc.ddpfPixelFormat.dwSize : 0;
	ZeroMemory(&Desc, dwSize);
	CopyMemory(&Desc, &Desc2, min(dwSize, Desc2.dwSize));
	Desc.dwSize = dwSize;
	if (ddpfPixelFormat_dwSize)
	{
		Desc.ddpfPixelFormat.dwSize = ddpfPixelFormat_dwSize;
	}
}

void ConvertSurfaceDesc(DDSURFACEDESC2 &Desc, DDSURFACEDESC2 &Desc2)
{
	DWORD dwSize = min(sizeof(DDSURFACEDESC2), Desc.dwSize);
	DWORD ddpfPixelFormat_dwSize = (dwSize > 96) ? Desc.ddpfPixelFormat.dwSize : 0;
	ZeroMemory(&Desc, dwSize);
	CopyMemory(&Desc, &Desc2, min(dwSize, Desc2.dwSize));
	Desc.dwSize = dwSize;
	Desc.dwSize = dwSize;
	if (ddpfPixelFormat_dwSize)
	{
		Desc.ddpfPixelFormat.dwSize = ddpfPixelFormat_dwSize;
	}
}

void ConvertSurfaceDesc(DDSURFACEDESC &Desc, DDSURFACEDESC2 &Desc2)
{
	// Prepare varables
	DWORD dwSize = min(sizeof(DDSURFACEDESC), Desc.dwSize);
	ZeroMemory(&Desc, dwSize);
	DDSURFACEDESC tmpDesc = { NULL };
	DDSURFACEDESC2 tmpDesc2 = { NULL };
	CopyMemory(&tmpDesc2, &Desc2, min(sizeof(DDSURFACEDESC2), Desc2.dwSize));
	// Convert varables
	tmpDesc.dwSize = dwSize;
	tmpDesc.dwFlags = tmpDesc2.dwFlags & ~(DDSD_ZBUFFERBITDEPTH | DDSD_TEXTURESTAGE | DDSD_FVF | DDSD_SRCVBHANDLE | DDSD_DEPTH);		// Remove unsupported flags
	tmpDesc.dwHeight = tmpDesc2.dwHeight;
	tmpDesc.dwWidth = tmpDesc2.dwWidth;
	tmpDesc.dwLinearSize = tmpDesc2.dwLinearSize;
	if (tmpDesc2.dwFlags & DDSD_BACKBUFFERCOUNT)
	{
		tmpDesc.dwBackBufferCount = tmpDesc2.dwBackBufferCount;
	}
	if (tmpDesc2.dwFlags & (DDSD_REFRESHRATE | DDSD_MIPMAPCOUNT))
	{
		tmpDesc.dwRefreshRate = tmpDesc2.dwRefreshRate;
	}
	tmpDesc.dwAlphaBitDepth = tmpDesc2.dwAlphaBitDepth;
	tmpDesc.dwReserved = tmpDesc2.dwReserved;
	tmpDesc.lpSurface = tmpDesc2.lpSurface;
	if (tmpDesc2.dwFlags & DDSD_CKSRCOVERLAY)
	{
		tmpDesc.ddckCKDestOverlay.dwColorSpaceLowValue = tmpDesc2.ddckCKDestOverlay.dwColorSpaceLowValue;
		tmpDesc.ddckCKDestOverlay.dwColorSpaceHighValue = tmpDesc2.ddckCKDestOverlay.dwColorSpaceHighValue;
	}
	tmpDesc.ddckCKDestBlt.dwColorSpaceLowValue = tmpDesc2.ddckCKDestBlt.dwColorSpaceLowValue;
	tmpDesc.ddckCKDestBlt.dwColorSpaceHighValue = tmpDesc2.ddckCKDestBlt.dwColorSpaceHighValue;
	tmpDesc.ddckCKSrcOverlay.dwColorSpaceLowValue = tmpDesc2.ddckCKSrcOverlay.dwColorSpaceLowValue;
	tmpDesc.ddckCKSrcOverlay.dwColorSpaceHighValue = tmpDesc2.ddckCKSrcOverlay.dwColorSpaceHighValue;
	tmpDesc.ddckCKSrcBlt.dwColorSpaceLowValue = tmpDesc2.ddckCKSrcBlt.dwColorSpaceLowValue;
	tmpDesc.ddckCKSrcBlt.dwColorSpaceHighValue = tmpDesc2.ddckCKSrcBlt.dwColorSpaceHighValue;
	if (tmpDesc2.dwFlags & DDSD_PIXELFORMAT)
	{
		tmpDesc.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
		ConvertPixelFormat(tmpDesc.ddpfPixelFormat, tmpDesc2.ddpfPixelFormat);
	}
	ConvertCaps(tmpDesc.ddsCaps, tmpDesc2.ddsCaps);
	// Copy to variable
	CopyMemory(&Desc, &tmpDesc, dwSize);
}

void ConvertSurfaceDesc(DDSURFACEDESC2 &Desc2, DDSURFACEDESC &Desc)
{
	// Prepare varables
	DWORD dwSize = min(sizeof(DDSURFACEDESC2), Desc2.dwSize);
	ZeroMemory(&Desc2, dwSize);
	DDSURFACEDESC2 tmpDesc2 = { NULL };
	DDSURFACEDESC tmpDesc = { NULL };
	CopyMemory(&tmpDesc, &Desc, min(sizeof(DDSURFACEDESC), Desc.dwSize));
	// Convert varables
	tmpDesc2.dwSize = dwSize;
	tmpDesc2.dwFlags = tmpDesc.dwFlags & ~(DDSD_ZBUFFERBITDEPTH | DDSD_TEXTURESTAGE | DDSD_FVF | DDSD_SRCVBHANDLE | DDSD_DEPTH);		// Remove unsupported flags
	tmpDesc2.dwHeight = tmpDesc.dwHeight;
	tmpDesc2.dwWidth = tmpDesc.dwWidth;
	tmpDesc2.dwLinearSize = tmpDesc.dwLinearSize;
	tmpDesc2.dwBackBufferCount = tmpDesc.dwBackBufferCount;
	if (tmpDesc.dwFlags & (DDSD_REFRESHRATE | DDSD_MIPMAPCOUNT))
	{
		tmpDesc2.dwRefreshRate = tmpDesc.dwRefreshRate;
	}
	tmpDesc2.dwAlphaBitDepth = tmpDesc.dwAlphaBitDepth;
	tmpDesc2.dwReserved = tmpDesc.dwReserved;
	tmpDesc2.lpSurface = tmpDesc.lpSurface;
	tmpDesc2.ddckCKDestOverlay.dwColorSpaceLowValue = tmpDesc.ddckCKDestOverlay.dwColorSpaceLowValue;
	tmpDesc2.ddckCKDestOverlay.dwColorSpaceHighValue = tmpDesc.ddckCKDestOverlay.dwColorSpaceHighValue;
	tmpDesc2.ddckCKDestBlt.dwColorSpaceLowValue = tmpDesc.ddckCKDestBlt.dwColorSpaceLowValue;
	tmpDesc2.ddckCKDestBlt.dwColorSpaceHighValue = tmpDesc.ddckCKDestBlt.dwColorSpaceHighValue;
	tmpDesc2.ddckCKSrcOverlay.dwColorSpaceLowValue = tmpDesc.ddckCKSrcOverlay.dwColorSpaceLowValue;
	tmpDesc2.ddckCKSrcOverlay.dwColorSpaceHighValue = tmpDesc.ddckCKSrcOverlay.dwColorSpaceHighValue;
	tmpDesc2.ddckCKSrcBlt.dwColorSpaceLowValue = tmpDesc.ddckCKSrcBlt.dwColorSpaceLowValue;
	tmpDesc2.ddckCKSrcBlt.dwColorSpaceHighValue = tmpDesc.ddckCKSrcBlt.dwColorSpaceHighValue;
	tmpDesc2.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
	if (tmpDesc.dwSize == sizeof(DDSURFACEDESC) && tmpDesc.dwFlags & DDSD_PIXELFORMAT && !tmpDesc.ddpfPixelFormat.dwSize)
	{
		tmpDesc.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
	}
	ConvertPixelFormat(tmpDesc2.ddpfPixelFormat, tmpDesc.ddpfPixelFormat);
	ConvertCaps(tmpDesc2.ddsCaps, tmpDesc.ddsCaps);
	// Extra parameters
	tmpDesc2.dwTextureStage = 0;			// Stage identifier that is used to bind a texture to a specific stage
	// Copy to variable
	CopyMemory(&Desc2, &tmpDesc2, dwSize);
}

void ConvertPixelFormat(DDPIXELFORMAT &Format, DDPIXELFORMAT &Format2)
{
	DWORD dwSize = min(sizeof(DDPIXELFORMAT), Format.dwSize);
	ZeroMemory(&Format, dwSize);
	CopyMemory(&Format, &Format2, min(dwSize, Format2.dwSize));
	Format.dwSize = dwSize;
}

void ConvertDeviceIdentifier(DDDEVICEIDENTIFIER2 &DeviceID, DDDEVICEIDENTIFIER2 &DeviceID2)
{
	CopyMemory(&DeviceID, &DeviceID2, sizeof(DDDEVICEIDENTIFIER2));
}

void ConvertDeviceIdentifier(DDDEVICEIDENTIFIER &DeviceID, DDDEVICEIDENTIFIER2 &DeviceID2)
{
	CopyMemory(&DeviceID, &DeviceID2, sizeof(DDDEVICEIDENTIFIER));
}

void ConvertDeviceIdentifier(DDDEVICEIDENTIFIER2 &DeviceID2, DDDEVICEIDENTIFIER &DeviceID)
{
	CopyMemory(&DeviceID2, &DeviceID, sizeof(DDDEVICEIDENTIFIER));
	// Extra parameters
	DeviceID2.dwWHQLLevel = 0;
}

void ConvertDeviceIdentifier(DDDEVICEIDENTIFIER2 &DeviceID2, D3DADAPTER_IDENTIFIER9 &Identifier9)
{
	memcpy(DeviceID2.szDriver, Identifier9.Driver, MAX_DDDEVICEID_STRING);
	memcpy(DeviceID2.szDescription, Identifier9.Description, MAX_DDDEVICEID_STRING);
	DeviceID2.guidDeviceIdentifier = Identifier9.DeviceIdentifier;
	DeviceID2.liDriverVersion.LowPart = Identifier9.DriverVersion.LowPart;
	DeviceID2.liDriverVersion.HighPart = Identifier9.DriverVersion.HighPart;
	DeviceID2.dwDeviceId = Identifier9.DeviceId;
	DeviceID2.dwRevision = Identifier9.Revision;
	DeviceID2.dwSubSysId = Identifier9.SubSysId;
	DeviceID2.dwVendorId = Identifier9.VendorId;
	DeviceID2.dwWHQLLevel = Identifier9.WHQLLevel;
}

void ConvertCaps(DDSCAPS2 &Caps, DDSCAPS2 &Caps2)
{
	Caps.dwCaps = Caps2.dwCaps;
	Caps.dwCaps2 = Caps2.dwCaps2;
	Caps.dwCaps3 = Caps2.dwCaps3;
	Caps.dwCaps4 = Caps2.dwCaps4;
}

void ConvertCaps(DDSCAPS &Caps, DDSCAPS2 &Caps2)
{
	Caps.dwCaps = Caps2.dwCaps;
}

void ConvertCaps(DDSCAPS2 &Caps2, DDSCAPS &Caps)
{
	Caps2.dwCaps = Caps.dwCaps;
	// Extra parameters
	Caps2.dwCaps2 = 0;				// Additional surface capabilities
	Caps2.dwCaps3 = 0;				// Not used
	Caps2.dwCaps4 = 0;				// Not used
}

void ConvertCaps(DDCAPS &Caps, DDCAPS &Caps7)
{
	// Convert caps
	DWORD dwSize = min(sizeof(DDCAPS), Caps.dwSize);
	ZeroMemory(&Caps, dwSize);
	CopyMemory(&Caps, &Caps7, min(dwSize, Caps7.dwSize));
	Caps.dwSize = dwSize;
	if (dwSize > 0x40)
	{
		m_IDirectDrawX::AdjustVidMemory(&Caps.dwVidMemTotal, &Caps.dwVidMemFree);
	}
}

void ConvertCaps(DDCAPS &Caps7, D3DCAPS9 &Caps9)
{
	// Note: dwVidMemTotal and dwVidMemFree are not part of D3DCAPS9 and need to be set separately
	DDCAPS tmpCaps7 = { NULL };
	tmpCaps7.dwSize = min(sizeof(DDCAPS), Caps7.dwSize);

	// General settings
	tmpCaps7.dwPalCaps = DDPCAPS_8BIT | DDPCAPS_ALLOW256;
	tmpCaps7.dwMaxVisibleOverlays = 0x1;
	tmpCaps7.dwNumFourCCCodes = 0x12;
	tmpCaps7.dwRops[6] = 4096;
	tmpCaps7.dwMinOverlayStretch = 0x1;
	tmpCaps7.dwMaxOverlayStretch = 0x4e20;

	// Caps
	tmpCaps7.dwCaps = (Caps9.Caps & (D3DCAPS_OVERLAY | D3DCAPS_READ_SCANLINE)) |
		(/*DDCAPS_3D |*/ DDCAPS_BLT | DDCAPS_BLTQUEUE /*| DDCAPS_BLTFOURCC*/ | DDCAPS_BLTSTRETCH | DDCAPS_GDI /*| DDCAPS_OVERLAYCANTCLIP | DDCAPS_OVERLAYFOURCC |
			DDCAPS_OVERLAYSTRETCH | DDCAPS_ZBLTS*/ | DDCAPS_COLORKEY | DDCAPS_ALPHA | DDCAPS_COLORKEYHWASSIST | DDCAPS_BLTCOLORFILL /*| DDCAPS_BLTDEPTHFILL*/);
	tmpCaps7.dwCaps2 = (Caps9.Caps2 & (D3DCAPS2_FULLSCREENGAMMA | D3DCAPS2_CANCALIBRATEGAMMA | D3DCAPS2_CANMANAGERESOURCE | D3DCAPS2_DYNAMICTEXTURES /*| D3DCAPS2_CANAUTOGENMIPMAP*/)) |
		(DDCAPS2_CANBOBINTERLEAVED | DDCAPS2_CANBOBNONINTERLEAVED | DDCAPS2_NONLOCALVIDMEM | DDCAPS2_WIDESURFACES /*| DDCAPS2_CANFLIPODDEVEN | DDCAPS2_COPYFOURCC*/ |
			DDCAPS2_CANRENDERWINDOWED /*| DDCAPS2_FLIPINTERVAL*/ | DDCAPS2_FLIPNOVSYNC);
	tmpCaps7.dwCKeyCaps = (/*DDCKEYCAPS_DESTOVERLAY |*/ DDCKEYCAPS_SRCBLT /*| DDCKEYCAPS_SRCOVERLAY*/);
	tmpCaps7.dwFXCaps = (DDFXCAPS_BLTARITHSTRETCHY | DDFXCAPS_BLTMIRRORLEFTRIGHT | DDFXCAPS_BLTMIRRORUPDOWN | DDFXCAPS_BLTSHRINKX | DDFXCAPS_BLTSHRINKY | DDFXCAPS_BLTSTRETCHX |
		DDFXCAPS_BLTSTRETCHY /*| DDFXCAPS_OVERLAYARITHSTRETCHY | DDFXCAPS_OVERLAYSHRINKX | DDFXCAPS_OVERLAYSHRINKY | DDFXCAPS_OVERLAYSTRETCHX | DDFXCAPS_OVERLAYSTRETCHY |
		DDFXCAPS_OVERLAYMIRRORLEFTRIGHT | DDFXCAPS_OVERLAYMIRRORUPDOWN | DDFXCAPS_OVERLAYDEINTERLACE*/);
	tmpCaps7.ddsCaps.dwCaps = (DDSCAPS_BACKBUFFER | DDSCAPS_COMPLEX | DDSCAPS_FLIP | DDSCAPS_FRONTBUFFER | DDSCAPS_OFFSCREENPLAIN /*| DDSCAPS_OVERLAY*/ | DDSCAPS_PALETTE |
		DDSCAPS_PRIMARYSURFACE | DDSCAPS_TEXTURE /*| DDSCAPS_3DDEVICE*/ | DDSCAPS_VIDEOMEMORY | /*DDSCAPS_ZBUFFER |*/ DDSCAPS_OWNDC /*| DDSCAPS_MIPMAP*/ | DDSCAPS_LOCALVIDMEM |
		DDSCAPS_NONLOCALVIDMEM);
	ConvertCaps(tmpCaps7.ddsOldCaps, tmpCaps7.ddsCaps);

	// Copy to variable
	CopyMemory(&Caps7, &tmpCaps7, tmpCaps7.dwSize);
}

DWORD GetBitCount(DDPIXELFORMAT ddpfPixelFormat)
{
	if (ddpfPixelFormat.dwRGBBitCount && (ddpfPixelFormat.dwFlags & 
		(DDPF_RGB | DDPF_YUV | DDPF_ALPHA | DDPF_ZBUFFER | DDPF_LUMINANCE | DDPF_BUMPDUDV | DDPF_FOURCC | DDPF_STENCILBUFFER | DDPF_PALETTEINDEXEDTO8)))
	{
		return ddpfPixelFormat.dwRGBBitCount;
	}
	if (ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8)
	{
		return 8;
	}
	if (ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED4)
	{
		return 4;
	}
	if (ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED2)
	{
		return 2;
	}
	if (ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED1)
	{
		return 1;
	}

	LOG_LIMIT(100, __FUNCTION__ << " Error: failed to get BitCount from PixelFormat! " << ddpfPixelFormat);
	return 0;
}

DWORD GetBitCount(D3DFORMAT Format)
{
	switch (Format)
	{
	case D3DFMT_UNKNOWN:
		return 0;
	case D3DFMT_R3G3B2:
	case D3DFMT_A8:
	case D3DFMT_P8:
	case D3DFMT_L8:
	case D3DFMT_A4L4:
		return 8;
	case D3DFMT_R5G6B5:
	case D3DFMT_X1R5G5B5:
	case D3DFMT_A1R5G5B5:
	case D3DFMT_A4R4G4B4:
	case D3DFMT_A8R3G3B2:
	case D3DFMT_X4R4G4B4:
	case D3DFMT_A8P8:
	case D3DFMT_A8L8:
	case D3DFMT_V8U8:
	case D3DFMT_L6V5U5:
	case D3DFMT_D16_LOCKABLE:
	case D3DFMT_D15S1:
	case D3DFMT_D16:
	case D3DFMT_UYVY:
	case D3DFMT_YUY2:
		return 16;
	case D3DFMT_R8G8B8:
		return 24;
	case D3DFMT_A8R8G8B8:
	case D3DFMT_X8R8G8B8:
	case D3DFMT_A2B10G10R10:
	case D3DFMT_A8B8G8R8:
	case D3DFMT_X8B8G8R8:
	case D3DFMT_G16R16:
	case D3DFMT_A2R10G10B10:
	case D3DFMT_X8L8V8U8:
	case D3DFMT_Q8W8V8U8:
	case D3DFMT_V16U16:
	case D3DFMT_A2W10V10U10:
	case D3DFMT_D32:
	case D3DFMT_D24S8:
	case D3DFMT_D24X8:
	case D3DFMT_D24X4S4:
		return 32;
	case D3DFMT_DXT1:
	case D3DFMT_DXT2:
	case D3DFMT_DXT3:
	case D3DFMT_DXT4:
		break;
	}

	LOG_LIMIT(100, __FUNCTION__ << " Display format not Implemented: " << Format);
	return 0;
}

D3DFORMAT GetDisplayFormat(DDPIXELFORMAT ddpfPixelFormat)
{
	if (ddpfPixelFormat.dwFlags & (DDPF_PALETTEINDEXED1 | DDPF_PALETTEINDEXED2 | DDPF_PALETTEINDEXED4))
	{
		LOG_LIMIT(100, __FUNCTION__ << " 1-bit, 2-bit and 4-bit palette formats not Implemented");
		return D3DFMT_UNKNOWN;
	}
	if (ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXEDTO8)
	{
		LOG_LIMIT(100, __FUNCTION__ << " 8-bit indexed to palette format not Implemented");
		return D3DFMT_UNKNOWN;
	}
	if (ddpfPixelFormat.dwFlags & DDPF_STENCILBUFFER)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Stencil buffer format not Implemented");
		return D3DFMT_UNKNOWN;
	}
	if (ddpfPixelFormat.dwFlags & DDPF_YUV)
	{
		LOG_LIMIT(100, __FUNCTION__ << " YUV format not Implemented");
		return D3DFMT_UNKNOWN;
	}
	if (ddpfPixelFormat.dwFlags & DDPF_ZBUFFER)
	{
		LOG_LIMIT(100, __FUNCTION__ << " z-buffer format not Implemented");
		return D3DFMT_UNKNOWN;
	}
	if (ddpfPixelFormat.dwFlags & DDPF_FOURCC)
	{
		switch (ddpfPixelFormat.dwFourCC)
		{
		case D3DFMT_DXT1:
			LOG_LIMIT(100, __FUNCTION__ << " Using format: D3DFMT_DXT1");
			return D3DFMT_DXT1;
		case D3DFMT_DXT2:
			LOG_LIMIT(100, __FUNCTION__ << " Using format: D3DFMT_DXT2");
			return D3DFMT_DXT2;
		case D3DFMT_DXT3:
			LOG_LIMIT(100, __FUNCTION__ << " Using format: D3DFMT_DXT3");
			return D3DFMT_DXT3;
		case D3DFMT_DXT4:
			LOG_LIMIT(100, __FUNCTION__ << " Using format: D3DFMT_DXT4");
			return D3DFMT_DXT4;
		}

		unsigned char ch0 = ddpfPixelFormat.dwFourCC & 0xFF;
		unsigned char ch1 = (ddpfPixelFormat.dwFourCC >> 8) & 0xFF;
		unsigned char ch2 = (ddpfPixelFormat.dwFourCC >> 16) & 0xFF;
		unsigned char ch3 = (ddpfPixelFormat.dwFourCC >> 24) & 0xFF;
		LOG_LIMIT(100, __FUNCTION__ << " FourCC format not Implemented. Code = MAKEFOURCC('" << (char)ch0 << "', '" << (char)ch1 << "', '" << (char)ch2 << "', '" << (char)ch3 << "')");
		return D3DFMT_UNKNOWN;
	}
	if (ddpfPixelFormat.dwFlags & DDPF_ALPHAPREMULT)
	{
		LOG_LIMIT(100, __FUNCTION__ << " premultiplied alpha format not Implemented");
	}
	if (ddpfPixelFormat.dwFlags & DDPF_RGBTOYUV)
	{
		LOG_LIMIT(100, __FUNCTION__ << " RGB to YUV format not Implemented");
	}
	if (ddpfPixelFormat.dwFlags & DDPF_ZPIXELS)
	{
		LOG_LIMIT(100, __FUNCTION__ << " z-pixels not Implemented");
	}

	if (ddpfPixelFormat.dwFlags & (DDPF_RGB | DDPF_PALETTEINDEXED8 | DDPF_LUMINANCE | DDPF_ALPHA))
	{
		DWORD BitCount = GetBitCount(ddpfPixelFormat);
		switch (BitCount)
		{
		case 8:
			if ((ddpfPixelFormat.dwFlags & (DDPF_ALPHAPIXELS | DDPF_LUMINANCE)) == (DDPF_ALPHAPIXELS | DDPF_LUMINANCE))
			{
				return D3DFMT_A4L4;
			}
			if (ddpfPixelFormat.dwFlags & DDPF_ALPHA)
			{
				return D3DFMT_A8;
			}
			if (ddpfPixelFormat.dwFlags & DDPF_LUMINANCE)
			{
				return D3DFMT_L8;
			}
			// Default 8-bit
			return D3DFMT_P8;
		case 16:
			if ((ddpfPixelFormat.dwFlags & (DDPF_ALPHAPIXELS | DDPF_LUMINANCE)) == (DDPF_ALPHAPIXELS | DDPF_LUMINANCE))
			{
				return D3DFMT_A8L8;
			}
			if (ddpfPixelFormat.dwGBitMask == 0x3E0)
			{
				if (ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS)
				{
					return D3DFMT_A1R5G5B5;
				}
				else
				{
					return D3DFMT_X1R5G5B5;
				}
			}
			// Default 16-bit
			return D3DFMT_R5G6B5;
		case 24:
			// Default 24-bit
			return D3DFMT_R8G8B8;
		case 32:
			if (ddpfPixelFormat.dwBBitMask == 0xFF)
			{
				if (ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS)
				{
					return D3DFMT_A8R8G8B8;
				}
			}
			else if (ddpfPixelFormat.dwBBitMask == 0x3FF && ddpfPixelFormat.dwRGBAlphaBitMask == 0xC0000000)
			{
				return D3DFMT_A2R10G10B10;
			}
			// Default 32-bit
			return D3DFMT_X8R8G8B8;
		}

		LOG_LIMIT(100, __FUNCTION__ << " Error: could not find format for BitCount: " << BitCount);
		return D3DFMT_UNKNOWN;
	}

	LOG_LIMIT(100, __FUNCTION__ << " PixelFormat not implemented: " << ddpfPixelFormat);
	return D3DFMT_UNKNOWN;
}

void SetPixelDisplayFormat(D3DFORMAT Format, DDPIXELFORMAT &ddpfPixelFormat)
{
	DWORD dwSize = min(sizeof(DDPIXELFORMAT), ddpfPixelFormat.dwSize);
	DDPIXELFORMAT tmpPixelFormat = { NULL };
	CopyMemory(&tmpPixelFormat, &ddpfPixelFormat, dwSize);
	tmpPixelFormat.dwSize = dwSize;

	tmpPixelFormat.dwFlags = DDPF_RGB;
	tmpPixelFormat.dwRGBBitCount = GetBitCount(Format);
	tmpPixelFormat.dwRGBAlphaBitMask = 0x0;
	tmpPixelFormat.dwRBitMask = 0x0;
	tmpPixelFormat.dwGBitMask = 0x0;
	tmpPixelFormat.dwBBitMask = 0x0;

	// Set BitCount and BitMask
	switch (Format)
	{
	case D3DFMT_R5G6B5:
		tmpPixelFormat.dwRBitMask = 0xF800;
		tmpPixelFormat.dwGBitMask = 0x7E0;
		tmpPixelFormat.dwBBitMask = 0x1F;
		break;
	case D3DFMT_A1R5G5B5:
		tmpPixelFormat.dwFlags |= DDPF_ALPHAPIXELS;
		tmpPixelFormat.dwRGBAlphaBitMask = 0x8000;
	case D3DFMT_X1R5G5B5:
		tmpPixelFormat.dwRBitMask = 0x7C00;
		tmpPixelFormat.dwGBitMask = 0x3E0;
		tmpPixelFormat.dwBBitMask = 0x1F;
		break;
	case D3DFMT_A8R8G8B8:
		tmpPixelFormat.dwFlags |= DDPF_ALPHAPIXELS;
		tmpPixelFormat.dwRGBAlphaBitMask = 0xFF000000;
	case D3DFMT_R8G8B8:
	case D3DFMT_X8R8G8B8:
		tmpPixelFormat.dwRBitMask = 0xFF0000;
		tmpPixelFormat.dwGBitMask = 0xFF00;
		tmpPixelFormat.dwBBitMask = 0xFF;
		break;
	case D3DFMT_A2R10G10B10:
		tmpPixelFormat.dwFlags |= DDPF_ALPHAPIXELS;
		tmpPixelFormat.dwRGBAlphaBitMask = 0xC0000000;
		tmpPixelFormat.dwRBitMask = 0x3FF00000;
		tmpPixelFormat.dwGBitMask = 0xFFC00;
		tmpPixelFormat.dwBBitMask = 0x3FF;
		break;
	case D3DFMT_P8:
		tmpPixelFormat.dwFlags = DDPF_PALETTEINDEXED8;
		break;
	case D3DFMT_A8:
		tmpPixelFormat.dwFlags = DDPF_ALPHA;
		break;
	case D3DFMT_L8:
		tmpPixelFormat.dwFlags = DDPF_LUMINANCE;
		break;
	case D3DFMT_A4L4:
		tmpPixelFormat.dwFlags = DDPF_ALPHAPIXELS | DDPF_LUMINANCE;
		tmpPixelFormat.dwLuminanceAlphaBitMask = 0xF0;
		break;
	case D3DFMT_A8L8:
		tmpPixelFormat.dwFlags = DDPF_ALPHAPIXELS | DDPF_LUMINANCE;
		tmpPixelFormat.dwLuminanceAlphaBitMask = 0xFF00;
		break;
	case D3DFMT_DXT1:
		tmpPixelFormat.dwFlags = DDPF_FOURCC;
		ddpfPixelFormat.dwFourCC = D3DFMT_DXT1;
		break;
	case D3DFMT_DXT2:
		tmpPixelFormat.dwFlags = DDPF_FOURCC;
		ddpfPixelFormat.dwFourCC = D3DFMT_DXT2;
		break;
	case D3DFMT_DXT3:
		tmpPixelFormat.dwFlags = DDPF_FOURCC;
		ddpfPixelFormat.dwFourCC = D3DFMT_DXT3;
		break;
	case D3DFMT_DXT4:
		tmpPixelFormat.dwFlags = DDPF_FOURCC;
		ddpfPixelFormat.dwFourCC = D3DFMT_DXT4;
		break;
	default:
		LOG_LIMIT(100, __FUNCTION__ << " Display format not Implemented: " << Format);
		return;
	}

	// Copy to variable
	CopyMemory(&ddpfPixelFormat, &tmpPixelFormat, dwSize);
}
