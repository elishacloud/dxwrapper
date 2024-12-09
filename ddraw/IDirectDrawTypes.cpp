/**
* Copyright (C) 2024 Elisha Riedlinger
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

// For storing resolution list
std::vector<std::pair<DWORD, DWORD>> CashedDisplayResolutions;

void AddDisplayResolution(DWORD Width, DWORD Height)
{
	if (!IsDisplayResolution(Width, Height))
	{
		CashedDisplayResolutions.push_back({ Width, Height });
	}
}

bool IsDisplayResolution(DWORD Width, DWORD Height)
{
	for (const auto& entry : CashedDisplayResolutions)
	{
		if (entry.first == Width && entry.second == Height)
		{
			return true;
		}
	}
	return false;
}

bool DoRectsMatch(const RECT& lhs, const RECT& rhs)
{
	return lhs.left == rhs.left && lhs.top == rhs.top &&
		lhs.right == rhs.right && lhs.bottom == rhs.bottom;
}

bool GetOverlappingRect(const RECT& rect1, const RECT& rect2, RECT& outOverlapRect)
{
	// Compute the boundaries of the overlap
	outOverlapRect.left = max(rect1.left, rect2.left);
	outOverlapRect.top = max(rect1.top, rect2.top);
	outOverlapRect.right = min(rect1.right, rect2.right);
	outOverlapRect.bottom = min(rect1.bottom, rect2.bottom);

	// Check if they overlap
	if (outOverlapRect.left < outOverlapRect.right && outOverlapRect.top < outOverlapRect.bottom)
	{
		return true; // Rectangles overlap
	}

	// If no overlap, return false
	return false;
}

void ConvertSurfaceDesc(DDSURFACEDESC& Desc, const DDSURFACEDESC2& Desc2)
{
	// Check for supported dwSize
	if (Desc.dwSize != sizeof(DDSURFACEDESC) || (Desc2.dwSize != sizeof(DDSURFACEDESC2) && Desc2.dwSize != sizeof(DDSURFACEDESC)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << Desc.dwSize << " " << Desc2.dwSize);
		return;
	}
	// Prepare destination structure
	ZeroMemory(&Desc, sizeof(DDSURFACEDESC));
	Desc.dwSize = sizeof(DDSURFACEDESC);
	// Convert variables
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
	if (Desc2.dwFlags & DDSD_PIXELFORMAT)
	{
		CopyMemory(&Desc.ddpfPixelFormat, &Desc2.ddpfPixelFormat, sizeof(DDPIXELFORMAT));
		Desc.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
		if (Desc2.ddpfPixelFormat.dwFlags & DDPF_ZBUFFER)
		{
			Desc.dwFlags = (Desc.dwFlags | DDSD_ZBUFFERBITDEPTH) & ~DDSD_PIXELFORMAT;
			Desc.dwZBufferBitDepth = Desc2.ddpfPixelFormat.dwZBufferBitDepth;
			Desc.ddpfPixelFormat.dwSize = 0;
			Desc.ddpfPixelFormat.dwFlags = 0;
			Desc.ddpfPixelFormat.dwZBufferBitDepth = 0;
			Desc.ddpfPixelFormat.dwZBitMask = 0;
		}
	}
	ConvertCaps(Desc.ddsCaps, Desc2.ddsCaps);
	// Check for dwFlags that did not get converted
	if (Desc.dwFlags != Desc2.dwFlags && (Desc.dwFlags - Desc2.dwFlags) != ((DWORD)DDSD_ZBUFFERBITDEPTH - (DWORD)DDSD_PIXELFORMAT))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Warning: (Desc2->Desc) Removing unsupported flags: " << Logging::hex(Desc2.dwFlags & ~Desc.dwFlags));
	}
}

void ConvertSurfaceDesc(DDSURFACEDESC2& Desc2, const DDSURFACEDESC& Desc)
{
	// Check for supported dwSize
	if (Desc2.dwSize != sizeof(DDSURFACEDESC2) || Desc.dwSize != sizeof(DDSURFACEDESC))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << Desc2.dwSize << " " << Desc.dwSize);
		return;
	}
	// Prepare destination structure
	ZeroMemory(&Desc2, sizeof(DDSURFACEDESC2));
	Desc2.dwSize = sizeof(DDSURFACEDESC2);
	// Convert variables
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
	if (Desc.dwFlags & DDSD_PIXELFORMAT)
	{
		CopyMemory(&Desc2.ddpfPixelFormat, &Desc.ddpfPixelFormat, sizeof(DDPIXELFORMAT));
		Desc2.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
	}
	if (Desc.dwFlags & DDSD_ZBUFFERBITDEPTH)
	{
		if ((Desc.dwFlags & DDSD_PIXELFORMAT) && Desc.ddpfPixelFormat.dwFlags)
		{
			D3DFORMAT Format = GetDisplayFormat(Desc2.ddpfPixelFormat);
			if (Format != D3DFMT_UNKNOWN)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Warning: overwritting existing pixel format: " << Format << " with zbuffer depth: " << Desc.dwZBufferBitDepth);
			}
		}
		Desc2.dwFlags |= DDSD_PIXELFORMAT;
		Desc2.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
		Desc2.ddpfPixelFormat.dwFlags = DDPF_ZBUFFER;
		Desc2.ddpfPixelFormat.dwZBufferBitDepth = Desc.dwZBufferBitDepth;
		Desc2.ddpfPixelFormat.dwZBitMask =
			(Desc2.ddpfPixelFormat.dwZBufferBitDepth == 32) ? 0xFFFFFFFF :
			(Desc2.ddpfPixelFormat.dwZBufferBitDepth == 24) ? 0xFFFFFF00 :
			(Desc2.ddpfPixelFormat.dwZBufferBitDepth == 16) ? 0xFFFF : 0;
		Desc2.dwRefreshRate = 0;	// Union with dwZBufferBitDepth
	}
	ConvertCaps(Desc2.ddsCaps, Desc.ddsCaps);
	// Extra parameters
	Desc2.dwTextureStage = 0;			// Stage identifier that is used to bind a texture to a specific stage
	// Check for dwFlags that did not get converted
	if (Desc.dwFlags != Desc2.dwFlags && (Desc.dwFlags - Desc2.dwFlags) != ((DWORD)DDSD_ZBUFFERBITDEPTH - (DWORD)DDSD_PIXELFORMAT))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Warning: (Desc->Desc2) Removing unsupported flags: " << Logging::hex(Desc.dwFlags & ~Desc2.dwFlags));
	}
}

void ClearUnusedValues(DDSURFACEDESC2& Desc2)
{
	// Check for supported dwSize
	if (Desc2.dwSize != sizeof(DDSURFACEDESC2))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << Desc2.dwSize);
		return;
	}

	if (!(Desc2.dwFlags & DDSD_HEIGHT)) Desc2.dwHeight = 0;
	if (!(Desc2.dwFlags & DDSD_WIDTH)) Desc2.dwWidth = 0;
	if (!(Desc2.dwFlags & (DDSD_PITCH | DDSD_LINEARSIZE))) Desc2.lPitch = 0;
	if (!(Desc2.dwFlags & (DDSD_BACKBUFFERCOUNT | DDSD_DEPTH))) Desc2.dwBackBufferCount = 0;
	if (!(Desc2.dwFlags & (DDSD_MIPMAPCOUNT | DDSD_REFRESHRATE | DDSD_SRCVBHANDLE))) Desc2.dwMipMapCount = 0;
	if (!(Desc2.dwFlags & DDSD_ALPHABITDEPTH)) Desc2.dwAlphaBitDepth = 0;

	Desc2.dwReserved = 0;

	if (!(Desc2.dwFlags & DDSD_LPSURFACE)) Desc2.lpSurface = nullptr;

	if (!(Desc2.dwFlags & DDSD_CKDESTOVERLAY))
	{
		Desc2.ddckCKDestOverlay.dwColorSpaceLowValue = 0;
		Desc2.ddckCKDestOverlay.dwColorSpaceHighValue = 0;
	}

	if (!(Desc2.dwFlags & DDSD_CKDESTBLT))
	{
		Desc2.ddckCKDestBlt.dwColorSpaceLowValue = 0;
		Desc2.ddckCKDestBlt.dwColorSpaceHighValue = 0;
	}

	if (!(Desc2.dwFlags & DDSD_CKSRCOVERLAY))
	{
		Desc2.ddckCKSrcOverlay.dwColorSpaceLowValue = 0;
		Desc2.ddckCKSrcOverlay.dwColorSpaceHighValue = 0;
	}

	if (!(Desc2.dwFlags & DDSD_CKSRCBLT))
	{
		Desc2.ddckCKSrcBlt.dwColorSpaceLowValue = 0;
		Desc2.ddckCKSrcBlt.dwColorSpaceHighValue = 0;
	}

	if (!(Desc2.dwFlags & DDSD_PIXELFORMAT))
	{
		if (Desc2.dwFlags & DDSD_FVF)
		{
			// Preserve the dwFVF value and clear ddpfPixelFormat
			DWORD dwFVF = Desc2.dwFVF;
			Desc2.ddpfPixelFormat = {}; // Clears the pixel format
			Desc2.dwFVF = dwFVF;        // Restore dwFVF
		}
		else
		{
			Desc2.ddpfPixelFormat = {};
		}
	}
	else
	{
		Desc2.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
	}

	if (!(Desc2.dwFlags & DDSD_CAPS))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Warning: Surface desc has no caps!");
		Desc2.ddsCaps = {};
	}

	if (!(Desc2.dwFlags & DDSD_TEXTURESTAGE)) Desc2.dwTextureStage = 0;
}

void ConvertPixelFormat(DDPIXELFORMAT& Format, const DDS_PIXELFORMAT& Format2)
{
	if (Format.dwSize != sizeof(DDPIXELFORMAT) || Format2.dwSize != sizeof(DDS_PIXELFORMAT))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << Format.dwSize << " " << Format2.dwSize);
		return;
	}
	Format.dwFlags = Format2.dwFlags;
	Format.dwFourCC = Format2.dwFourCC;
	Format.dwRGBBitCount = Format2.dwRGBBitCount;
	Format.dwRBitMask = Format2.dwRBitMask;
	Format.dwGBitMask = Format2.dwGBitMask;
	Format.dwBBitMask = Format2.dwBBitMask;
	Format.dwRGBAlphaBitMask = Format2.dwABitMask;
}

void ConvertDeviceIdentifier(DDDEVICEIDENTIFIER& DeviceID, const DDDEVICEIDENTIFIER2& DeviceID2)
{
	CopyMemory(&DeviceID, &DeviceID2, sizeof(DDDEVICEIDENTIFIER));
}

void ConvertDeviceIdentifier(DDDEVICEIDENTIFIER2& DeviceID2, const DDDEVICEIDENTIFIER& DeviceID)
{
	CopyMemory(&DeviceID2, &DeviceID, sizeof(DDDEVICEIDENTIFIER));
	// Extra parameters
	DeviceID2.dwWHQLLevel = 0;
}

void ConvertDeviceIdentifier(DDDEVICEIDENTIFIER2& DeviceID2, const D3DADAPTER_IDENTIFIER9& Identifier9)
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

void ConvertCaps(DDSCAPS& Caps, const DDSCAPS2& Caps2)
{
	Caps.dwCaps = Caps2.dwCaps;
	// Check for dwFlags that did not get converted
	if (Caps2.dwCaps2 || Caps2.dwVolumeDepth)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Warning: Removing unsupported dwCaps2 and dwVolumeDepth: " << Logging::hex(Caps2.dwCaps2) << " " << Caps2.dwVolumeDepth);
	}
}

void ConvertCaps(DDSCAPS2& Caps2, const DDSCAPS& Caps)
{
	Caps2.dwCaps = Caps.dwCaps;
	// Extra parameters
	Caps2.dwCaps2 = 0;				// Additional surface capabilities
	Caps2.dwCaps3 = 0;				// Not used
	Caps2.dwCaps4 = 0;				// Not used
	Caps2.dwVolumeDepth = 0;		// Not used
}

void ConvertCaps(DDCAPS& Caps, const DDCAPS& Caps2)
{
	if ((Caps.dwSize != sizeof(DDCAPS_DX1) && Caps.dwSize != sizeof(DDCAPS_DX3) &&
		Caps.dwSize != sizeof(DDCAPS_DX5) && Caps.dwSize != sizeof(DDCAPS_DX6) &&
		Caps.dwSize != sizeof(DDCAPS_DX7)) || (Caps2.dwSize != sizeof(DDCAPS_DX1) &&
		Caps2.dwSize != sizeof(DDCAPS_DX3) && Caps2.dwSize != sizeof(DDCAPS_DX5) &&
		Caps2.dwSize != sizeof(DDCAPS_DX6) && Caps2.dwSize != sizeof(DDCAPS_DX7)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << Caps.dwSize << " " << Caps2.dwSize);
		return;
	}
	DWORD Size = Caps.dwSize;
	ZeroMemory(&Caps, Caps.dwSize);
	Caps.dwSize = Size;
	CopyMemory(&Caps, &Caps2, min(Caps.dwSize, Caps2.dwSize));
	AdjustVidMemory(&Caps.dwVidMemTotal, &Caps.dwVidMemFree);
}

void ConvertCaps(DDCAPS& Caps7, D3DCAPS9& Caps9)
{
	// Note: dwVidMemTotal, dwVidMemFree and dwNumFourCCCodes are not part of D3DCAPS9 and need to be set separately
	if (Caps7.dwSize != sizeof(DDCAPS))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << Caps7.dwSize);
		return;
	}

	// Initialize the output structure
	ZeroMemory(&Caps7, sizeof(DDCAPS));
	Caps7.dwSize = sizeof(DDCAPS);

	// Set the primary capabilities flags to indicate support for everything
	Caps7.dwCaps = (Caps9.Caps & DDCAPS_READSCANLINE) |
		(DDCAPS_BLT | DDCAPS_BLTFOURCC | DDCAPS_BLTSTRETCH | DDCAPS_BLTCOLORFILL | DDCAPS_CANBLTSYSMEM | /*DDCAPS_BLTQUEUE |*/
			DDCAPS_OVERLAY | DDCAPS_OVERLAYFOURCC | /*DDCAPS_OVERLAYCANTCLIP | DDCAPS_OVERLAYSTRETCH |*/
			DDCAPS_PALETTE | DDCAPS_PALETTEVSYNC | DDCAPS_COLORKEY | /*DDCAPS_COLORKEYHWASSIST |*/
			DDCAPS_ALPHA | DDCAPS_GDI | DDCAPS_VBI | DDCAPS_CANCLIP | DDCAPS_CANCLIPSTRETCHED) |
		(Config.DdrawDisableDirect3DCaps ? 0 : DDCAPS_3D | DDCAPS_BLTDEPTHFILL /*| DDCAPS_ZBLTS | DDCAPS_ZOVERLAYS*/);

	// Additional capabilities (Caps2)
	Caps7.dwCaps2 = (Caps9.Caps2 & (DDCAPS2_PRIMARYGAMMA /*| DDCAPS2_CANCALIBRATEGAMMA*/)) |
		(DDCAPS2_CERTIFIED | DDCAPS2_NOPAGELOCKREQUIRED |
			DDCAPS2_COLORCONTROLPRIMARY | DDCAPS2_CANDROPZ16BIT |
			DDCAPS2_FLIPNOVSYNC | /*DDCAPS2_FLIPINTERVAL | DDCAPS2_CANFLIPODDEVEN | DDCAPS2_STEREO |*/
			/*DDCAPS2_CANBOBINTERLEAVED | DDCAPS2_CANBOBNONINTERLEAVED | DDCAPS2_CANBOBHARDWARE |*/
			/*DDCAPS2_NONLOCALVIDMEM | DDCAPS2_VIDEOPORT | DDCAPS2_AUTOFLIPOVERLAY | DDCAPS2_COLORCONTROLOVERLAY |*/
			DDCAPS2_WIDESURFACES | DDCAPS2_COPYFOURCC | DDCAPS2_CANRENDERWINDOWED) |
		(Config.DdrawDisableDirect3DCaps ? 0 : DDCAPS2_DYNAMICTEXTURES | DDCAPS2_CANMANAGETEXTURE /*| DDCAPS2_CANMANAGERESOURCE*/);

	// Color key capabilities
	Caps7.dwCKeyCaps = (DDCKEYCAPS_DESTBLT | DDCKEYCAPS_SRCBLT |
		DDCKEYCAPS_DESTOVERLAY | DDCKEYCAPS_SRCOVERLAY /*|
		DDCKEYCAPS_DESTBLTCLRSPACE | DDCKEYCAPS_SRCBLTCLRSPACE |
		DDCKEYCAPS_DESTBLTYUV | DDCKEYCAPS_SRCBLTYUV |
		DDCKEYCAPS_DESTOVERLAYYUV | DDCKEYCAPS_SRCOVERLAYYUV |
		DDCKEYCAPS_DESTBLTCLRSPACEYUV | DDCKEYCAPS_SRCBLTCLRSPACEYUV |
		DDCKEYCAPS_DESTOVERLAYCLRSPACEYUV | DDCKEYCAPS_SRCOVERLAYCLRSPACEYUV |
		DDCKEYCAPS_DESTOVERLAYCLRSPACE | DDCKEYCAPS_SRCOVERLAYCLRSPACE |
		DDCKEYCAPS_NOCOSTOVERLAY*/);

	// Effects capabilities
	Caps7.dwFXCaps = (DDFXCAPS_BLTARITHSTRETCHY | DDFXCAPS_BLTSTRETCHX | DDFXCAPS_BLTSTRETCHY |
		DDFXCAPS_BLTSHRINKX | DDFXCAPS_BLTSHRINKY |
		DDFXCAPS_BLTMIRRORLEFTRIGHT | DDFXCAPS_BLTMIRRORUPDOWN /*|
		DDFXCAPS_BLTROTATION | DDFXCAPS_BLTROTATION90 |
		DDFXCAPS_BLTALPHA | DDFXCAPS_OVERLAYALPHA |
		DDFXCAPS_OVERLAYARITHSTRETCHY | DDFXCAPS_OVERLAYSTRETCHX | DDFXCAPS_OVERLAYSTRETCHY |
		DDFXCAPS_OVERLAYSHRINKX | DDFXCAPS_OVERLAYSHRINKY |
		DDFXCAPS_OVERLAYMIRRORLEFTRIGHT | DDFXCAPS_OVERLAYMIRRORUPDOWN |
		DDFXCAPS_OVERLAYDEINTERLACE*/);

	// Alpha capabilities
	Caps7.dwFXAlphaCaps = 0 /*| DDFXALPHACAPS_BLTALPHAEDGEBLEND | DDFXALPHACAPS_BLTALPHAPIXELS | DDFXALPHACAPS_BLTALPHAPIXELSNEG | DDFXALPHACAPS_BLTALPHASURFACES |
		DDFXALPHACAPS_BLTALPHASURFACESNEG | DDFXALPHACAPS_OVERLAYALPHAEDGEBLEND | DDFXALPHACAPS_OVERLAYALPHAPIXELS | DDFXALPHACAPS_OVERLAYALPHAPIXELSNEG |
		DDFXALPHACAPS_OVERLAYALPHASURFACES | DDFXALPHACAPS_OVERLAYALPHASURFACESNEG*/;

	// Palette capabilities
	Caps7.dwPalCaps = DDPCAPS_8BIT | DDPCAPS_ALLOW256 | DDPCAPS_PRIMARYSURFACE | DDPCAPS_PRIMARYSURFACELEFT | DDPCAPS_VSYNC /*| DDPCAPS_ALPHA*/;

	// Surface capabilities
	Caps7.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FRONTBUFFER | DDSCAPS_BACKBUFFER | DDSCAPS_COMPLEX | DDSCAPS_FLIP |
		DDSCAPS_LOCALVIDMEM | DDSCAPS_VIDEOMEMORY | ((Caps7.dwCaps2 & DDCAPS2_NONLOCALVIDMEM) ? DDSCAPS_NONLOCALVIDMEM : 0) |
		DDSCAPS_OFFSCREENPLAIN | ((Caps7.dwCaps & DDCAPS_OVERLAY) ? DDSCAPS_OVERLAY : 0) | /*DDSCAPS_OWNDC |*/
		(Config.DdrawDisableDirect3DCaps ? 0 : DDSCAPS_3DDEVICE | DDSCAPS_TEXTURE | DDSCAPS_ZBUFFER | DDSCAPS_MIPMAP);
	Caps7.ddsCaps.dwCaps2 = (Config.DdrawDisableDirect3DCaps ? 0 : 0 /*| DDSCAPS2_CUBEMAP | DDSCAPS2_VOLUME*/);
	Caps7.ddsCaps.dwCaps3 = 0;								// Not used
	Caps7.ddsCaps.dwCaps4 = 0;								// Not used
	Caps7.ddsCaps.dwVolumeDepth = 0;						// Not used
	Caps7.ddsOldCaps.dwCaps = Caps7.ddsCaps.dwCaps;

	// Stereo Vision capabilities
	Caps7.dwSVCaps = 0 /*| DDSVCAPS_STEREOSEQUENTIAL*/; // Obsolite: DDSVCAPS_ENIGMA | DDSVCAPS_FLICKER | DDSVCAPS_REDBLUE | DDSVCAPS_SPLIT | DDSVCAPS_ZOOM;

	// Alpha bit depths
	if (Caps7.dwFXAlphaCaps & (DDFXALPHACAPS_BLTALPHAPIXELS | DDFXALPHACAPS_BLTALPHAPIXELSNEG))
	{
		Caps7.dwAlphaBltPixelBitDepths = DDBD_1 | DDBD_2 | DDBD_4 | DDBD_8;
	}
	if (Caps7.dwFXAlphaCaps & (DDFXALPHACAPS_BLTALPHASURFACES | DDFXALPHACAPS_BLTALPHASURFACESNEG))
	{
		Caps7.dwAlphaBltConstBitDepths = DDBD_2 | DDBD_4 | DDBD_8;
		Caps7.dwAlphaBltSurfaceBitDepths = DDBD_1 | DDBD_2 | DDBD_4 | DDBD_8;
	}
	if (Caps7.dwFXAlphaCaps & (DDFXALPHACAPS_OVERLAYALPHAPIXELS | DDFXALPHACAPS_OVERLAYALPHAPIXELSNEG))
	{
		Caps7.dwAlphaOverlayPixelBitDepths = DDBD_1 | DDBD_2 | DDBD_4 | DDBD_8;
	}
	if (Caps7.dwFXAlphaCaps & (DDFXALPHACAPS_OVERLAYALPHASURFACES | DDFXALPHACAPS_OVERLAYALPHASURFACESNEG))
	{
		Caps7.dwAlphaOverlayConstBitDepths = DDBD_2 | DDBD_4 | DDBD_8;
		Caps7.dwAlphaOverlaySurfaceBitDepths = DDBD_1 | DDBD_2 | DDBD_4 | DDBD_8;
	}

	// Z-buffer bit depths
	if (Caps7.dwCaps & DDCAPS_ZBLTS)
	{
		Caps7.dwZBufferBitDepths = DDBD_16 | DDBD_24 | DDBD_32;
	}

	// Overlay settings
	if (Caps7.dwCaps & DDCAPS_OVERLAY)
	{
		Caps7.dwCurrVisibleOverlays = 0;
		Caps7.dwMaxVisibleOverlays = 32;
		Caps7.dwMinOverlayStretch = 1;
		Caps7.dwMaxOverlayStretch = 20000;
	}

	// Bit Blt Caps
	DWORD BitBltCaps = Caps7.dwCaps & (DDCAPS_BLT | DDCAPS_COLORKEY | DDCAPS_OVERLAY | DDCAPS_ALPHA | DDCAPS_BLTFOURCC | DDCAPS_BLTSTRETCH | DDCAPS_ZBLTS | DDCAPS_BLTCOLORFILL | DDCAPS_BLTDEPTHFILL | DDCAPS_CANBLTSYSMEM);
	DWORD BitBltCaps2 = Caps7.dwCaps2 & (DDCAPS2_COPYFOURCC);
	DWORD BitBltCKeyCaps = Caps7.dwCKeyCaps & (DDCKEYCAPS_DESTBLT | DDCKEYCAPS_DESTBLTCLRSPACE | DDCKEYCAPS_DESTBLTCLRSPACEYUV | DDCKEYCAPS_DESTBLTYUV | DDCKEYCAPS_SRCBLT | DDCKEYCAPS_SRCBLTCLRSPACE
		| DDCKEYCAPS_SRCBLTCLRSPACEYUV | DDCKEYCAPS_SRCBLTYUV);
	DWORD BitBltFXCaps = Caps7.dwFXCaps & (DDFXCAPS_BLTARITHSTRETCHY | DDFXCAPS_BLTARITHSTRETCHYN | DDFXCAPS_BLTMIRRORLEFTRIGHT | DDFXCAPS_BLTMIRRORUPDOWN | DDFXCAPS_BLTROTATION | DDFXCAPS_BLTROTATION90 |
		DDFXCAPS_BLTSHRINKX | DDFXCAPS_BLTSHRINKXN | DDFXCAPS_BLTSHRINKY | DDFXCAPS_BLTSHRINKYN | DDFXCAPS_BLTSTRETCHX | DDFXCAPS_BLTSTRETCHXN | DDFXCAPS_BLTSTRETCHY |
		DDFXCAPS_BLTSTRETCHYN | DDFXCAPS_BLTALPHA);

	// Raster Operations
	for (DWORD rop : { SRCCOPY, /*SRCPAINT, SRCAND, SRCINVERT, SRCERASE, NOTSRCCOPY, NOTSRCERASE, MERGECOPY, MERGEPAINT, PATCOPY, PATPAINT, PATINVERT, DSTINVERT,*/ BLACKNESS, WHITENESS })
	{
		const DWORD x = ((rop >> 16) & 0xFF) / 32;
		Caps7.dwRops[x] |= static_cast<DWORD>(1 << ((rop >> 16) & 0xFF) % 32);
	}

	// System -> Video memory blts
	Caps7.dwSVBCaps = BitBltCaps;
	Caps7.dwSVBCaps2 = BitBltCaps2;
	Caps7.dwSVBCKeyCaps = BitBltCKeyCaps;
	Caps7.dwSVBFXCaps = BitBltFXCaps;
	for (int i = 0; i < DD_ROP_SPACE; i++)
	{
		Caps7.dwSVBRops[i] = Caps7.dwRops[i];
	}

	// Video -> System memory blts
	Caps7.dwVSBCaps = BitBltCaps;
	Caps7.dwVSBCKeyCaps = BitBltCKeyCaps;
	Caps7.dwVSBFXCaps = BitBltFXCaps;
	for (int i = 0; i < DD_ROP_SPACE; i++)
	{
		Caps7.dwVSBRops[i] = Caps7.dwRops[i];
	}

	// System memory to system memory blts
	Caps7.dwSSBCaps = BitBltCaps;
	Caps7.dwSSBCKeyCaps = BitBltCKeyCaps;
	Caps7.dwSSBFXCaps = BitBltFXCaps;
	for (int i = 0; i < DD_ROP_SPACE; i++)
	{
		Caps7.dwSSBRops[i] = Caps7.dwRops[i];
	}

	// Non-local -> system memory blts
	if (Caps7.dwCaps2 & DDCAPS2_NONLOCALVIDMEM)
	{
		Caps7.dwNLVBCaps = BitBltCaps;
		Caps7.dwNLVBCaps2 = BitBltCaps2;
		Caps7.dwNLVBCKeyCaps = BitBltCKeyCaps;
		Caps7.dwNLVBFXCaps = BitBltFXCaps;
		for (int i = 0; i < DD_ROP_SPACE; i++)
		{
			Caps7.dwNLVBRops[i] = Caps7.dwRops[i];
		}
	}

	// Live video ports
	Caps7.dwMaxVideoPorts = 0;
	Caps7.dwCurrVideoPorts = 0;

	// Min/Max Live Video Stretch Factors
	Caps7.dwMinLiveVideoStretch = 0;
	Caps7.dwMaxLiveVideoStretch = 0;

	// Min/Max HW Codec Stretch Factors
	Caps7.dwMinHwCodecStretch = 0;
	Caps7.dwMaxHwCodecStretch = 0;

	// FourCC codes
	Caps7.dwNumFourCCCodes = sizeof(FourCCTypes) / sizeof(D3DFORMAT);

	// Video memory
	Caps7.dwVidMemTotal = MaxVidMemory;
	Caps7.dwVidMemFree = MaxVidMemory - MinUsedVidMemory;

	// Reserved fields
	Caps7.dwReserved1 = 0;
	Caps7.dwReserved2 = 0;
	Caps7.dwReserved3 = 0;
}

void AdjustVidMemory(LPDWORD lpdwTotal, LPDWORD lpdwFree)
{
	DWORD TotalVidMem = (lpdwTotal && *lpdwTotal) ? *lpdwTotal : (lpdwFree && *lpdwFree) ? *lpdwFree + MinUsedVidMemory : MaxVidMemory;
	TotalVidMem = min(TotalVidMem, MaxVidMemory);
	DWORD AvailVidMem = (lpdwFree && *lpdwFree) ? *lpdwFree : TotalVidMem - MinUsedVidMemory;
	AvailVidMem = min(AvailVidMem, TotalVidMem - MinUsedVidMemory);
	if (lpdwTotal && *lpdwTotal)
	{
		*lpdwTotal = TotalVidMem;
	}
	if (lpdwFree && *lpdwFree)
	{
		*lpdwFree = AvailVidMem;
	}
}

DWORD GetByteAlignedWidth(DWORD Width, DWORD BitCount)
{
	if (Config.DdrawEnableByteAlignment)
	{
		while ((Width * BitCount) % 64)
		{
			Width++;
		}
	}
	return Width;
}

DWORD GetMaxMipMapLevel(DWORD Width, DWORD Height)
{
	return 1 + static_cast<int>(std::floor(std::log2(min(Width, Height))));
}

DWORD GetBitCount(const DDPIXELFORMAT& ddpfPixelFormat)
{
	if (ddpfPixelFormat.dwSize != sizeof(DDPIXELFORMAT))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << ddpfPixelFormat.dwSize);
		return 0;
	}
	if (ddpfPixelFormat.dwRGBBitCount && (ddpfPixelFormat.dwFlags &
		(DDPF_RGB | DDPF_YUV | DDPF_ALPHA | DDPF_ZBUFFER | DDPF_STENCILBUFFER | DDPF_LUMINANCE | DDPF_BUMPDUDV)))
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
		return GetBitCount((D3DFORMAT)ddpfPixelFormat.dwFourCC);
	}

	LOG_LIMIT(100, __FUNCTION__ << " Error: failed to get BitCount from PixelFormat! " << ddpfPixelFormat);
	return 0;
}

// https://github.com/microsoft/Windows-driver-samples/blob/master/video/pixlib/pixel.cpp
DWORD GetBitCount(D3DFORMAT Format)
{
	switch (DWORD(Format))
	{
	case D3DFMT_DXT1:
		// Size is negative to indicate DXT; and indicates
		// the size of the block
		return (UINT)(-64);

	case D3DFMT_DXT2:
	case D3DFMT_DXT3:
	case D3DFMT_DXT4:
	case D3DFMT_DXT5:
		// Size is negative to indicate DXT; and indicates
		// the size of the block
		return (UINT)(-128);

	case D3DFMT_A32B32G32R32F:
		return 128;

	case D3DFMT_A16B16G16R16:
	case D3DFMT_Q16W16V16U16:
	case D3DFMT_A16B16G16R16F:
	case D3DFMT_G32R32F:
	case D3DFMT_MULTI2_ARGB8:
		return 64;

	case D3DFMT_A8R8G8B8:
	case D3DFMT_X8R8G8B8:
	case D3DFMT_D32:
	case D3DFMT_D24S8:
	case D3DFMT_S8D24:
	case D3DFMT_AYUV:
	case D3DFMT_X8L8V8U8:
	case D3DFMT_X4S4D24:
	case D3DFMT_D24X4S4:
	case D3DFMT_Q8W8V8U8:
	case D3DFMT_V16U16:
	case D3DFMT_A2W10V10U10:
	case D3DFMT_A2B10G10R10:
	case D3DFMT_A8B8G8R8:
	case D3DFMT_X8B8G8R8:
	case D3DFMT_G16R16:
	case D3DFMT_D24X8:
	case D3DFMT_X8D24:
	case D3DFMT_W11V11U10:
	case D3DFMT_A2R10G10B10:
	case D3DFMT_G16R16F:
	case D3DFMT_R32F:
	case D3DFMT_D32F_LOCKABLE:
	case D3DFMT_D24FS8:
	case D3DFMT_D32_LOCKABLE:
		return 32;

	case D3DFMT_R8G8B8:
	case D3DFMT_B8G8R8:
		return 24;

	case D3DFMT_R5G6B5:
	case D3DFMT_X1R5G5B5:
	case D3DFMT_A1R5G5B5:
	case D3DFMT_A4R4G4B4:
	case D3DFMT_A8L8:
	case D3DFMT_V8U8:
	case D3DFMT_L6V5U5:
	case D3DFMT_D16:
	case D3DFMT_D16_LOCKABLE:
	case D3DFMT_D15S1:
	case D3DFMT_S1D15:
	case D3DFMT_A8P8:
	case D3DFMT_A8R3G3B2:
	case D3DFMT_UYVY:
	case D3DFMT_YUY2:
	case D3DFMT_X4R4G4B4:
	case D3DFMT_CxV8U8:
	case D3DFMT_L16:
	case D3DFMT_R16F:
	case D3DFMT_R8G8_B8G8:
	case D3DFMT_G8R8_G8B8:
		return 16;

	case D3DFMT_YV12:
	case D3DFMT_NV12:
		return 12;

	case D3DFMT_P8:
	case D3DFMT_L8:
	case D3DFMT_R3G3B2:
	case D3DFMT_A4L4:
	case D3DFMT_A8:
	case D3DFMT_A1:
	case D3DFMT_S8_LOCKABLE:
		return 8;

	default:
		LOG_LIMIT(100, __FUNCTION__ << " Error: Display format not Implemented: " << Format);
		return 0;
	};
}

float ConvertDepthValue(DWORD dwFillDepth, D3DFORMAT Format)
{
	switch ((DWORD)Format)
	{
	case D3DFMT_S1D15:
		return static_cast<float>(dwFillDepth & 0x7FFF) / 0x7FFF; // 15-bit depth

	case D3DFMT_D15S1:
		// Shift the depth value to the right by 1 bits before extracting
		return static_cast<float>((dwFillDepth >> 1) & 0x7FFF) / 0x7FFF; // 15-bit depth

	case D3DFMT_D16:
	case D3DFMT_D16_LOCKABLE:
		return static_cast<float>(dwFillDepth & 0xFFFF) / 0xFFFF; // 16-bit depth

	case D3DFMT_X8D24:
	case D3DFMT_S8D24:
	case D3DFMT_X4S4D24:
		return static_cast<float>(dwFillDepth & 0xFFFFFF) / 0xFFFFFF; // 24-bit depth

	case D3DFMT_D24X8:
	case D3DFMT_D24S8:
	case D3DFMT_D24FS8:
	case D3DFMT_D24X4S4:
		// Shift the depth value to the right by 8 bits before extracting
		return static_cast<float>((dwFillDepth >> 8) & 0xFFFFFF) / 0xFFFFFF; // 24-bit depth

	case D3DFMT_D32:
	case D3DFMT_D32_LOCKABLE:
	case D3DFMT_D32F_LOCKABLE:
		return (float)(static_cast<double>(dwFillDepth & 0xFFFFFFFF) / 0xFFFFFFFF); // 32-bit depth

	default:
		LOG_LIMIT(100, __FUNCTION__ << " Error: Depth Stencil format not Implemented: " << Format);
		return 0.0f;
	}
}

DWORD ComputePitch(D3DFORMAT Format, DWORD Width, DWORD Height)
{
	if (ISDXTEX(Format))
	{
		return max(1, (GetByteAlignedWidth(Width, GetBitCount(Format)) + 3) / 4) * max(1, (Height + 3) / 4) * (Format == D3DFMT_DXT1 ? 8 : 16);
	}
	else if (Format == D3DFMT_YV12 || Format == D3DFMT_NV12)
	{
		return GetByteAlignedWidth(Width, GetBitCount(Format));
	}
	else if (Format & 0xFF000000)	// Other FourCC types
	{
		return 0;
	}
	else
	{
		return ((((Width * GetBitCount(Format)) + 31) & ~31) >> 3);	// Use Surface Stride for pitch
	}
}

DWORD GetSurfaceSize(D3DFORMAT Format, DWORD Width, DWORD Height, INT Pitch)
{
	if (Format & 0xFF000000)	// All FourCC types
	{
		if (ISDXTEX(Format))
		{
			return ComputePitch(Format, Width, Height);	// Picth for DXT surfaces is the full surface size
		}
		else
		{
			DWORD Size = ComputePitch(Format, Width, Height);
			if (Size)
			{
				return Size * Height;
			}
			else if (Pitch)
			{
				return Pitch * Height;
			}
		}
	}
	else
	{
		return Pitch * Height;
	}

	LOG_LIMIT(100, __FUNCTION__ << " Error: Surface size for surface format not Implemented: " << Format);
	return 0;
}

// Count leading zeros and total number of bits
inline void CountBits(DWORD value, DWORD& LeadingZeros, DWORD& TotalBits)
{
	LeadingZeros = 0;
	while (value && !(value & 1))
	{
		LeadingZeros++;
		value >>= 1;
	}
	TotalBits = 0;
	while (value)
	{
		TotalBits += value & 1;
		value >>= 1;
	}
}

DWORD GetARGBColorKey(DWORD ColorKey, const DDPIXELFORMAT& ddpfPixelFormat)
{
	if (!ddpfPixelFormat.dwRBitMask || !ddpfPixelFormat.dwGBitMask || !ddpfPixelFormat.dwBBitMask)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: pixel format not Implemented: " << ddpfPixelFormat);
		return 0;
	}

	DWORD dwRBitCount, dwGBitCount, dwBBitCount, rShift, gShift, bShift;

	// Calculate bits for each color component
	CountBits(ddpfPixelFormat.dwRBitMask, rShift, dwRBitCount);
	CountBits(ddpfPixelFormat.dwGBitMask, gShift, dwGBitCount);
	CountBits(ddpfPixelFormat.dwBBitMask, bShift, dwBBitCount);

	// Calculate size of color space for bit depth
	const float rColorRange = 255.0f / (ddpfPixelFormat.dwRBitMask >> rShift);
	const float gColorRange = 255.0f / (ddpfPixelFormat.dwGBitMask >> gShift);
	const float bColorRange = 255.0f / (ddpfPixelFormat.dwBBitMask >> bShift);

	// Extract individual components according to pixel format for low color key
	DWORD r = static_cast<DWORD>(((ColorKey & ddpfPixelFormat.dwRBitMask) >> rShift) * rColorRange);
	DWORD g = static_cast<DWORD>(((ColorKey & ddpfPixelFormat.dwGBitMask) >> gShift) * gColorRange);
	DWORD b = static_cast<DWORD>(((ColorKey & ddpfPixelFormat.dwBBitMask) >> bShift) * bColorRange);

	// Return ARGB color key
	return D3DCOLOR_ARGB(0xFF, r, g, b);
}

void GetColorKeyArray(float(&lowColorKey)[4], float(&highColorKey)[4], DWORD lowColorSpace, DWORD highColorSpace, const DDPIXELFORMAT& ddpfPixelFormat)
{
	if (!ddpfPixelFormat.dwRBitMask || !ddpfPixelFormat.dwGBitMask || !ddpfPixelFormat.dwBBitMask)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: pixel format not Implemented: " << ddpfPixelFormat);
		return;
	}

	DWORD dwRBitCount, dwGBitCount, dwBBitCount, rShift, gShift, bShift;

	// Calculate bits for each color component
	CountBits(ddpfPixelFormat.dwRBitMask, rShift, dwRBitCount);
	CountBits(ddpfPixelFormat.dwGBitMask, gShift, dwGBitCount);
	CountBits(ddpfPixelFormat.dwBBitMask, bShift, dwBBitCount);

	// Calculate size of color space for bit depth
	const float rColorRange = 255.0f / (ddpfPixelFormat.dwRBitMask >> rShift);
	const float gColorRange = 255.0f / (ddpfPixelFormat.dwGBitMask >> gShift);
	const float bColorRange = 255.0f / (ddpfPixelFormat.dwBBitMask >> bShift);

	// Allow some range for padding (half of a pixel's color range)
	const float rPadding = (rColorRange / 255.0f) * 0.1f;
	const float gPadding = (gColorRange / 255.0f) * 0.1f;
	const float bPadding = (bColorRange / 255.0f) * 0.1f;

	// Extract individual components according to pixel format for low color key
	BYTE r = (BYTE)((lowColorSpace & ddpfPixelFormat.dwRBitMask) >> rShift);
	BYTE g = (BYTE)((lowColorSpace & ddpfPixelFormat.dwGBitMask) >> gShift);
	BYTE b = (BYTE)((lowColorSpace & ddpfPixelFormat.dwBBitMask) >> bShift);

	// Convert to float and normalize to range [0, 1] for low color key
	lowColorKey[0] = (r * rColorRange / 255.0f) - rPadding;
	lowColorKey[0] = lowColorKey[0] < 0.0f ? 0.0f : lowColorKey[0];
	lowColorKey[1] = (g * gColorRange / 255.0f) - gPadding;
	lowColorKey[1] = lowColorKey[1] < 0.0f ? 0.0f : lowColorKey[1];
	lowColorKey[2] = (b * bColorRange / 255.0f) - bPadding;
	lowColorKey[2] = lowColorKey[2] < 0.0f ? 0.0f : lowColorKey[2];
	lowColorKey[3] = 0.0f;

	// Extract individual components according to pixel format for high color key
	r = (BYTE)((highColorSpace & ddpfPixelFormat.dwRBitMask) >> rShift);
	g = (BYTE)((highColorSpace & ddpfPixelFormat.dwGBitMask) >> gShift);
	b = (BYTE)((highColorSpace & ddpfPixelFormat.dwBBitMask) >> bShift);

	// Convert to float and normalize to range [0, 1] for high color key
	highColorKey[0] = (r * rColorRange / 255.0f) + rPadding;
	highColorKey[0] = highColorKey[0] > 1.0f ? 1.0f : highColorKey[0];
	highColorKey[1] = (g * gColorRange / 255.0f) + gPadding;
	highColorKey[1] = highColorKey[1] > 1.0f ? 1.0f : highColorKey[1];
	highColorKey[2] = (b * bColorRange / 255.0f) + bPadding;
	highColorKey[2] = highColorKey[2] > 1.0f ? 1.0f : highColorKey[2];
	highColorKey[3] = 0.0f;
}

bool IsPixelFormatRGB(const DDPIXELFORMAT& ddpfPixelFormat)
{
	if (ddpfPixelFormat.dwFlags & (DDPF_ALPHA | DDPF_ALPHAPREMULT | DDPF_FOURCC | DDPF_COMPRESSED |
		DDPF_PALETTEINDEXED8 | DDPF_PALETTEINDEXEDTO8 | DDPF_PALETTEINDEXED4 | DDPF_PALETTEINDEXED2 | DDPF_PALETTEINDEXED1 |
		DDPF_LUMINANCE | DDPF_BUMPLUMINANCE | DDPF_BUMPDUDV | DDPF_RGBTOYUV | DDPF_YUV |
		DDPF_ZBUFFER | DDPF_ZPIXELS | DDPF_STENCILBUFFER))
	{
		return false;
	}
	if (ddpfPixelFormat.dwFlags & (DDPF_RGB))
	{
		return true;
	}
	return false;
}

bool IsPixelFormatPalette(const DDPIXELFORMAT& ddpfPixelFormat)
{
	if (ddpfPixelFormat.dwFlags & (DDPF_ALPHA | DDPF_ALPHAPREMULT | DDPF_FOURCC | DDPF_COMPRESSED |
		DDPF_LUMINANCE | DDPF_BUMPLUMINANCE | DDPF_BUMPDUDV | DDPF_RGBTOYUV | DDPF_YUV |
		DDPF_ZBUFFER | DDPF_ZPIXELS | DDPF_STENCILBUFFER))
	{
		return false;
	}
	if (ddpfPixelFormat.dwFlags & (DDPF_PALETTEINDEXED8 | DDPF_PALETTEINDEXEDTO8 | DDPF_PALETTEINDEXED4 | DDPF_PALETTEINDEXED2 | DDPF_PALETTEINDEXED1))
	{
		return true;
	}
	return false;
}

D3DFORMAT ConvertSurfaceFormat(D3DFORMAT Format)
{
	return (Format == D3DFMT_X8B8G8R8 || Format == D3DFMT_B8G8R8 || Format == D3DFMT_R8G8B8) ? D3DFMT_X8R8G8B8 :
		(Format == D3DFMT_A8B8G8R8) ? D3DFMT_A8R8G8B8 : Format;
}

D3DFORMAT GetFailoverFormat(D3DFORMAT Format)
{
	std::vector<std::pair<D3DFORMAT, D3DFORMAT>> FormatVector =
	{
		{ D3DFMT_P8, D3DFMT_L8 },
		{ D3DFMT_X1R5G5B5, D3DFMT_A1R5G5B5 },
		{ D3DFMT_X4R4G4B4, D3DFMT_A4R4G4B4 },
		{ D3DFMT_X8R8G8B8, D3DFMT_A8R8G8B8 },
		{ D3DFMT_X8B8G8R8, D3DFMT_A8B8G8R8 },
		{ D3DFMT_D32, D3DFMT_D24S8 },
		{ D3DFMT_D16_LOCKABLE, D3DFMT_D16 }
	};

	for (const auto& FormatPair : FormatVector)
	{
		if (Format == FormatPair.first)
		{
			return FormatPair.second;
		}
	}

	return Format;
}

D3DFORMAT GetDisplayFormat(const DDPIXELFORMAT& ddpfPixelFormat)
{
	if (ddpfPixelFormat.dwSize != sizeof(DDPIXELFORMAT))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << ddpfPixelFormat.dwSize);
		return D3DFMT_UNKNOWN;
	}
	if (ddpfPixelFormat.dwFlags & (DDPF_PALETTEINDEXED1 | DDPF_PALETTEINDEXED2 | DDPF_PALETTEINDEXED4))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: 1-bit, 2-bit and 4-bit palette formats not Implemented");
		return D3DFMT_UNKNOWN;
	}
	if (ddpfPixelFormat.dwFlags & DDPF_FOURCC)
	{
		switch (ddpfPixelFormat.dwFourCC)
		{
		case D3DFMT_DXT1:
		case D3DFMT_DXT2:
		case D3DFMT_DXT3:
		case D3DFMT_DXT4:
		case D3DFMT_DXT5:
		case D3DFMT_AYUV:
		case D3DFMT_UYVY:
		case D3DFMT_YUY2:
		case D3DFMT_YV12:
		case D3DFMT_NV12:
		case D3DFMT_MULTI2_ARGB8:
		case D3DFMT_G8R8_G8B8:
		case D3DFMT_R8G8_B8G8:
			break;
		default:
			LOG_LIMIT(100, __FUNCTION__ << " Warning: FourCC format not Implemented. Code = " << (DDFOURCC)ddpfPixelFormat.dwFourCC);
			break;
		}
		return (D3DFORMAT)ddpfPixelFormat.dwFourCC;
	}
	if (ddpfPixelFormat.dwFlags & DDPF_YUV)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Warning: YUV format not Implemented");
	}
	if (ddpfPixelFormat.dwFlags & DDPF_RGBTOYUV)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Warning: RGB to YUV format not Implemented");
	}
	if (ddpfPixelFormat.dwFlags & DDPF_ALPHAPREMULT)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Warning: premultiplied alpha format not Implemented");
	}
	if (ddpfPixelFormat.dwFlags & DDPF_ZPIXELS)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Warning: z-pixels not Implemented");
	}

	if (ddpfPixelFormat.dwFlags & (DDPF_RGB | DDPF_PALETTEINDEXED8 | DDPF_LUMINANCE | DDPF_ALPHA))
	{
		switch (GetBitCount(ddpfPixelFormat))
		{
		case 8:
			if (((ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS) || (ddpfPixelFormat.dwLuminanceAlphaBitMask == 0xF0)) &&
				(ddpfPixelFormat.dwFlags & DDPF_LUMINANCE) && (ddpfPixelFormat.dwLuminanceBitMask == 0x0F))
			{
				return D3DFMT_A4L4;
			}
			if (ddpfPixelFormat.dwFlags & DDPF_ALPHA)
			{
				return D3DFMT_A8;
			}
			if ((ddpfPixelFormat.dwFlags & DDPF_LUMINANCE) && (ddpfPixelFormat.dwLuminanceBitMask == 0xFF))
			{
				return D3DFMT_L8;
			}
			if (ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8)
			{
				return D3DFMT_P8;
			}
			if (ddpfPixelFormat.dwRBitMask == 0xE0 && ddpfPixelFormat.dwGBitMask == 0x1C && ddpfPixelFormat.dwBBitMask == 0x03)
			{
				return D3DFMT_R3G3B2;
			}
			break;
		case 16:
			if (((ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS) || (ddpfPixelFormat.dwLuminanceAlphaBitMask == 0xFF00)) &&
				(ddpfPixelFormat.dwFlags & DDPF_LUMINANCE) && (ddpfPixelFormat.dwLuminanceBitMask == 0x00FF))
			{
				return D3DFMT_A8L8;
			}
			if ((ddpfPixelFormat.dwRBitMask == 0x7C00) && (ddpfPixelFormat.dwGBitMask == 0x03E0) && (ddpfPixelFormat.dwBBitMask == 0x001F))
			{
				if ((ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS) || (ddpfPixelFormat.dwRGBAlphaBitMask == 0x8000))
				{
					return D3DFMT_A1R5G5B5;
				}
				else if (ddpfPixelFormat.dwRGBAlphaBitMask == 0x0000)
				{
					return D3DFMT_X1R5G5B5;
				}
			}
			if ((ddpfPixelFormat.dwRBitMask == 0xF800) && (ddpfPixelFormat.dwGBitMask == 0x07E0) && (ddpfPixelFormat.dwBBitMask == 0x001F))
			{
				return D3DFMT_R5G6B5;
			}
			if ((ddpfPixelFormat.dwRBitMask == 0x0F00) && (ddpfPixelFormat.dwGBitMask == 0x00F0) && (ddpfPixelFormat.dwBBitMask == 0x000F) &&
				((ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS) || (ddpfPixelFormat.dwRGBAlphaBitMask == 0xF000)))
			{
				return D3DFMT_A4R4G4B4;
			}
			if (((ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS) || ddpfPixelFormat.dwRGBAlphaBitMask == 0xFF00) &&
				ddpfPixelFormat.dwRBitMask == 0x00E0 && ddpfPixelFormat.dwGBitMask == 0x001C && ddpfPixelFormat.dwBBitMask == 0x0003)
			{
				return D3DFMT_A8R3G3B2;
			}
			break;
		case 24:
			if ((ddpfPixelFormat.dwRBitMask == 0xFF0000) && (ddpfPixelFormat.dwGBitMask == 0x00FF00) && (ddpfPixelFormat.dwBBitMask == 0x0000FF))
			{
				return D3DFMT_R8G8B8;
			}
			if ((ddpfPixelFormat.dwRBitMask == 0x0000FF) && (ddpfPixelFormat.dwGBitMask == 0x00FF00) && (ddpfPixelFormat.dwBBitMask == 0xFF0000))
			{
				return D3DFMT_B8G8R8;
			}
			break;
		case 32:
			if ((ddpfPixelFormat.dwRBitMask == 0xFF0000) && (ddpfPixelFormat.dwGBitMask == 0x00FF00) && (ddpfPixelFormat.dwBBitMask == 0x0000FF))
			{
				if ((ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS) || (ddpfPixelFormat.dwRGBAlphaBitMask == 0xFF000000))
				{
					return D3DFMT_A8R8G8B8;
				}
				else if (ddpfPixelFormat.dwRGBAlphaBitMask == 0x00)
				{
					return D3DFMT_X8R8G8B8;
				}
			}
			if ((ddpfPixelFormat.dwRBitMask == 0x0000FF) && (ddpfPixelFormat.dwGBitMask == 0x00FF00) && (ddpfPixelFormat.dwBBitMask == 0xFF0000))
			{
				if ((ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS) || (ddpfPixelFormat.dwRGBAlphaBitMask == 0xFF000000))
				{
					return D3DFMT_A8B8G8R8;
				}
				else if (ddpfPixelFormat.dwRGBAlphaBitMask == 0x00000000)
				{
					return D3DFMT_X8B8G8R8;
				}
			}
			if ((ddpfPixelFormat.dwRBitMask == 0x3FF00000) && (ddpfPixelFormat.dwGBitMask == 0x0000FFC00) && (ddpfPixelFormat.dwBBitMask == 0x000003FF) &&
				((ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS) || (ddpfPixelFormat.dwRGBAlphaBitMask == 0xC0000000)))
			{
				return D3DFMT_A2R10G10B10;
			}
			break;
		}
		LOG_LIMIT(100, __FUNCTION__ << " Error: could not find RGB format for PixelFormat: " << ddpfPixelFormat);
		return D3DFMT_UNKNOWN;
	}

	// Bump formats
	if (ddpfPixelFormat.dwFlags & DDPF_BUMPDUDV)
	{
		if (ddpfPixelFormat.dwBumpBitCount == 16 && ddpfPixelFormat.dwBumpDuBitMask == 0x00FF && ddpfPixelFormat.dwBumpDvBitMask == 0xFF00)
		{
			return D3DFMT_V8U8;
		}
		if (ddpfPixelFormat.dwFlags & DDPF_BUMPLUMINANCE && ddpfPixelFormat.dwBumpBitCount == 16 &&
			ddpfPixelFormat.dwBumpDuBitMask == 0x0000001F && ddpfPixelFormat.dwBumpDvBitMask == 0x000003E0 && ddpfPixelFormat.dwBumpLuminanceBitMask == 0x0000FC00)
		{
			return D3DFMT_L6V5U5;
		}
		if (ddpfPixelFormat.dwFlags & DDPF_BUMPLUMINANCE && ddpfPixelFormat.dwBumpBitCount == 32 &&
			ddpfPixelFormat.dwBumpDuBitMask == 0x000000FF && ddpfPixelFormat.dwBumpDvBitMask == 0x0000FF00 && ddpfPixelFormat.dwBumpLuminanceBitMask == 0x00FF0000)
		{
			return D3DFMT_X8L8V8U8;
		}
		LOG_LIMIT(100, __FUNCTION__ << " Error: could not find Bump format for PixelFormat: " << ddpfPixelFormat);
		return D3DFMT_UNKNOWN;
	}

	// zBuffer formats
	if (ddpfPixelFormat.dwFlags & (DDPF_ZBUFFER | DDPF_STENCILBUFFER))
	{
		switch (ddpfPixelFormat.dwZBufferBitDepth)
		{
		case 16:
			if (ddpfPixelFormat.dwFlags & DDPF_STENCILBUFFER && ddpfPixelFormat.dwStencilBitDepth == 1)
			{
				if (ddpfPixelFormat.dwZBitMask == 0xFFFE && ddpfPixelFormat.dwStencilBitMask == 0x01)
				{
					return D3DFMT_D15S1;
				}
				else if (ddpfPixelFormat.dwZBitMask == 0x7FFF && ddpfPixelFormat.dwStencilBitMask == 0x8000)
				{
					return D3DFMT_S1D15;
				}
			}
			if (ddpfPixelFormat.dwZBitMask == 0xFFFF)
			{
				return D3DFMT_D16_LOCKABLE;
			}
			break;
		case 24:
			{
				return D3DFMT_D24X8;
			}
			break;
		case 32:
			if (ddpfPixelFormat.dwZBitMask == 0xFFFFFFFF)
			{
				return D3DFMT_D32;
			}
			if (ddpfPixelFormat.dwZBitMask == 0xFFFFFF00 || ddpfPixelFormat.dwZBitMask == 0x00FFFFFF)
			{
				if (ddpfPixelFormat.dwFlags & DDPF_STENCILBUFFER)
				{
					if (ddpfPixelFormat.dwStencilBitDepth == 8 && (ddpfPixelFormat.dwZBitMask | ddpfPixelFormat.dwStencilBitMask) == 0xFFFFFFFF)
					{
						return D3DFMT_D24S8;
					}
					if (ddpfPixelFormat.dwStencilBitDepth == 4)
					{
						return D3DFMT_D24X4S4;
					}
				}
				else
				{
					return D3DFMT_D24X8;
				}
			}
			break;
		}
		LOG_LIMIT(100, __FUNCTION__ << " Error: could not find z-buffer format for PixelFormat: " << ddpfPixelFormat);
		return D3DFMT_UNKNOWN;
	}

	LOG_LIMIT(100, __FUNCTION__ << " Error: PixelFormat not implemented: " << ddpfPixelFormat);
	return D3DFMT_UNKNOWN;
}

void SetPixelDisplayFormat(D3DFORMAT Format, DDPIXELFORMAT& ddpfPixelFormat)
{
	if (ddpfPixelFormat.dwSize != sizeof(DDPIXELFORMAT))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << ddpfPixelFormat.dwSize);
		return;
	}

	// Initialize pixel format
	ddpfPixelFormat.dwFlags = 0;
	ddpfPixelFormat.dwFourCC = 0;
	ddpfPixelFormat.dwRGBBitCount = 0;
	ddpfPixelFormat.dwRBitMask = 0x0;
	ddpfPixelFormat.dwGBitMask = 0x0;
	ddpfPixelFormat.dwBBitMask = 0x0;
	ddpfPixelFormat.dwRGBAlphaBitMask = 0x0;

	// Set BitCount and BitMask
	switch ((DWORD)Format)
	{
	// 8-bit RGB
	case D3DFMT_R3G3B2:
		ddpfPixelFormat.dwFlags = DDPF_RGB;
		ddpfPixelFormat.dwRGBBitCount = 8;
		ddpfPixelFormat.dwRBitMask = 0xE0;
		ddpfPixelFormat.dwGBitMask = 0x1C;
		ddpfPixelFormat.dwBBitMask = 0x03;
		break;
	// 16-bit RGB
	case D3DFMT_R5G6B5:
		ddpfPixelFormat.dwFlags = DDPF_RGB;
		ddpfPixelFormat.dwRGBBitCount = 16;
		ddpfPixelFormat.dwRBitMask = 0xF800;
		ddpfPixelFormat.dwGBitMask = 0x07E0;
		ddpfPixelFormat.dwBBitMask = 0x001F;
		break;
	case D3DFMT_A1R5G5B5:
		ddpfPixelFormat.dwFlags = DDPF_ALPHAPIXELS;
		ddpfPixelFormat.dwRGBAlphaBitMask = 0x8000;
		[[fallthrough]];
	case D3DFMT_X1R5G5B5:
		ddpfPixelFormat.dwFlags |= DDPF_RGB;
		ddpfPixelFormat.dwRGBBitCount = 16;
		ddpfPixelFormat.dwRBitMask = 0x7C00;
		ddpfPixelFormat.dwGBitMask = 0x03E0;
		ddpfPixelFormat.dwBBitMask = 0x001F;
		break;
	case D3DFMT_A4R4G4B4:
		ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
		ddpfPixelFormat.dwRGBBitCount = 16;
		ddpfPixelFormat.dwRGBAlphaBitMask = 0xF000;
		ddpfPixelFormat.dwRBitMask = 0x0F00;
		ddpfPixelFormat.dwGBitMask = 0x00F0;
		ddpfPixelFormat.dwBBitMask = 0x000F;
		break;
	case D3DFMT_A8R3G3B2:
		ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
		ddpfPixelFormat.dwRGBBitCount = 16;
		ddpfPixelFormat.dwRGBAlphaBitMask = 0xFF00;
		ddpfPixelFormat.dwRBitMask = 0x00E0;
		ddpfPixelFormat.dwGBitMask = 0x001C;
		ddpfPixelFormat.dwBBitMask = 0x0003;
		break;

	// 24-bit RGB
	case D3DFMT_R8G8B8:
		ddpfPixelFormat.dwFlags = DDPF_RGB;
		ddpfPixelFormat.dwRGBBitCount = 24;
		ddpfPixelFormat.dwRBitMask = 0xFF0000;
		ddpfPixelFormat.dwGBitMask = 0x00FF00;
		ddpfPixelFormat.dwBBitMask = 0x0000FF;
		break;
	case D3DFMT_B8G8R8:
		ddpfPixelFormat.dwFlags = DDPF_RGB;
		ddpfPixelFormat.dwRGBBitCount = 24;
		ddpfPixelFormat.dwRBitMask = 0x0000FF;
		ddpfPixelFormat.dwGBitMask = 0x00FF00;
		ddpfPixelFormat.dwBBitMask = 0xFF0000;
		break;

	// 32-bit RGB
	case D3DFMT_A8R8G8B8:
		ddpfPixelFormat.dwFlags = DDPF_ALPHAPIXELS;
		ddpfPixelFormat.dwRGBAlphaBitMask = 0xFF000000;
		[[fallthrough]];
	case D3DFMT_X8R8G8B8:
		ddpfPixelFormat.dwFlags |= DDPF_RGB;
		ddpfPixelFormat.dwRGBBitCount = 32;
		ddpfPixelFormat.dwRBitMask = 0x00FF0000;
		ddpfPixelFormat.dwGBitMask = 0x0000FF00;
		ddpfPixelFormat.dwBBitMask = 0x000000FF;
		break;
	case D3DFMT_A8B8G8R8:
		ddpfPixelFormat.dwFlags = DDPF_ALPHAPIXELS;
		ddpfPixelFormat.dwRGBAlphaBitMask = 0xFF000000;
		[[fallthrough]];
	case D3DFMT_X8B8G8R8:
		ddpfPixelFormat.dwFlags |= DDPF_RGB;
		ddpfPixelFormat.dwRGBBitCount = 32;
		ddpfPixelFormat.dwRBitMask = 0x000000FF;
		ddpfPixelFormat.dwGBitMask = 0x0000FF00;
		ddpfPixelFormat.dwBBitMask = 0x00FF0000;
		break;
	case D3DFMT_A2R10G10B10:
		ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
		ddpfPixelFormat.dwRGBBitCount = 32;
		ddpfPixelFormat.dwRGBAlphaBitMask = 0xC0000000;
		ddpfPixelFormat.dwRBitMask = 0x3FF00000;
		ddpfPixelFormat.dwGBitMask = 0x000FFC00;
		ddpfPixelFormat.dwBBitMask = 0x000003FF;
		break;

	// Palette
	case D3DFMT_P8:
		ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_PALETTEINDEXED8;
		ddpfPixelFormat.dwRGBBitCount = 8;
		break;

	// Bump
	case D3DFMT_V8U8:
		ddpfPixelFormat.dwFlags = DDPF_BUMPDUDV;
		ddpfPixelFormat.dwBumpBitCount = 16;
		ddpfPixelFormat.dwBumpDuBitMask = 0x00FF;
		ddpfPixelFormat.dwBumpDvBitMask = 0xFF00;
		break;
	case D3DFMT_L6V5U5:
		ddpfPixelFormat.dwFlags = DDPF_BUMPDUDV | DDPF_BUMPLUMINANCE;
		ddpfPixelFormat.dwBumpBitCount = 16;
		ddpfPixelFormat.dwBumpDuBitMask = 0x0000001F;
		ddpfPixelFormat.dwBumpDvBitMask = 0x000003E0;
		ddpfPixelFormat.dwBumpLuminanceBitMask = 0x0000FC00;
		break;
	case D3DFMT_X8L8V8U8:
		ddpfPixelFormat.dwFlags = DDPF_BUMPDUDV | DDPF_BUMPLUMINANCE;
		ddpfPixelFormat.dwBumpBitCount = 32;
		ddpfPixelFormat.dwBumpDuBitMask = 0x000000FF;
		ddpfPixelFormat.dwBumpDvBitMask = 0x0000FF00;
		ddpfPixelFormat.dwBumpLuminanceBitMask = 0x00FF0000;
		break;

	// Alpha
	case D3DFMT_A8:
		ddpfPixelFormat.dwFlags = DDPF_ALPHA;
		ddpfPixelFormat.dwAlphaBitDepth = 8;
		break;

	// Luminance
	case D3DFMT_L8:
		ddpfPixelFormat.dwFlags = DDPF_LUMINANCE;
		ddpfPixelFormat.dwLuminanceBitCount = 8;
		ddpfPixelFormat.dwLuminanceBitMask = 0xFF;
		break;
	case D3DFMT_A4L4:
		ddpfPixelFormat.dwFlags = DDPF_LUMINANCE | DDPF_ALPHAPIXELS;
		ddpfPixelFormat.dwLuminanceBitCount = 8;
		ddpfPixelFormat.dwLuminanceAlphaBitMask = 0xF0;
		ddpfPixelFormat.dwLuminanceBitMask = 0x0F;
		break;
	case D3DFMT_A8L8:
		ddpfPixelFormat.dwFlags = DDPF_LUMINANCE | DDPF_ALPHAPIXELS;
		ddpfPixelFormat.dwLuminanceBitCount = 16;
		ddpfPixelFormat.dwLuminanceAlphaBitMask = 0xFF00;
		ddpfPixelFormat.dwLuminanceBitMask = 0x00FF;
		break;

	// zBuffer formats
	case D3DFMT_D16:
	case D3DFMT_D16_LOCKABLE:
		ddpfPixelFormat.dwFlags = DDPF_ZBUFFER;
		ddpfPixelFormat.dwZBufferBitDepth = 16;
		ddpfPixelFormat.dwZBitMask = 0xFFFF;
		break;
	case D3DFMT_D15S1:
	case D3DFMT_S1D15:
		ddpfPixelFormat.dwFlags = DDPF_ZBUFFER | DDPF_STENCILBUFFER;
		ddpfPixelFormat.dwZBufferBitDepth = 16;
		ddpfPixelFormat.dwStencilBitDepth = 1;
		ddpfPixelFormat.dwZBitMask = 0x7FFF;
		ddpfPixelFormat.dwStencilBitMask = 0x8000;
		break;
	case D3DFMT_D32:
		ddpfPixelFormat.dwFlags = DDPF_ZBUFFER;
		ddpfPixelFormat.dwZBufferBitDepth = 32;
		ddpfPixelFormat.dwZBitMask = 0xFFFFFFFF;
		break;
	case D3DFMT_D24S8:
	case D3DFMT_S8D24:
		ddpfPixelFormat.dwFlags = DDPF_STENCILBUFFER;
		ddpfPixelFormat.dwStencilBitDepth = 8;
		ddpfPixelFormat.dwStencilBitMask = 0xFF000000;
		[[fallthrough]];
	case D3DFMT_D24X8:
	case D3DFMT_X8D24:
		ddpfPixelFormat.dwFlags |= DDPF_ZBUFFER;
		ddpfPixelFormat.dwZBufferBitDepth = 32;
		ddpfPixelFormat.dwZBitMask = 0x00FFFFFF;
		break;
	case D3DFMT_D24X4S4:
	case D3DFMT_X4S4D24:
		ddpfPixelFormat.dwFlags = DDPF_ZBUFFER | DDPF_STENCILBUFFER;
		ddpfPixelFormat.dwZBufferBitDepth = 32;
		ddpfPixelFormat.dwStencilBitDepth = 4;
		ddpfPixelFormat.dwZBitMask = 0x00FFFFFF;
		ddpfPixelFormat.dwStencilBitMask = 0x0F000000;
		break;

	// FourCC
	case D3DFMT_AYUV:
	case D3DFMT_UYVY:
	case D3DFMT_YUY2:
	case D3DFMT_YV12:
	case D3DFMT_NV12:
		ddpfPixelFormat.dwFlags = DDPF_FOURCC;
		ddpfPixelFormat.dwFourCC = Format;
		break;
	default:
		// FourCC
		if (Format & 0xFF000000)
		{
			ddpfPixelFormat.dwFlags = DDPF_FOURCC;
			ddpfPixelFormat.dwFourCC = Format;
			break;
		}
		// Not supported
		else
		{
			LOG_LIMIT(100, __FUNCTION__ << " Display format not Implemented: " << Format);
			ddpfPixelFormat.dwFlags = 0;
			ddpfPixelFormat.dwRGBBitCount = 0;
			return;
		}
	}
}

D3DFORMAT SetDisplayFormat(DDPIXELFORMAT& ddpfPixelFormat, DWORD BPP)
{
	D3DFORMAT Format = D3DFMT_UNKNOWN;
	switch (BPP)
	{
	case 8:
		Format = D3DFMT_P8;
		break;
	case 16:
		Format = D3DFMT_R5G6B5;
		break;
	case 24:
		Format = D3DFMT_R8G8B8;
		break;
	case 32:
		Format = D3DFMT_X8R8G8B8;
		break;
	default:
		LOG_LIMIT(100, __FUNCTION__ << " Error: Bit mode not supported: " << BPP);
		ddpfPixelFormat.dwFlags = 0;
		ddpfPixelFormat.dwFourCC = 0;
		ddpfPixelFormat.dwRGBBitCount = 0;
		ddpfPixelFormat.dwRBitMask = 0x0;
		ddpfPixelFormat.dwGBitMask = 0x0;
		ddpfPixelFormat.dwBBitMask = 0x0;
		ddpfPixelFormat.dwRGBAlphaBitMask = 0x0;
		return Format;
	}
	SetPixelDisplayFormat(Format, ddpfPixelFormat);
	return Format;
}
