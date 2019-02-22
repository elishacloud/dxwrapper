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

void ConvertMaterial(D3DMATERIAL &Material, D3DMATERIAL7 &Material7)
{
	D3DMATERIAL tmpMaterial;
	tmpMaterial.dwSize = min(sizeof(D3DMATERIAL), Material.dwSize);
	tmpMaterial.dcvDiffuse = Material7.dcvDiffuse;
	tmpMaterial.dcvAmbient = Material7.dcvAmbient;
	tmpMaterial.dcvSpecular = Material7.dcvSpecular;
	tmpMaterial.dcvEmissive = Material7.dcvEmissive;
	tmpMaterial.dvPower = Material7.dvPower;
	// Extra parameters
	tmpMaterial.hTexture = 0;
	tmpMaterial.dwRampSize = 0;
	// Copy to variable
	CopyMemory(&Material, &tmpMaterial, tmpMaterial.dwSize);
}

void ConvertMaterial(D3DMATERIAL7 &Material7, D3DMATERIAL &Material)
{
	D3DMATERIAL tmpMaterial = { NULL };
	CopyMemory(&tmpMaterial, &Material, min(sizeof(D3DMATERIAL), Material.dwSize));
	Material7.dcvDiffuse = tmpMaterial.dcvDiffuse;
	Material7.dcvAmbient = tmpMaterial.dcvAmbient;
	Material7.dcvSpecular = tmpMaterial.dcvSpecular;
	Material7.dcvEmissive = tmpMaterial.dcvEmissive;
	Material7.dvPower = tmpMaterial.dvPower;
}

void ConvertViewport(D3DVIEWPORT &ViewPort, D3DVIEWPORT &ViewPort2)
{
	DWORD dwSize = min(sizeof(D3DVIEWPORT), ViewPort.dwSize);
	ZeroMemory(&ViewPort, dwSize);
	memcpy(&ViewPort, &ViewPort2, min(dwSize, ViewPort2.dwSize));
	ViewPort.dwSize = dwSize;
}

void ConvertViewport(D3DVIEWPORT2 &ViewPort, D3DVIEWPORT2 &ViewPort2)
{
	DWORD dwSize = min(sizeof(D3DVIEWPORT2), ViewPort.dwSize);
	ZeroMemory(&ViewPort, dwSize);
	memcpy(&ViewPort, &ViewPort2, min(dwSize, ViewPort2.dwSize));
	ViewPort.dwSize = dwSize;
}

void ConvertViewport(D3DVIEWPORT &ViewPort, D3DVIEWPORT2 &ViewPort2)
{
	// Prepare varables
	D3DVIEWPORT tmpViewPort;
	D3DVIEWPORT2 tmpViewPort2 = { NULL };
	CopyMemory(&tmpViewPort2, &ViewPort2, min(ViewPort2.dwSize, sizeof(D3DVIEWPORT2)));
	// Convert varables
	tmpViewPort.dwSize = min(sizeof(D3DVIEWPORT), ViewPort.dwSize);
	tmpViewPort.dwX = tmpViewPort2.dwX;
	tmpViewPort.dwY = tmpViewPort2.dwY;
	tmpViewPort.dwWidth = tmpViewPort2.dwWidth;
	tmpViewPort.dwHeight = tmpViewPort2.dwHeight;
	tmpViewPort.dvMinZ = tmpViewPort2.dvMinZ;
	tmpViewPort.dvMaxZ = tmpViewPort2.dvMaxZ;
	// Extra parameters
	tmpViewPort.dvScaleX = 0;        /* Scale homogeneous to screen */
	tmpViewPort.dvScaleY = 0;        /* Scale homogeneous to screen */
	tmpViewPort.dvMaxX = 0;          /* Min/max homogeneous x coord */
	tmpViewPort.dvMaxY = 0;          /* Min/max homogeneous y coord */
	// Copy to variable
	CopyMemory(&ViewPort, &tmpViewPort, tmpViewPort.dwSize);
}

void ConvertViewport(D3DVIEWPORT2 &ViewPort2, D3DVIEWPORT &ViewPort)
{
	// Prepare varables
	D3DVIEWPORT2 tmpViewPort2;
	D3DVIEWPORT tmpViewPort = { NULL };
	CopyMemory(&tmpViewPort, &ViewPort, min(ViewPort.dwSize, sizeof(D3DVIEWPORT)));
	// Convert varables
	tmpViewPort2.dwSize = min(sizeof(D3DVIEWPORT2), ViewPort2.dwSize);
	tmpViewPort2.dwX = tmpViewPort.dwX;
	tmpViewPort2.dwY = tmpViewPort.dwY;
	tmpViewPort2.dwWidth = tmpViewPort.dwWidth;
	tmpViewPort2.dwHeight = tmpViewPort.dwHeight;
	tmpViewPort2.dvMinZ = tmpViewPort.dvMinZ;
	tmpViewPort2.dvMaxZ = tmpViewPort.dvMaxZ;
	// Extra parameters
	tmpViewPort2.dvClipX = 0;        /* Top left of clip volume */
	tmpViewPort2.dvClipY = 0;
	tmpViewPort2.dvClipWidth = 0;    /* Clip Volume Dimensions */
	tmpViewPort2.dvClipHeight = 0;
	// Copy to variable
	CopyMemory(&ViewPort2, &tmpViewPort2, tmpViewPort2.dwSize);
}

void ConvertViewport(D3DVIEWPORT &ViewPort, D3DVIEWPORT7 &ViewPort7)
{
	D3DVIEWPORT tmpViewPort;
	tmpViewPort.dwSize = min(sizeof(D3DVIEWPORT), ViewPort.dwSize);
	tmpViewPort.dwX = ViewPort7.dwX;
	tmpViewPort.dwY = ViewPort7.dwY;
	tmpViewPort.dwWidth = ViewPort7.dwWidth;
	tmpViewPort.dwHeight = ViewPort7.dwHeight;
	tmpViewPort.dvMinZ = ViewPort7.dvMinZ;
	tmpViewPort.dvMaxZ = ViewPort7.dvMaxZ;
	// Extra parameters
	tmpViewPort.dvScaleX = 0;        /* Scale homogeneous to screen */
	tmpViewPort.dvScaleY = 0;        /* Scale homogeneous to screen */
	tmpViewPort.dvMaxX = 0;          /* Min/max homogeneous x coord */
	tmpViewPort.dvMaxY = 0;          /* Min/max homogeneous y coord */
	// Copy to variable
	CopyMemory(&ViewPort, &tmpViewPort, tmpViewPort.dwSize);
}

void ConvertViewport(D3DVIEWPORT2 &ViewPort2, D3DVIEWPORT7 &ViewPort7)
{
	D3DVIEWPORT2 tmpViewPort2;
	tmpViewPort2.dwSize = min(sizeof(D3DVIEWPORT2), ViewPort2.dwSize);
	tmpViewPort2.dwX = ViewPort7.dwX;
	tmpViewPort2.dwY = ViewPort7.dwY;
	tmpViewPort2.dwWidth = ViewPort7.dwWidth;
	tmpViewPort2.dwHeight = ViewPort7.dwHeight;
	tmpViewPort2.dvMinZ = ViewPort7.dvMinZ;
	tmpViewPort2.dvMaxZ = ViewPort7.dvMaxZ;
	// Extra parameters
	tmpViewPort2.dvClipX = 0;        /* Top left of clip volume */
	tmpViewPort2.dvClipY = 0;
	tmpViewPort2.dvClipWidth = 0;    /* Clip Volume Dimensions */
	tmpViewPort2.dvClipHeight = 0;
	// Copy to variable
	CopyMemory(&ViewPort2, &tmpViewPort2, tmpViewPort2.dwSize);
}

void ConvertViewport(D3DVIEWPORT7 &ViewPort7, D3DVIEWPORT &ViewPort)
{
	D3DVIEWPORT tmpViewPort = { NULL };
	CopyMemory(&tmpViewPort, &ViewPort, min(sizeof(D3DVIEWPORT), ViewPort.dwSize));
	ViewPort7.dwX = tmpViewPort.dwX;
	ViewPort7.dwY = tmpViewPort.dwY;
	ViewPort7.dwWidth = tmpViewPort.dwWidth;
	ViewPort7.dwHeight = tmpViewPort.dwHeight;
	ViewPort7.dvMinZ = tmpViewPort.dvMinZ;
	ViewPort7.dvMaxZ = tmpViewPort.dvMaxZ;
}

void ConvertViewport(D3DVIEWPORT7 &ViewPort7, D3DVIEWPORT2 &ViewPort2)
{
	D3DVIEWPORT2 tmpViewPort2 = { NULL };
	CopyMemory(&tmpViewPort2, &ViewPort2, min(sizeof(D3DVIEWPORT2), ViewPort2.dwSize));
	ViewPort7.dwX = tmpViewPort2.dwX;
	ViewPort7.dwY = tmpViewPort2.dwY;
	ViewPort7.dwWidth = tmpViewPort2.dwWidth;
	ViewPort7.dwHeight = tmpViewPort2.dwHeight;
	ViewPort7.dvMinZ = tmpViewPort2.dvMinZ;
	ViewPort7.dvMaxZ = tmpViewPort2.dvMaxZ;
}

void ConvertViewport(D3DVIEWPORT7 &ViewPort, D3DVIEWPORT7 &ViewPort7)
{
	ViewPort.dwX = ViewPort7.dwX;
	ViewPort.dwY = ViewPort7.dwY;
	ViewPort.dwWidth = ViewPort7.dwWidth;
	ViewPort.dwHeight = ViewPort7.dwHeight;
	ViewPort.dvMinZ = ViewPort7.dvMinZ;
	ViewPort.dvMaxZ = ViewPort7.dvMaxZ;
}

void ConvertDeviceDesc(D3DDEVICEDESC &Desc, D3DDEVICEDESC7 &Desc7)
{
	// Prepare varables
	D3DDEVICEDESC tmpDesc;
	// Convert varables
	tmpDesc.dwSize = min(sizeof(D3DDEVICEDESC), Desc.dwSize);
	tmpDesc.dwDevCaps = Desc7.dwDevCaps;
	CopyMemory(&tmpDesc.dpcLineCaps, &Desc7.dpcLineCaps, sizeof(D3DPRIMCAPS));
	CopyMemory(&tmpDesc.dpcTriCaps, &Desc7.dpcTriCaps, sizeof(D3DPRIMCAPS));
	tmpDesc.dwDeviceRenderBitDepth = Desc7.dwDeviceRenderBitDepth;
	tmpDesc.dwDeviceZBufferBitDepth = Desc7.dwDeviceZBufferBitDepth;
	tmpDesc.dwMaxTextureRepeat = Desc7.dwMaxTextureRepeat;
	tmpDesc.dwFlags = (D3DDD_DEVCAPS | D3DDD_LINECAPS | D3DDD_TRICAPS | D3DDD_DEVICERENDERBITDEPTH | D3DDD_DEVICEZBUFFERBITDEPTH);
	// Extra parameters
	tmpDesc.dcmColorModel = 0;                 /* Color model of device */
	tmpDesc.dtcTransformCaps.dwSize = sizeof(D3DTRANSFORMCAPS);
	tmpDesc.dtcTransformCaps.dwCaps = 0;       /* Capabilities of transform */
	tmpDesc.bClipping = 0;                     /* Device can do 3D clipping */
	tmpDesc.dlcLightingCaps.dwSize = sizeof(D3DLIGHTINGCAPS);
	tmpDesc.dlcLightingCaps.dwCaps = 0;        /* Capabilities of lighting */
	tmpDesc.dlcLightingCaps.dwLightingModel = 0;
	tmpDesc.dlcLightingCaps.dwNumLights = 0;
	tmpDesc.dwMaxBufferSize = 0;               /* Maximum execute buffer size */
	tmpDesc.dwMaxVertexCount = 0;              /* Maximum vertex count */
	/* DIRECT3D_VERSION >= 0x0500 */
	tmpDesc.dwMinTextureWidth = Desc7.dwMinTextureWidth;
	tmpDesc.dwMinTextureHeight = Desc7.dwMinTextureHeight;
	tmpDesc.dwMaxTextureWidth = Desc7.dwMaxTextureWidth;
	tmpDesc.dwMaxTextureHeight = Desc7.dwMaxTextureHeight;
	// Extra parameters
	tmpDesc.dwMinStippleWidth = 0;
	tmpDesc.dwMaxStippleWidth = 0;
	tmpDesc.dwMinStippleHeight = 0;
	tmpDesc.dwMaxStippleHeight = 0;
	/* DIRECT3D_VERSION >= 0x0600 */
	tmpDesc.dwMaxTextureAspectRatio = Desc7.dwMaxTextureAspectRatio;
	tmpDesc.dwMaxAnisotropy = Desc7.dwMaxAnisotropy;
	tmpDesc.dvGuardBandLeft = Desc7.dvGuardBandLeft;
	tmpDesc.dvGuardBandTop = Desc7.dvGuardBandTop;
	tmpDesc.dvGuardBandRight = Desc7.dvGuardBandRight;
	tmpDesc.dvGuardBandBottom = Desc7.dvGuardBandBottom;
	tmpDesc.dvExtentsAdjust = Desc7.dvExtentsAdjust;
	tmpDesc.dwStencilCaps = Desc7.dwStencilCaps;
	tmpDesc.dwFVFCaps = Desc7.dwFVFCaps;
	tmpDesc.dwTextureOpCaps = Desc7.dwTextureOpCaps;
	tmpDesc.wMaxTextureBlendStages = Desc7.wMaxTextureBlendStages;
	tmpDesc.wMaxSimultaneousTextures = Desc7.wMaxSimultaneousTextures;
	// Copy to variable
	CopyMemory(&Desc, &tmpDesc, tmpDesc.dwSize);
}

void ConvertDeviceDesc(D3DDEVICEDESC7 &Desc7, D3DCAPS9 &Caps9)
{
	// General settings
	Desc7.dwMinTextureWidth = (Caps9.MaxTextureWidth) ? 1 : 0;
	Desc7.dwMinTextureHeight = (Caps9.MaxTextureHeight) ? 1 : 0;
	Desc7.dwMaxTextureWidth = Caps9.MaxTextureWidth;
	Desc7.dwMaxTextureHeight = Caps9.MaxTextureHeight;
	Desc7.dwMaxTextureRepeat = Caps9.MaxTextureRepeat;
	Desc7.dwMaxTextureAspectRatio = Caps9.MaxTextureAspectRatio;
	Desc7.dwMaxAnisotropy = Caps9.MaxAnisotropy;
	Desc7.dvGuardBandLeft = Caps9.GuardBandLeft;
	Desc7.dvGuardBandTop = Caps9.GuardBandTop;
	Desc7.dvGuardBandRight = Caps9.GuardBandRight;
	Desc7.dvGuardBandBottom = Caps9.GuardBandBottom;
	Desc7.dvExtentsAdjust = Caps9.ExtentsAdjust;
	Desc7.wMaxTextureBlendStages = (WORD)((Caps9.MaxTextureBlendStages > 0xFFFF) ? 0xFFFF : Caps9.MaxTextureBlendStages);
	Desc7.wMaxSimultaneousTextures = (WORD)((Caps9.MaxSimultaneousTextures > 0xFFFF) ? 0xFFFF : Caps9.MaxSimultaneousTextures);
	Desc7.dwMaxActiveLights = Caps9.MaxActiveLights;
	Desc7.dvMaxVertexW = Caps9.MaxVertexW;
	Desc7.wMaxUserClipPlanes = (WORD)((Caps9.MaxUserClipPlanes > 0xFFFF) ? 0xFFFF : Caps9.MaxUserClipPlanes);
	Desc7.wMaxVertexBlendMatrices = (WORD)((Caps9.MaxVertexBlendMatrices > 0xFFFF) ? 0xFFFF : Caps9.MaxVertexBlendMatrices);

	// Caps
	Desc7.dwDevCaps = (Caps9.DevCaps & 0x2FFFF);
	Desc7.dwStencilCaps = (Caps9.StencilCaps & 0xFF);
	Desc7.dwFVFCaps = (Caps9.FVFCaps & 0x8FFFF);
	Desc7.dwTextureOpCaps = (Caps9.TextureOpCaps & 0xFFFFFF);
	Desc7.dwVertexProcessingCaps = (Caps9.VertexProcessingCaps & 0x3F);

	// Line Caps
	Desc7.dpcLineCaps.dwSize = sizeof(D3DPRIMCAPS);
	Desc7.dpcLineCaps.dwMiscCaps = (Caps9.PrimitiveMiscCaps & 0x8F);
	Desc7.dpcLineCaps.dwRasterCaps = (Caps9.RasterCaps & 0x3FFFFF);
	Desc7.dpcLineCaps.dwZCmpCaps = (Caps9.ZCmpCaps & 0xFF);
	Desc7.dpcLineCaps.dwSrcBlendCaps = (Caps9.SrcBlendCaps & 0x1FFF);
	Desc7.dpcLineCaps.dwDestBlendCaps = (Caps9.DestBlendCaps & 0x1FFF);
	Desc7.dpcLineCaps.dwAlphaCmpCaps = (Caps9.AlphaCmpCaps & 0xFF);
	Desc7.dpcLineCaps.dwShadeCaps = (Caps9.ShadeCaps & 0x1FFFFF);
	Desc7.dpcLineCaps.dwTextureCaps = (Caps9.TextureCaps & 0x1FFF);
	Desc7.dpcLineCaps.dwTextureFilterCaps = (Caps9.TextureFilterCaps & 0x1F373F);
	Desc7.dpcLineCaps.dwTextureBlendCaps = (D3DPTBLENDCAPS_DECAL | D3DPTBLENDCAPS_MODULATE | D3DPTBLENDCAPS_DECALALPHA | D3DPTBLENDCAPS_MODULATEALPHA | D3DPTBLENDCAPS_COPY | D3DPTBLENDCAPS_ADD);
	Desc7.dpcLineCaps.dwTextureAddressCaps = (Caps9.TextureAddressCaps & 0x1F);

	// Tri Caps same as Line Caps
	memcpy(&Desc7.dpcTriCaps, &Desc7.dpcLineCaps, sizeof(D3DPRIMCAPS));

	// Specific settings
	if (Caps9.DeviceType == D3DDEVTYPE_REF)
	{
		Desc7.deviceGUID = IID_IDirect3DRGBDevice;
		Desc7.dpcLineCaps.dwStippleWidth = 4;
		Desc7.dpcLineCaps.dwStippleHeight = 4;
		Desc7.dwDeviceRenderBitDepth = 3840;
		Desc7.dwDeviceZBufferBitDepth = 1024;
	}
	else if (Caps9.DeviceType == D3DDEVTYPE_HAL)
	{
		Desc7.deviceGUID = IID_IDirect3DHALDevice;
		Desc7.dwDevCaps |= D3DDEVCAPS_HWRASTERIZATION;
		Desc7.dpcLineCaps.dwStippleWidth = 0;
		Desc7.dpcLineCaps.dwStippleHeight = 0;
		Desc7.dwDeviceRenderBitDepth = 3328;
		Desc7.dwDeviceZBufferBitDepth = 1536;
	}
	else if (Caps9.DeviceType == D3DDEVTYPE_HAL + 0x10)
	{
		Desc7.deviceGUID = IID_IDirect3DTnLHalDevice;
		Desc7.dwDevCaps |= D3DDEVCAPS_HWRASTERIZATION | D3DDEVCAPS_HWTRANSFORMANDLIGHT;
		Desc7.dpcLineCaps.dwStippleWidth = 0;
		Desc7.dpcLineCaps.dwStippleHeight = 0;
		Desc7.dwDeviceRenderBitDepth = 3328;
		Desc7.dwDeviceZBufferBitDepth = 1536;
	}

	// Reserved
	Desc7.dwReserved1 = 0;
	Desc7.dwReserved2 = 0;
	Desc7.dwReserved3 = 0;
	Desc7.dwReserved4 = 0;
}

void GetBufferFormat(DDPIXELFORMAT &ddpfPixelFormat, DWORD Num)
{
	if (ddpfPixelFormat.dwSize != sizeof(DDPIXELFORMAT))
	{
		Logging::Log() << __FUNCTION__ << " Error with DDPIXELFORMAT dwSize! " << ddpfPixelFormat.dwSize;
		return;
	}

	switch (Num)
	{
	case 0:
		ddpfPixelFormat.dwFlags = 1024;
		ddpfPixelFormat.dwFourCC = 0;
		ddpfPixelFormat.dwRGBBitCount = 16;
		ddpfPixelFormat.dwYUVBitCount = 16;
		ddpfPixelFormat.dwZBufferBitDepth = 16;
		ddpfPixelFormat.dwAlphaBitDepth = 16;
		ddpfPixelFormat.dwLuminanceBitCount = 16;
		ddpfPixelFormat.dwBumpBitCount = 16;
		ddpfPixelFormat.dwPrivateFormatBitCount = 16;
		ddpfPixelFormat.dwRBitMask = 0;
		ddpfPixelFormat.dwYBitMask = 0;
		ddpfPixelFormat.dwStencilBitDepth = 0;
		ddpfPixelFormat.dwLuminanceBitMask = 0;
		ddpfPixelFormat.dwBumpDuBitMask = 0;
		ddpfPixelFormat.dwOperations = 0;
		ddpfPixelFormat.dwGBitMask = 65535;
		ddpfPixelFormat.dwUBitMask = 65535;
		ddpfPixelFormat.dwZBitMask = 65535;
		ddpfPixelFormat.dwBumpDvBitMask = 65535;
		ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 65535;
		ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
		ddpfPixelFormat.dwBBitMask = 0;
		ddpfPixelFormat.dwVBitMask = 0;
		ddpfPixelFormat.dwStencilBitMask = 0;
		ddpfPixelFormat.dwBumpLuminanceBitMask = 0;
		ddpfPixelFormat.dwRGBAlphaBitMask = 0;
		ddpfPixelFormat.dwYUVAlphaBitMask = 0;
		ddpfPixelFormat.dwLuminanceAlphaBitMask = 0;
		ddpfPixelFormat.dwRGBZBitMask = 0;
		ddpfPixelFormat.dwYUVZBitMask = 0;
		break;
	case 1:
		ddpfPixelFormat.dwFlags = 1024;
		ddpfPixelFormat.dwFourCC = 0;
		ddpfPixelFormat.dwRGBBitCount = 32;
		ddpfPixelFormat.dwYUVBitCount = 32;
		ddpfPixelFormat.dwZBufferBitDepth = 32;
		ddpfPixelFormat.dwAlphaBitDepth = 32;
		ddpfPixelFormat.dwLuminanceBitCount = 32;
		ddpfPixelFormat.dwBumpBitCount = 32;
		ddpfPixelFormat.dwPrivateFormatBitCount = 32;
		ddpfPixelFormat.dwRBitMask = 0;
		ddpfPixelFormat.dwYBitMask = 0;
		ddpfPixelFormat.dwStencilBitDepth = 0;
		ddpfPixelFormat.dwLuminanceBitMask = 0;
		ddpfPixelFormat.dwBumpDuBitMask = 0;
		ddpfPixelFormat.dwOperations = 0;
		ddpfPixelFormat.dwGBitMask = 4294967040;
		ddpfPixelFormat.dwUBitMask = 4294967040;
		ddpfPixelFormat.dwZBitMask = 4294967040;
		ddpfPixelFormat.dwBumpDvBitMask = 4294967040;
		ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 65280;
		ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 65535;
		ddpfPixelFormat.dwBBitMask = 0;
		ddpfPixelFormat.dwVBitMask = 0;
		ddpfPixelFormat.dwStencilBitMask = 0;
		ddpfPixelFormat.dwBumpLuminanceBitMask = 0;
		ddpfPixelFormat.dwRGBAlphaBitMask = 0;
		ddpfPixelFormat.dwYUVAlphaBitMask = 0;
		ddpfPixelFormat.dwLuminanceAlphaBitMask = 0;
		ddpfPixelFormat.dwRGBZBitMask = 0;
		ddpfPixelFormat.dwYUVZBitMask = 0;
		break;
	case 2:
		ddpfPixelFormat.dwFlags = 17408;
		ddpfPixelFormat.dwFourCC = 0;
		ddpfPixelFormat.dwRGBBitCount = 32;
		ddpfPixelFormat.dwYUVBitCount = 32;
		ddpfPixelFormat.dwZBufferBitDepth = 32;
		ddpfPixelFormat.dwAlphaBitDepth = 32;
		ddpfPixelFormat.dwLuminanceBitCount = 32;
		ddpfPixelFormat.dwBumpBitCount = 32;
		ddpfPixelFormat.dwPrivateFormatBitCount = 32;
		ddpfPixelFormat.dwRBitMask = 8;
		ddpfPixelFormat.dwYBitMask = 8;
		ddpfPixelFormat.dwStencilBitDepth = 8;
		ddpfPixelFormat.dwLuminanceBitMask = 8;
		ddpfPixelFormat.dwBumpDuBitMask = 8;
		ddpfPixelFormat.dwOperations = 8;
		ddpfPixelFormat.dwGBitMask = 4294967040;
		ddpfPixelFormat.dwUBitMask = 4294967040;
		ddpfPixelFormat.dwZBitMask = 4294967040;
		ddpfPixelFormat.dwBumpDvBitMask = 4294967040;
		ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 65280;
		ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 65535;
		ddpfPixelFormat.dwBBitMask = 255;
		ddpfPixelFormat.dwVBitMask = 255;
		ddpfPixelFormat.dwStencilBitMask = 255;
		ddpfPixelFormat.dwBumpLuminanceBitMask = 255;
		ddpfPixelFormat.dwRGBAlphaBitMask = 0;
		ddpfPixelFormat.dwYUVAlphaBitMask = 0;
		ddpfPixelFormat.dwLuminanceAlphaBitMask = 0;
		ddpfPixelFormat.dwRGBZBitMask = 0;
		ddpfPixelFormat.dwYUVZBitMask = 0;
		break;
	case 3:
		ddpfPixelFormat.dwFlags = 17408;
		ddpfPixelFormat.dwFourCC = 0;
		ddpfPixelFormat.dwRGBBitCount = 32;
		ddpfPixelFormat.dwYUVBitCount = 32;
		ddpfPixelFormat.dwZBufferBitDepth = 32;
		ddpfPixelFormat.dwAlphaBitDepth = 32;
		ddpfPixelFormat.dwLuminanceBitCount = 32;
		ddpfPixelFormat.dwBumpBitCount = 32;
		ddpfPixelFormat.dwPrivateFormatBitCount = 32;
		ddpfPixelFormat.dwRBitMask = 8;
		ddpfPixelFormat.dwYBitMask = 8;
		ddpfPixelFormat.dwStencilBitDepth = 8;
		ddpfPixelFormat.dwLuminanceBitMask = 8;
		ddpfPixelFormat.dwBumpDuBitMask = 8;
		ddpfPixelFormat.dwOperations = 8;
		ddpfPixelFormat.dwGBitMask = 16777215;
		ddpfPixelFormat.dwUBitMask = 16777215;
		ddpfPixelFormat.dwZBitMask = 16777215;
		ddpfPixelFormat.dwBumpDvBitMask = 16777215;
		ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 65535;
		ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 255;
		ddpfPixelFormat.dwBBitMask = 4278190080;
		ddpfPixelFormat.dwVBitMask = 4278190080;
		ddpfPixelFormat.dwStencilBitMask = 4278190080;
		ddpfPixelFormat.dwBumpLuminanceBitMask = 4278190080;
		ddpfPixelFormat.dwRGBAlphaBitMask = 0;
		ddpfPixelFormat.dwYUVAlphaBitMask = 0;
		ddpfPixelFormat.dwLuminanceAlphaBitMask = 0;
		ddpfPixelFormat.dwRGBZBitMask = 0;
		ddpfPixelFormat.dwYUVZBitMask = 0;
		break;
	case 4:
		ddpfPixelFormat.dwFlags = 1024;
		ddpfPixelFormat.dwFourCC = 0;
		ddpfPixelFormat.dwRGBBitCount = 32;
		ddpfPixelFormat.dwYUVBitCount = 32;
		ddpfPixelFormat.dwZBufferBitDepth = 32;
		ddpfPixelFormat.dwAlphaBitDepth = 32;
		ddpfPixelFormat.dwLuminanceBitCount = 32;
		ddpfPixelFormat.dwBumpBitCount = 32;
		ddpfPixelFormat.dwPrivateFormatBitCount = 32;
		ddpfPixelFormat.dwRBitMask = 0;
		ddpfPixelFormat.dwYBitMask = 0;
		ddpfPixelFormat.dwStencilBitDepth = 0;
		ddpfPixelFormat.dwLuminanceBitMask = 0;
		ddpfPixelFormat.dwBumpDuBitMask = 0;
		ddpfPixelFormat.dwOperations = 0;
		ddpfPixelFormat.dwGBitMask = 16777215;
		ddpfPixelFormat.dwUBitMask = 16777215;
		ddpfPixelFormat.dwZBitMask = 16777215;
		ddpfPixelFormat.dwBumpDvBitMask = 16777215;
		ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 65535;
		ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 255;
		ddpfPixelFormat.dwBBitMask = 0;
		ddpfPixelFormat.dwVBitMask = 0;
		ddpfPixelFormat.dwStencilBitMask = 0;
		ddpfPixelFormat.dwBumpLuminanceBitMask = 0;
		ddpfPixelFormat.dwRGBAlphaBitMask = 0;
		ddpfPixelFormat.dwYUVAlphaBitMask = 0;
		ddpfPixelFormat.dwLuminanceAlphaBitMask = 0;
		ddpfPixelFormat.dwRGBZBitMask = 0;
		ddpfPixelFormat.dwYUVZBitMask = 0;
		break;
	case 5:
		ddpfPixelFormat.dwFlags = 1024;
		ddpfPixelFormat.dwFourCC = 0;
		ddpfPixelFormat.dwRGBBitCount = 24;
		ddpfPixelFormat.dwYUVBitCount = 24;
		ddpfPixelFormat.dwZBufferBitDepth = 24;
		ddpfPixelFormat.dwAlphaBitDepth = 24;
		ddpfPixelFormat.dwLuminanceBitCount = 24;
		ddpfPixelFormat.dwBumpBitCount = 24;
		ddpfPixelFormat.dwPrivateFormatBitCount = 24;
		ddpfPixelFormat.dwRBitMask = 0;
		ddpfPixelFormat.dwYBitMask = 0;
		ddpfPixelFormat.dwStencilBitDepth = 0;
		ddpfPixelFormat.dwLuminanceBitMask = 0;
		ddpfPixelFormat.dwBumpDuBitMask = 0;
		ddpfPixelFormat.dwOperations = 0;
		ddpfPixelFormat.dwGBitMask = 16777215;
		ddpfPixelFormat.dwUBitMask = 16777215;
		ddpfPixelFormat.dwZBitMask = 16777215;
		ddpfPixelFormat.dwBumpDvBitMask = 16777215;
		ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 65535;
		ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 255;
		ddpfPixelFormat.dwBBitMask = 0;
		ddpfPixelFormat.dwVBitMask = 0;
		ddpfPixelFormat.dwStencilBitMask = 0;
		ddpfPixelFormat.dwBumpLuminanceBitMask = 0;
		ddpfPixelFormat.dwRGBAlphaBitMask = 0;
		ddpfPixelFormat.dwYUVAlphaBitMask = 0;
		ddpfPixelFormat.dwLuminanceAlphaBitMask = 0;
		ddpfPixelFormat.dwRGBZBitMask = 0;
		ddpfPixelFormat.dwYUVZBitMask = 0;
		break;
	}
}
