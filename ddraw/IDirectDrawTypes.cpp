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
