/**
* Copyright (C) 2018 Elisha Riedlinger
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
	ColorControl.dwSize = ColorControl2.dwSize;
	ColorControl.dwFlags = ColorControl2.dwFlags;
	ColorControl.lBrightness = ColorControl2.lBrightness;
	ColorControl.lContrast = ColorControl2.lContrast;
	ColorControl.lHue = ColorControl2.lHue;
	ColorControl.lSaturation = ColorControl2.lSaturation;
	ColorControl.lSharpness = ColorControl2.lSharpness;
	ColorControl.lGamma = ColorControl2.lGamma;
	ColorControl.lColorEnable = ColorControl2.lColorEnable;
	ColorControl.dwReserved1 = ColorControl2.dwReserved1;
}

void ConvertSurfaceDesc(DDSURFACEDESC &Desc, DDSURFACEDESC &Desc2)
{
	CopyMemory(&Desc, &Desc2, sizeof(DDSURFACEDESC));
}

void ConvertSurfaceDesc(DDSURFACEDESC2 &Desc, DDSURFACEDESC2 &Desc2)
{
	CopyMemory(&Desc, &Desc2, sizeof(DDSURFACEDESC2));
}

void ConvertSurfaceDesc(DDSURFACEDESC &Desc, DDSURFACEDESC2 &Desc2)
{
	Desc.dwSize = sizeof(DDSURFACEDESC);
	Desc.dwFlags = Desc2.dwFlags & ~(DDSD_TEXTURESTAGE | DDSD_ZBUFFERBITDEPTH);		// Remove unsupported flags
	Desc.dwHeight = Desc2.dwHeight;
	Desc.dwWidth = Desc2.dwWidth;
	Desc.lPitch = Desc2.lPitch;
	Desc.dwLinearSize = Desc2.dwLinearSize;
	Desc.dwBackBufferCount = Desc2.dwBackBufferCount;
	Desc.dwMipMapCount = Desc2.dwMipMapCount;
	Desc.dwRefreshRate = Desc2.dwRefreshRate;
	Desc.dwAlphaBitDepth = Desc2.dwAlphaBitDepth;
	Desc.dwReserved = Desc2.dwReserved;
	Desc.lpSurface = Desc2.lpSurface;
	Desc.ddckCKDestOverlay.dwColorSpaceHighValue = Desc2.ddckCKDestOverlay.dwColorSpaceHighValue;
	Desc.ddckCKDestOverlay.dwColorSpaceLowValue = Desc2.ddckCKDestOverlay.dwColorSpaceLowValue;
	Desc.ddckCKDestBlt.dwColorSpaceHighValue = Desc2.ddckCKDestBlt.dwColorSpaceHighValue;
	Desc.ddckCKDestBlt.dwColorSpaceLowValue = Desc2.ddckCKDestBlt.dwColorSpaceLowValue;
	Desc.ddckCKSrcOverlay.dwColorSpaceHighValue = Desc2.ddckCKSrcOverlay.dwColorSpaceHighValue;
	Desc.ddckCKSrcOverlay.dwColorSpaceLowValue = Desc2.ddckCKSrcOverlay.dwColorSpaceLowValue;
	Desc.ddckCKSrcBlt.dwColorSpaceHighValue = Desc2.ddckCKSrcBlt.dwColorSpaceHighValue;
	Desc.ddckCKSrcBlt.dwColorSpaceLowValue = Desc2.ddckCKSrcBlt.dwColorSpaceLowValue;
	ConvertPixelFormat(Desc.ddpfPixelFormat, Desc2.ddpfPixelFormat);
	ConvertCaps(Desc.ddsCaps, Desc2.ddsCaps);
	// Extra parameters
	Desc.dwZBufferBitDepth = 0;				// depth of Z buffer requested
}

void ConvertSurfaceDesc(DDSURFACEDESC2 &Desc2, DDSURFACEDESC &Desc)
{
	Desc2.dwSize = sizeof(DDSURFACEDESC2);
	Desc2.dwFlags = Desc.dwFlags & ~(DDSD_TEXTURESTAGE | DDSD_ZBUFFERBITDEPTH);		// Remove unsupported flags
	Desc2.dwHeight = Desc.dwHeight;
	Desc2.dwWidth = Desc.dwWidth;
	Desc2.lPitch = Desc.lPitch;
	Desc2.dwLinearSize = Desc.dwLinearSize;
	Desc2.dwBackBufferCount = Desc.dwBackBufferCount;
	Desc2.dwMipMapCount = Desc.dwMipMapCount;
	Desc2.dwRefreshRate = Desc.dwRefreshRate;
	Desc2.dwAlphaBitDepth = Desc.dwAlphaBitDepth;
	Desc2.dwReserved = Desc.dwReserved;
	Desc2.lpSurface = Desc.lpSurface;
	Desc2.ddckCKDestOverlay.dwColorSpaceHighValue = Desc.ddckCKDestOverlay.dwColorSpaceHighValue;
	Desc2.ddckCKDestOverlay.dwColorSpaceLowValue = Desc.ddckCKDestOverlay.dwColorSpaceLowValue;
	Desc2.ddckCKDestBlt.dwColorSpaceHighValue = Desc.ddckCKDestBlt.dwColorSpaceHighValue;
	Desc2.ddckCKDestBlt.dwColorSpaceLowValue = Desc.ddckCKDestBlt.dwColorSpaceLowValue;
	Desc2.ddckCKSrcOverlay.dwColorSpaceHighValue = Desc.ddckCKSrcOverlay.dwColorSpaceHighValue;
	Desc2.ddckCKSrcOverlay.dwColorSpaceLowValue = Desc.ddckCKSrcOverlay.dwColorSpaceLowValue;
	Desc2.ddckCKSrcBlt.dwColorSpaceHighValue = Desc.ddckCKSrcBlt.dwColorSpaceHighValue;
	Desc2.ddckCKSrcBlt.dwColorSpaceLowValue = Desc.ddckCKSrcBlt.dwColorSpaceLowValue;
	ConvertPixelFormat(Desc2.ddpfPixelFormat, Desc.ddpfPixelFormat);
	ConvertCaps(Desc2.ddsCaps, Desc.ddsCaps);
	// Extra parameters
	Desc2.dwDepth = 0;						// The depth, in pixels, of the surface if it is a volume texture.
	Desc2.dwSrcVBHandle = 0;				// Not used
	Desc2.dwEmptyFaceColor = 0;				// Physical color for empty cubemap faces
	Desc2.dwFVF = 0;						// Vertex format description of vertex buffers
	Desc2.dwTextureStage = 0;				// Stage identifier that is used to bind a texture to a specific stage
}

void ConvertPixelFormat(DDPIXELFORMAT &Format, DDPIXELFORMAT &Format2)
{
	CopyMemory(&Format, &Format2, sizeof(DDPIXELFORMAT));
	Format.dwSize = sizeof(DDPIXELFORMAT);
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
	DeviceID2.dwWHQLLevel;
}

void ConvertDeviceIdentifier(DDDEVICEIDENTIFIER2 &DeviceID2, D3DADAPTER_IDENTIFIER9 &Identifier9)
{
	memcpy(DeviceID2.szDriver, Identifier9.Driver, MAX_DDDEVICEID_STRING);
	memcpy(DeviceID2.szDescription, Identifier9.Description, MAX_DDDEVICEID_STRING);
	DeviceID2.guidDeviceIdentifier = Identifier9.DeviceIdentifier;
	memcpy(&DeviceID2.liDriverVersion, &Identifier9.DriverVersion, sizeof(LARGE_INTEGER));
	DeviceID2.dwDeviceId = Identifier9.DeviceId;
	DeviceID2.dwRevision = Identifier9.Revision;
	DeviceID2.dwSubSysId = Identifier9.SubSysId;
	DeviceID2.dwVendorId = Identifier9.VendorId;
	DeviceID2.dwWHQLLevel = Identifier9.WHQLLevel;
}

void ConvertCaps(DDSCAPS2 &Caps, DDSCAPS2 &Caps2)
{
	CopyMemory(&Caps, &Caps2, sizeof(DDSCAPS2));
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
	Caps2.dwVolumeDepth = 0;		// Not used
}

bool ConvertCaps(DDCAPS_DX7 &Caps, DDCAPS_DX7 &Caps7)
{
	// Set available memory, some games have issues if this is set to high
	if (Config.ConvertToDirectDraw7 && Caps7.dwVidMemTotal > 0x8000000)
	{
		Caps7.dwVidMemFree = 0x8000000 - (Caps7.dwVidMemTotal - Caps7.dwVidMemFree);
		Caps7.dwVidMemTotal = 0x8000000;
	}

	// Convert caps
	switch (Caps.dwSize)
	{
	case sizeof(DDCAPS_DX1) :
		ConvertCaps(*(DDCAPS_DX1*)&Caps, Caps7);
		return true;
	case sizeof(DDCAPS_DX3) :
		ConvertCaps(*(DDCAPS_DX3*)&Caps, Caps7);
		return true;
	case sizeof(DDCAPS_DX5) :
		ConvertCaps(*(DDCAPS_DX5*)&Caps, Caps7);
		return true;
	case sizeof(DDCAPS_DX7) :
		CopyMemory(&Caps, &Caps7, sizeof(DDCAPS_DX7));
		Caps.dwSize = sizeof(DDCAPS_DX7);
		return true;
	default:
		return false;
	}
}

void ConvertCaps(DDCAPS_DX1 &Caps1, DDCAPS_DX7 &Caps7)
{
	CopyMemory(&Caps1, &Caps7, sizeof(DDCAPS_DX1));
	Caps1.dwSize = sizeof(DDCAPS_DX1);
}

void ConvertCaps(DDCAPS_DX3 &Caps3, DDCAPS_DX7 &Caps7)
{
	CopyMemory(&Caps3, &Caps7, sizeof(DDCAPS_DX3));
	Caps3.dwSize = sizeof(DDCAPS_DX3);
}

void ConvertCaps(DDCAPS_DX5 &Caps5, DDCAPS_DX7 &Caps7)
{
	CopyMemory(&Caps5, &Caps7, sizeof(DDCAPS_DX5));
	Caps5.dwSize = sizeof(DDCAPS_DX5);
}

void ConvertCaps(DDCAPS &Caps7, D3DCAPS9 &Caps9)
{
	ZeroMemory(&Caps7, sizeof(DDCAPS));
	Caps7.dwSize = sizeof(DDCAPS);

	// General settings
	Caps7.dwVidMemTotal = 0x8000000;		// Just hard code the memory size
	Caps7.dwVidMemFree = 0x7e00000;			// Just hard code the memory size
	Caps7.dwMaxVisibleOverlays = 0x1;
	Caps7.dwNumFourCCCodes = 0x12;
	Caps7.dwRops[6] = 4096;
	Caps7.dwMinOverlayStretch = 0x1;
	Caps7.dwMaxOverlayStretch = 0x4e20;

	// Caps
	Caps7.dwCaps = (Caps9.Caps & (D3DCAPS_OVERLAY | D3DCAPS_READ_SCANLINE)) |
		(DDCAPS_3D | DDCAPS_BLT | DDCAPS_BLTQUEUE | DDCAPS_BLTFOURCC | DDCAPS_BLTSTRETCH | DDCAPS_GDI | DDCAPS_OVERLAYCANTCLIP | DDCAPS_OVERLAYFOURCC |
			DDCAPS_OVERLAYSTRETCH | DDCAPS_ZBLTS | DDCAPS_COLORKEY | DDCAPS_ALPHA | DDCAPS_COLORKEYHWASSIST | DDCAPS_BLTCOLORFILL | DDCAPS_BLTDEPTHFILL);
	Caps7.dwCaps2 = (Caps9.Caps2 & (D3DCAPS2_FULLSCREENGAMMA | D3DCAPS2_CANCALIBRATEGAMMA | D3DCAPS2_CANMANAGERESOURCE | D3DCAPS2_DYNAMICTEXTURES | D3DCAPS2_CANAUTOGENMIPMAP)) |
		(DDCAPS2_CANBOBINTERLEAVED | DDCAPS2_CANBOBNONINTERLEAVED | DDCAPS2_NONLOCALVIDMEM | DDCAPS2_WIDESURFACES | DDCAPS2_CANFLIPODDEVEN | DDCAPS2_COPYFOURCC |
			DDCAPS2_CANRENDERWINDOWED | DDCAPS2_FLIPINTERVAL | DDCAPS2_FLIPNOVSYNC);
	Caps7.dwCKeyCaps = (DDCKEYCAPS_DESTOVERLAY | DDCKEYCAPS_SRCBLT | DDCKEYCAPS_SRCOVERLAY);
	Caps7.dwFXCaps = (DDFXCAPS_BLTARITHSTRETCHY | DDFXCAPS_BLTMIRRORLEFTRIGHT | DDFXCAPS_BLTMIRRORUPDOWN | DDFXCAPS_BLTSHRINKX | DDFXCAPS_BLTSHRINKY | DDFXCAPS_BLTSTRETCHX |
					   DDFXCAPS_BLTSTRETCHY | DDFXCAPS_OVERLAYARITHSTRETCHY | DDFXCAPS_OVERLAYSHRINKX | DDFXCAPS_OVERLAYSHRINKY | DDFXCAPS_OVERLAYSTRETCHX | DDFXCAPS_OVERLAYSTRETCHY |
					   DDFXCAPS_OVERLAYMIRRORLEFTRIGHT | DDFXCAPS_OVERLAYMIRRORUPDOWN | DDFXCAPS_OVERLAYDEINTERLACE);
	Caps7.ddsCaps.dwCaps = (DDSCAPS_BACKBUFFER | DDSCAPS_COMPLEX | DDSCAPS_FLIP | DDSCAPS_FRONTBUFFER | DDSCAPS_OFFSCREENPLAIN | DDSCAPS_OVERLAY | DDSCAPS_PALETTE |
		DDSCAPS_PRIMARYSURFACE | DDSCAPS_TEXTURE | DDSCAPS_3DDEVICE | DDSCAPS_VIDEOMEMORY | DDSCAPS_ZBUFFER | DDSCAPS_OWNDC | DDSCAPS_MIPMAP | DDSCAPS_LOCALVIDMEM | 
		DDSCAPS_NONLOCALVIDMEM);
	Caps7.ddsOldCaps.dwCaps = Caps7.ddsCaps.dwCaps;
}

DWORD GetBitCount(DDPIXELFORMAT ddpfPixelFormat)
{
	if ((ddpfPixelFormat.dwFlags & DDPF_RGB) != 0)
	{
		return ddpfPixelFormat.dwRGBBitCount;
	}
	else if ((ddpfPixelFormat.dwFlags & DDPF_YUV) != 0)
	{
		return ddpfPixelFormat.dwYUVBitCount;
	}
	else if ((ddpfPixelFormat.dwFlags & DDPF_ALPHA) != 0)
	{
		return ddpfPixelFormat.dwAlphaBitDepth;
	}
	else if ((ddpfPixelFormat.dwFlags & DDPF_ZBUFFER) != 0)
	{
		return ddpfPixelFormat.dwZBufferBitDepth;
	}
	else if ((ddpfPixelFormat.dwFlags & DDPF_LUMINANCE) != 0)
	{
		return ddpfPixelFormat.dwLuminanceBitCount;
	}
	else if ((ddpfPixelFormat.dwFlags & DDPF_BUMPDUDV) != 0)
	{
		return ddpfPixelFormat.dwBumpBitCount;
	}

	Logging::Log() << __FUNCTION__ << " Failed to get BitCount from PixelFormat!";
	return 0;
}

DWORD GetBitCount(D3DFORMAT Format)
{
	switch (Format)
	{
	case D3DFMT_R5G6B5:
	case D3DFMT_X1R5G5B5:
	case D3DFMT_A1R5G5B5:
		return 16;
	case D3DFMT_X8R8G8B8:
	case D3DFMT_A8R8G8B8:
	case D3DFMT_A2R10G10B10:
		return 32;
	}

	Logging::Log() << __FUNCTION__ << " Display format not Implemented: " << Format;
	return 0;
}

D3DFORMAT GetDisplayFormat(DDPIXELFORMAT ddpfPixelFormat)
{
	if (ddpfPixelFormat.dwFlags & DDPF_RGB)
	{
		DWORD BitCount = GetBitCount(ddpfPixelFormat);
		switch (BitCount)
		{
		case 8:
			// Default 8-bit
			return D3DFMT_X8R8G8B8;
		case 16:
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
			Logging::Log() << __FUNCTION__ << " 24-bit RGB not Implemented";
			return D3DFMT_UNKNOWN;
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

		Logging::Log() << __FUNCTION__ << " Error, could not find RGB format for BitCount: " << BitCount;
		return D3DFMT_UNKNOWN;
	}
	else if(ddpfPixelFormat.dwFlags & DDPF_YUV)
	{
		Logging::Log() << __FUNCTION__ << " YUV format not Implemented";
		return D3DFMT_UNKNOWN;
	}
	else if (ddpfPixelFormat.dwFlags & DDPF_ALPHA)
	{
		Logging::Log() << __FUNCTION__ << " Alpha format not Implemented";
		return D3DFMT_UNKNOWN;
	}
	else if (ddpfPixelFormat.dwFlags & DDPF_ZBUFFER)
	{
		Logging::Log() << __FUNCTION__ << " zBuffer format not Implemented";
		return D3DFMT_UNKNOWN;
	}
	else if (ddpfPixelFormat.dwFlags & DDPF_LUMINANCE)
	{
		Logging::Log() << __FUNCTION__ << " Luminance format not Implemented";
		return D3DFMT_UNKNOWN;
	}
	else if (ddpfPixelFormat.dwFlags & DDPF_BUMPDUDV)
	{
		Logging::Log() << __FUNCTION__ << " Bump DUVU format not Implemented";
		return D3DFMT_UNKNOWN;
	}

	Logging::Log() << __FUNCTION__ << " Error, PixelFormat not Implemented see flags: " << ddpfPixelFormat.dwFlags;
	return D3DFMT_UNKNOWN;
}

void GetPixelDisplayFormat(D3DFORMAT Format, DDPIXELFORMAT &ddpfPixelFormat)
{
	// Supported RGB formats
	if (Format == D3DFMT_R5G6B5 || Format == D3DFMT_X1R5G5B5 || Format == D3DFMT_A1R5G5B5 || Format == D3DFMT_X8R8G8B8 || Format == D3DFMT_A8R8G8B8 || Format == D3DFMT_A2R10G10B10)
	{
		ddpfPixelFormat.dwFlags = DDPF_RGB;
		ddpfPixelFormat.dwRGBBitCount = GetBitCount(Format);

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
		}
		return;
	}

	Logging::Log() << __FUNCTION__ << " Display format not Implemented: " << Format;
	return;
}
