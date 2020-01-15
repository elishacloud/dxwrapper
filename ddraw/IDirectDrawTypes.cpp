/**
* Copyright (C) 2020 Elisha Riedlinger
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
	if (ColorControl.dwSize != sizeof(DDCOLORCONTROL) || ColorControl.dwSize != sizeof(DDCOLORCONTROL))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error! Incorrect dwSize: " << ColorControl.dwSize << " " << ColorControl.dwSize);
		return;
	}
	CopyMemory(&ColorControl, &ColorControl2, sizeof(DDCOLORCONTROL));
}

void ConvertGammaRamp(DDGAMMARAMP &RampData, DDGAMMARAMP &RampData2)
{
	CopyMemory(&RampData, &RampData2, sizeof(DDGAMMARAMP));
}

void ConvertSurfaceDesc(DDSURFACEDESC &Desc, DDSURFACEDESC &Desc2)
{
	if (Desc.dwSize != sizeof(DDSURFACEDESC) || Desc2.dwSize != sizeof(DDSURFACEDESC))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error! Incorrect dwSize: " << Desc.dwSize << " " << Desc2.dwSize);
		return;
	}
	CopyMemory(&Desc, &Desc2, sizeof(DDSURFACEDESC));
}

void ConvertSurfaceDesc(DDSURFACEDESC2 &Desc, DDSURFACEDESC2 &Desc2)
{
	if (Desc.dwSize != sizeof(DDSURFACEDESC2) || Desc2.dwSize != sizeof(DDSURFACEDESC2))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error! Incorrect dwSize: " << Desc.dwSize << " " << Desc2.dwSize);
		return;
	}
	CopyMemory(&Desc, &Desc2, sizeof(DDSURFACEDESC2));
}

void ConvertSurfaceDesc(DDSURFACEDESC &Desc, DDSURFACEDESC2 &Desc2)
{
	// Check for supported dwSize
	if (Desc.dwSize != sizeof(DDSURFACEDESC) || Desc2.dwSize != sizeof(DDSURFACEDESC2))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error! Incorrect dwSize: " << Desc.dwSize << " " << Desc2.dwSize);
		return;
	}
	// Prepare destination structure
	ZeroMemory(&Desc, sizeof(DDSURFACEDESC));
	Desc.dwSize = sizeof(DDSURFACEDESC);
	// Convert varables
	Desc.dwFlags = Desc2.dwFlags & ~(DDSD_ZBUFFERBITDEPTH | DDSD_TEXTURESTAGE | DDSD_FVF | DDSD_SRCVBHANDLE | DDSD_DEPTH);		// Remove unsupported flags
	Desc.dwHeight = Desc2.dwHeight;
	Desc.dwWidth = Desc2.dwWidth;
	Desc.lPitch = Desc2.lPitch;
	Desc.dwBackBufferCount = Desc2.dwBackBufferCount;
	Desc.dwRefreshRate = Desc2.dwRefreshRate;
	Desc.dwAlphaBitDepth = Desc2.dwAlphaBitDepth;
	Desc.dwReserved = Desc2.dwReserved;
	Desc.lpSurface = Desc2.lpSurface;
	Desc.ddckCKDestOverlay = Desc2.ddckCKDestOverlay;
	Desc.ddckCKDestBlt = Desc2.ddckCKDestBlt;
	Desc.ddckCKSrcOverlay = Desc2.ddckCKSrcOverlay;
	Desc.ddckCKSrcBlt = Desc2.ddckCKSrcBlt;
	if (Desc.dwFlags & DDSD_PIXELFORMAT)
	{
		Desc.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
		Desc2.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
		ConvertPixelFormat(Desc.ddpfPixelFormat, Desc2.ddpfPixelFormat);
	}
	ConvertCaps(Desc.ddsCaps, Desc2.ddsCaps);
	// Check for dwFlags that did not get converted
	if (Desc.dwFlags != Desc2.dwFlags)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error! Removing unsupported flags: " << (Desc2.dwFlags & ~Desc.dwFlags));
	}
}

void ConvertSurfaceDesc(DDSURFACEDESC2 &Desc2, DDSURFACEDESC &Desc)
{
	// Check for supported dwSize
	if (Desc2.dwSize != sizeof(DDSURFACEDESC2) || Desc.dwSize != sizeof(DDSURFACEDESC))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error! Incorrect dwSize: " << Desc2.dwSize << " " << Desc.dwSize);
		return;
	}
	// Prepare destination structure
	ZeroMemory(&Desc2, sizeof(DDSURFACEDESC2));
	Desc2.dwSize = sizeof(DDSURFACEDESC2);
	// Convert varables
	Desc2.dwFlags = Desc.dwFlags & ~(DDSD_ZBUFFERBITDEPTH | DDSD_TEXTURESTAGE | DDSD_FVF | DDSD_SRCVBHANDLE | DDSD_DEPTH);		// Remove unsupported flags
	Desc2.dwHeight = Desc.dwHeight;
	Desc2.dwWidth = Desc.dwWidth;
	Desc2.lPitch = Desc.lPitch;
	Desc2.dwBackBufferCount = Desc.dwBackBufferCount;
	Desc2.dwRefreshRate = Desc.dwRefreshRate;
	Desc2.dwAlphaBitDepth = Desc.dwAlphaBitDepth;
	Desc2.dwReserved = Desc.dwReserved;
	Desc2.lpSurface = Desc.lpSurface;
	Desc2.ddckCKDestOverlay = Desc.ddckCKDestOverlay;
	Desc2.ddckCKDestBlt = Desc.ddckCKDestBlt;
	Desc2.ddckCKSrcOverlay = Desc.ddckCKSrcOverlay;
	Desc2.ddckCKSrcBlt = Desc.ddckCKSrcBlt;
	if (Desc2.dwFlags & DDSD_PIXELFORMAT)
	{
		Desc.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
		Desc2.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
		ConvertPixelFormat(Desc2.ddpfPixelFormat, Desc.ddpfPixelFormat);
	}
	ConvertCaps(Desc2.ddsCaps, Desc.ddsCaps);
	// Extra parameters
	Desc2.dwTextureStage = 0;			// Stage identifier that is used to bind a texture to a specific stage
										// Check for dwFlags that did not get converted
	if (Desc2.dwFlags != Desc.dwFlags)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error! Removing unsupported flags: " << (Desc.dwFlags & ~Desc2.dwFlags));
	}
}

void ConvertPixelFormat(DDPIXELFORMAT &Format, DDPIXELFORMAT &Format2)
{
	if (Format.dwSize != sizeof(DDPIXELFORMAT) || Format2.dwSize != sizeof(DDPIXELFORMAT))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error! Incorrect dwSize: " << Format.dwSize << " " << Format2.dwSize);
		return;
	}
	CopyMemory(&Format, &Format2, sizeof(DDPIXELFORMAT));
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
	Caps.dwVolumeDepth = Caps2.dwVolumeDepth;
}

void ConvertCaps(DDSCAPS &Caps, DDSCAPS2 &Caps2)
{
	Caps.dwCaps = Caps2.dwCaps;
	// Check for dwFlags that did not get converted
	if (Caps2.dwCaps2 || Caps2.dwVolumeDepth)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error! Removing unsupported dwCaps2 and dwVolumeDepth: " << Caps2.dwCaps2 << " " << Caps2.dwVolumeDepth);
	}
}

void ConvertCaps(DDSCAPS2 &Caps2, DDSCAPS &Caps)
{
	Caps2.dwCaps = Caps.dwCaps;
	// Extra parameters
	Caps2.dwCaps2 = 0;				// Additional surface capabilities
	Caps2.dwCaps3 = 0;				// Not used
	Caps2.dwCaps4 = 0;				// Not used
	Caps2.dwVolumeDepth = 0;		// Not used
}

void ConvertCaps(DDCAPS &Caps, DDCAPS &Caps2)
{
	if ((Caps.dwSize != sizeof(DDCAPS_DX1) && Caps.dwSize != sizeof(DDCAPS_DX3) &&
		Caps.dwSize != sizeof(DDCAPS_DX5) && Caps.dwSize != sizeof(DDCAPS_DX6) &&
		Caps.dwSize != sizeof(DDCAPS_DX7)) || (Caps2.dwSize != sizeof(DDCAPS_DX1) &&
		Caps2.dwSize != sizeof(DDCAPS_DX3) && Caps2.dwSize != sizeof(DDCAPS_DX5) &&
		Caps2.dwSize != sizeof(DDCAPS_DX6) && Caps2.dwSize != sizeof(DDCAPS_DX7)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error! Incorrect dwSize: " << Caps.dwSize << " " << Caps2.dwSize);
		return;
	}
	DWORD Size = Caps.dwSize;
	ZeroMemory(&Caps, Caps.dwSize);
	Caps.dwSize = Size;
	CopyMemory(&Caps, &Caps2, min(Caps.dwSize, Caps2.dwSize));
	m_IDirectDrawX::AdjustVidMemory(&Caps.dwVidMemTotal, &Caps.dwVidMemFree);
}

void ConvertCaps(DDCAPS &Caps7, D3DCAPS9 &Caps9)
{
	// Note: dwVidMemTotal and dwVidMemFree are not part of D3DCAPS9 and need to be set separately
	if (Caps7.dwSize != sizeof(DDCAPS))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error! Incorrect dwSize: " << Caps7.dwSize);
		return;
	}
	ZeroMemory(&Caps7, sizeof(DDCAPS));
	Caps7.dwSize = sizeof(DDCAPS);

	// Caps
	Caps7.dwCaps = (Caps9.Caps & (/*D3DCAPS_OVERLAY |*/ D3DCAPS_READ_SCANLINE)) |
		(/*DDCAPS_3D |*/ DDCAPS_BLT | DDCAPS_BLTQUEUE | DDCAPS_BLTFOURCC | DDCAPS_BLTSTRETCH | DDCAPS_GDI | /*DDCAPS_OVERLAYCANTCLIP | DDCAPS_OVERLAYFOURCC |
			DDCAPS_OVERLAYSTRETCH | DDCAPS_ZBLTS |*/ DDCAPS_COLORKEY | DDCAPS_ALPHA | DDCAPS_COLORKEYHWASSIST | DDCAPS_BLTCOLORFILL | DDCAPS_CANBLTSYSMEM);
	Caps7.dwCaps2 = (Caps9.Caps2 & (D3DCAPS2_FULLSCREENGAMMA | D3DCAPS2_CANCALIBRATEGAMMA | D3DCAPS2_CANMANAGERESOURCE | D3DCAPS2_DYNAMICTEXTURES | /*D3DCAPS2_CANAUTOGENMIPMAP |*/ D3DCAPS2_CANSHARERESOURCE)) |
		(DDCAPS2_CANBOBINTERLEAVED | DDCAPS2_CANBOBNONINTERLEAVED | /*DDCAPS2_NONLOCALVIDMEM |*/ DDCAPS2_WIDESURFACES | /*DDCAPS2_CANFLIPODDEVEN |*/ DDCAPS2_COPYFOURCC |
			DDCAPS2_PRIMARYGAMMA | DDCAPS2_CANRENDERWINDOWED | /*DDCAPS2_FLIPINTERVAL |*/ DDCAPS2_FLIPNOVSYNC);
	Caps7.dwCKeyCaps = (/*DDCKEYCAPS_DESTOVERLAY |*/ DDCKEYCAPS_SRCBLT /*| DDCKEYCAPS_SRCOVERLAY*/);
	Caps7.dwFXCaps = (DDFXCAPS_BLTARITHSTRETCHY | DDFXCAPS_BLTMIRRORLEFTRIGHT | DDFXCAPS_BLTMIRRORUPDOWN | DDFXCAPS_BLTSHRINKX | DDFXCAPS_BLTSHRINKY | DDFXCAPS_BLTSTRETCHX |
		DDFXCAPS_BLTSTRETCHY /*| DDFXCAPS_OVERLAYARITHSTRETCHY | DDFXCAPS_OVERLAYSHRINKX | DDFXCAPS_OVERLAYSHRINKY | DDFXCAPS_OVERLAYSTRETCHX | DDFXCAPS_OVERLAYSTRETCHY |
		DDFXCAPS_OVERLAYMIRRORLEFTRIGHT | DDFXCAPS_OVERLAYMIRRORUPDOWN | DDFXCAPS_OVERLAYDEINTERLACE*/);


	// ddsCaps
	Caps7.ddsCaps.dwCaps = (/*DDSCAPS_3DDEVICE |*/ DDSCAPS_BACKBUFFER | DDSCAPS_COMPLEX | DDSCAPS_FLIP | DDSCAPS_FRONTBUFFER | DDSCAPS_LOCALVIDMEM | /*DDSCAPS_MIPMAP | DDSCAPS_NONLOCALVIDMEM |*/
		DDSCAPS_OFFSCREENPLAIN | /*DDSCAPS_OVERLAY |*/ DDSCAPS_OWNDC | DDSCAPS_PRIMARYSURFACE | /*DDSCAPS_TEXTURE |*/ DDSCAPS_VIDEOMEMORY /*| DDSCAPS_ZBUFFER*/);
	Caps7.ddsCaps.dwCaps2 = 0 /*DDSCAPS2_CUBEMAP*/;	// Additional surface capabilities
	Caps7.ddsCaps.dwCaps3 = 0;					// Not used
	Caps7.ddsCaps.dwCaps4 = 0;					// Not used
	Caps7.ddsCaps.dwVolumeDepth = 0;			// Not used
	Caps7.ddsOldCaps.dwCaps = Caps7.ddsCaps.dwCaps;

	// Other settings
	//Caps7.dwMaxVisibleOverlays = 0x1;
	Caps7.dwNumFourCCCodes = NumFourCCs;
	//Caps7.dwMinOverlayStretch = 0x1;
	//Caps7.dwMaxOverlayStretch = 0x4e20;
	Caps7.dwRops[6] = 4096;
	Caps7.dwSVBCaps = DDCAPS_BLT;
	Caps7.dwSVBRops[6] = 4096;
	Caps7.dwVSBCaps = DDCAPS_BLT;
	Caps7.dwVSBRops[6] = 4096;
	Caps7.dwNLVBCaps = Caps7.dwCaps /*| DDCAPS_OVERLAY*/;
	Caps7.dwNLVBCaps2 = (Caps7.dwCaps2 /*| DDCAPS2_COLORCONTROLOVERLAY*/) & ~(DDCAPS2_CANRENDERWINDOWED);
	Caps7.dwNLVBCKeyCaps = Caps7.dwCKeyCaps;
	Caps7.dwNLVBFXCaps = Caps7.dwFXCaps;
	Caps7.dwNLVBRops[6] = 4096;
}

DWORD GetBitCount(DDPIXELFORMAT ddpfPixelFormat)
{
	if (ddpfPixelFormat.dwSize != sizeof(DDPIXELFORMAT))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error! Incorrect dwSize: " << ddpfPixelFormat.dwSize);
		return 0;
	}
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
	if (ddpfPixelFormat.dwFlags & DDPF_FOURCC)
	{
		switch (ddpfPixelFormat.dwFourCC)
		{
		case D3DFMT_UYVY:
		case D3DFMT_YUY2:
			return 2;
		}
	}

	LOG_LIMIT(100, __FUNCTION__ << " Error: failed to get BitCount from PixelFormat! " << ddpfPixelFormat);
	return 0;
}

DWORD GetBitCount(D3DFORMAT Format)
{
	switch (Format)
	{
	case D3DFMT_UNKNOWN:
		break;
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
	if (ddpfPixelFormat.dwSize != sizeof(DDPIXELFORMAT))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error! Incorrect dwSize: " << ddpfPixelFormat.dwSize);
		return D3DFMT_UNKNOWN;
	}
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
		case D3DFMT_UYVY:
			LOG_LIMIT(100, __FUNCTION__ << " Using format: D3DFMT_UYVY");
			return D3DFMT_UYVY;
		case D3DFMT_YUY2:
			LOG_LIMIT(100, __FUNCTION__ << " Using format: D3DFMT_YUY2");
			return D3DFMT_YUY2;
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
	if (ddpfPixelFormat.dwSize != sizeof(DDPIXELFORMAT))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error! Incorrect dwSize: " << ddpfPixelFormat.dwSize);
		return;
	}

	ddpfPixelFormat.dwFlags = DDPF_RGB;
	ddpfPixelFormat.dwRGBBitCount = GetBitCount(Format);
	ddpfPixelFormat.dwRGBAlphaBitMask = 0x0;
	ddpfPixelFormat.dwRBitMask = 0x0;
	ddpfPixelFormat.dwGBitMask = 0x0;
	ddpfPixelFormat.dwBBitMask = 0x0;

	// Set BitCount and BitMask
	switch (Format)
	{
	case D3DFMT_R5G6B5:
		ddpfPixelFormat.dwRBitMask = 0xF800;
		ddpfPixelFormat.dwGBitMask = 0x7E0;
		ddpfPixelFormat.dwBBitMask = 0x1F;
		break;
	case D3DFMT_A1R5G5B5:
		ddpfPixelFormat.dwFlags |= DDPF_ALPHAPIXELS;
		ddpfPixelFormat.dwRGBAlphaBitMask = 0x8000;
	case D3DFMT_X1R5G5B5:
		ddpfPixelFormat.dwRBitMask = 0x7C00;
		ddpfPixelFormat.dwGBitMask = 0x3E0;
		ddpfPixelFormat.dwBBitMask = 0x1F;
		break;
	case D3DFMT_A8R8G8B8:
		ddpfPixelFormat.dwFlags |= DDPF_ALPHAPIXELS;
		ddpfPixelFormat.dwRGBAlphaBitMask = 0xFF000000;
	case D3DFMT_R8G8B8:
	case D3DFMT_X8R8G8B8:
		ddpfPixelFormat.dwRBitMask = 0xFF0000;
		ddpfPixelFormat.dwGBitMask = 0xFF00;
		ddpfPixelFormat.dwBBitMask = 0xFF;
		break;
	case D3DFMT_A2R10G10B10:
		ddpfPixelFormat.dwFlags |= DDPF_ALPHAPIXELS;
		ddpfPixelFormat.dwRGBAlphaBitMask = 0xC0000000;
		ddpfPixelFormat.dwRBitMask = 0x3FF00000;
		ddpfPixelFormat.dwGBitMask = 0xFFC00;
		ddpfPixelFormat.dwBBitMask = 0x3FF;
		break;
	case D3DFMT_P8:
		ddpfPixelFormat.dwFlags |= DDPF_PALETTEINDEXED8;
		break;
	case D3DFMT_A8:
		ddpfPixelFormat.dwFlags = DDPF_ALPHA;
		break;
	case D3DFMT_L8:
		ddpfPixelFormat.dwFlags = DDPF_LUMINANCE;
		break;
	case D3DFMT_A4L4:
		ddpfPixelFormat.dwFlags = DDPF_ALPHAPIXELS | DDPF_LUMINANCE;
		ddpfPixelFormat.dwLuminanceAlphaBitMask = 0xF0;
		break;
	case D3DFMT_A8L8:
		ddpfPixelFormat.dwFlags = DDPF_ALPHAPIXELS | DDPF_LUMINANCE;
		ddpfPixelFormat.dwLuminanceAlphaBitMask = 0xFF00;
		break;
	case D3DFMT_DXT1:
		ddpfPixelFormat.dwFlags = DDPF_FOURCC;
		ddpfPixelFormat.dwFourCC = D3DFMT_DXT1;
		break;
	case D3DFMT_DXT2:
		ddpfPixelFormat.dwFlags = DDPF_FOURCC;
		ddpfPixelFormat.dwFourCC = D3DFMT_DXT2;
		break;
	case D3DFMT_DXT3:
		ddpfPixelFormat.dwFlags = DDPF_FOURCC;
		ddpfPixelFormat.dwFourCC = D3DFMT_DXT3;
		break;
	case D3DFMT_DXT4:
		ddpfPixelFormat.dwFlags = DDPF_FOURCC;
		ddpfPixelFormat.dwFourCC = D3DFMT_DXT4;
		break;
	case D3DFMT_UYVY:
		ddpfPixelFormat.dwFlags = DDPF_FOURCC;
		ddpfPixelFormat.dwFourCC = D3DFMT_UYVY;
		break;
	case D3DFMT_YUY2:
		ddpfPixelFormat.dwFlags = DDPF_FOURCC;
		ddpfPixelFormat.dwFourCC = D3DFMT_YUY2;
		break;
	default:
		LOG_LIMIT(100, __FUNCTION__ << " Display format not Implemented: " << Format);
		return;
	}
}

HRESULT SetDisplayFormat(DWORD BPP, DDPIXELFORMAT &ddpfPixelFormat)
{
	switch (BPP)
	{
	case 8:
		SetPixelDisplayFormat(D3DFMT_P8, ddpfPixelFormat);
		break;
	case 16:
		SetPixelDisplayFormat(D3DFMT_R5G6B5, ddpfPixelFormat);
		break;
	case 24:
		SetPixelDisplayFormat(D3DFMT_R8G8B8, ddpfPixelFormat);
		break;
	case 32:
		SetPixelDisplayFormat(D3DFMT_X8R8G8B8, ddpfPixelFormat);
		break;
	default:
		LOG_LIMIT(100, __FUNCTION__ << " Error! Bit mode not supported: " << BPP);
		return DDERR_UNSUPPORTED;
	}
	return DD_OK;
}

DWORD ComputePitch(DWORD Width, DWORD BitCount)
{
	return ((((Width * BitCount) + 31) & ~31) >> 3);	// Use Surface Stride for pitch
}
