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

#define D3DLIGHTCAPS_PARALLELPOINT      0x00000008L /* Parallel point lights supported */
#define D3DLIGHTCAPS_GLSPOT             0x00000010L /* GL syle spot lights supported */

void ConvertMaterial(D3DMATERIAL &Material, D3DMATERIAL &Material2)
{
	if (Material.dwSize != sizeof(D3DMATERIAL) || Material2.dwSize != sizeof(D3DMATERIAL))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error! Incorrect dwSize: " << Material.dwSize << " " << Material2.dwSize);
		return;
	}
	CopyMemory(&Material, &Material2, sizeof(D3DMATERIAL));
}

void ConvertMaterial(D3DMATERIAL7 &Material, D3DMATERIAL7 &Material2)
{
	CopyMemory(&Material, &Material2, sizeof(D3DMATERIAL7));
}

void ConvertMaterial(D3DMATERIAL &Material, D3DMATERIAL7 &Material7)
{
	if (Material.dwSize != sizeof(D3DMATERIAL))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error! Incorrect dwSize: " << Material.dwSize);
		return;
	}
	Material.dcvDiffuse = Material7.dcvDiffuse;
	Material.dcvAmbient = Material7.dcvAmbient;
	Material.dcvSpecular = Material7.dcvSpecular;
	Material.dcvEmissive = Material7.dcvEmissive;
	Material.dvPower = Material7.dvPower;
	// Extra parameters
	Material.hTexture = 0;
	Material.dwRampSize = 0;
}

void ConvertMaterial(D3DMATERIAL7 &Material7, D3DMATERIAL &Material)
{
	if (Material.dwSize != sizeof(D3DMATERIAL))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error! Incorrect dwSize: " << Material.dwSize);
		return;
	}
	Material7.dcvDiffuse = Material.dcvDiffuse;
	Material7.dcvAmbient = Material.dcvAmbient;
	Material7.dcvSpecular = Material.dcvSpecular;
	Material7.dcvEmissive = Material.dcvEmissive;
	Material7.dvPower = Material.dvPower;
}

void ConvertViewport(D3DVIEWPORT &ViewPort, D3DVIEWPORT &ViewPort2)
{
	if (ViewPort.dwSize != sizeof(D3DVIEWPORT) || ViewPort2.dwSize != sizeof(D3DVIEWPORT))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error! Incorrect dwSize: " << ViewPort.dwSize << " " << ViewPort2.dwSize);
		return;
	}
	memcpy(&ViewPort, &ViewPort2, sizeof(D3DVIEWPORT));
}

void ConvertViewport(D3DVIEWPORT2 &ViewPort, D3DVIEWPORT2 &ViewPort2)
{
	if (ViewPort.dwSize != sizeof(D3DVIEWPORT2) || ViewPort2.dwSize != sizeof(D3DVIEWPORT2))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error! Incorrect dwSize: " << ViewPort.dwSize << " " << ViewPort2.dwSize);
		return;
	}
	memcpy(&ViewPort, &ViewPort2, sizeof(D3DVIEWPORT2));
}

void ConvertViewport(D3DVIEWPORT &ViewPort, D3DVIEWPORT2 &ViewPort2)
{
	if (ViewPort.dwSize != sizeof(D3DVIEWPORT) || ViewPort2.dwSize != sizeof(D3DVIEWPORT2))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error! Incorrect dwSize: " << ViewPort.dwSize << " " << ViewPort2.dwSize);
		return;
	}
	// Convert varables
	ViewPort.dwX = ViewPort2.dwX;
	ViewPort.dwY = ViewPort2.dwY;
	ViewPort.dwWidth = ViewPort2.dwWidth;
	ViewPort.dwHeight = ViewPort2.dwHeight;
	ViewPort.dvMinZ = ViewPort2.dvMinZ;
	ViewPort.dvMaxZ = ViewPort2.dvMaxZ;
	// Extra parameters
	ViewPort.dvScaleX = 0;        /* Scale homogeneous to screen */
	ViewPort.dvScaleY = 0;        /* Scale homogeneous to screen */
	ViewPort.dvMaxX = 0;          /* Min/max homogeneous x coord */
	ViewPort.dvMaxY = 0;          /* Min/max homogeneous y coord */
}

void ConvertViewport(D3DVIEWPORT2 &ViewPort2, D3DVIEWPORT &ViewPort)
{
	if (ViewPort2.dwSize != sizeof(D3DVIEWPORT2) || ViewPort.dwSize != sizeof(D3DVIEWPORT))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error! Incorrect dwSize: " << ViewPort2.dwSize << " " << ViewPort.dwSize);
		return;
	}
	// Convert varables
	ViewPort2.dwX = ViewPort.dwX;
	ViewPort2.dwY = ViewPort.dwY;
	ViewPort2.dwWidth = ViewPort.dwWidth;
	ViewPort2.dwHeight = ViewPort.dwHeight;
	ViewPort2.dvMinZ = ViewPort.dvMinZ;
	ViewPort2.dvMaxZ = ViewPort.dvMaxZ;
	// Extra parameters
	ViewPort2.dvClipX = 0;        /* Top left of clip volume */
	ViewPort2.dvClipY = 0;
	ViewPort2.dvClipWidth = 0;    /* Clip Volume Dimensions */
	ViewPort2.dvClipHeight = 0;
}

void ConvertViewport(D3DVIEWPORT &ViewPort, D3DVIEWPORT7 &ViewPort7)
{
	if (ViewPort.dwSize != sizeof(D3DVIEWPORT))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error! Incorrect dwSize: " << ViewPort.dwSize);
		return;
	}
	// Convert varables
	ViewPort.dwX = ViewPort7.dwX;
	ViewPort.dwY = ViewPort7.dwY;
	ViewPort.dwWidth = ViewPort7.dwWidth;
	ViewPort.dwHeight = ViewPort7.dwHeight;
	ViewPort.dvMinZ = ViewPort7.dvMinZ;
	ViewPort.dvMaxZ = ViewPort7.dvMaxZ;
	// Extra parameters
	ViewPort.dvScaleX = 0;        /* Scale homogeneous to screen */
	ViewPort.dvScaleY = 0;        /* Scale homogeneous to screen */
	ViewPort.dvMaxX = 0;          /* Min/max homogeneous x coord */
	ViewPort.dvMaxY = 0;          /* Min/max homogeneous y coord */
}

void ConvertViewport(D3DVIEWPORT2 &ViewPort2, D3DVIEWPORT7 &ViewPort7)
{
	if (ViewPort2.dwSize != sizeof(D3DVIEWPORT2))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error! Incorrect dwSize: " << ViewPort2.dwSize);
		return;
	}
	// Convert varables
	ViewPort2.dwX = ViewPort7.dwX;
	ViewPort2.dwY = ViewPort7.dwY;
	ViewPort2.dwWidth = ViewPort7.dwWidth;
	ViewPort2.dwHeight = ViewPort7.dwHeight;
	ViewPort2.dvMinZ = ViewPort7.dvMinZ;
	ViewPort2.dvMaxZ = ViewPort7.dvMaxZ;
	// Extra parameters
	ViewPort2.dvClipX = 0;        /* Top left of clip volume */
	ViewPort2.dvClipY = 0;
	ViewPort2.dvClipWidth = 0;    /* Clip Volume Dimensions */
	ViewPort2.dvClipHeight = 0;
}

void ConvertViewport(D3DVIEWPORT7 &ViewPort7, D3DVIEWPORT &ViewPort)
{
	if (ViewPort.dwSize != sizeof(D3DVIEWPORT))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error! Incorrect dwSize: " << ViewPort.dwSize);
		return;
	}
	// Convert varables
	ViewPort7.dwX = ViewPort.dwX;
	ViewPort7.dwY = ViewPort.dwY;
	ViewPort7.dwWidth = ViewPort.dwWidth;
	ViewPort7.dwHeight = ViewPort.dwHeight;
	ViewPort7.dvMinZ = ViewPort.dvMinZ;
	ViewPort7.dvMaxZ = ViewPort.dvMaxZ;
}

void ConvertViewport(D3DVIEWPORT7 &ViewPort7, D3DVIEWPORT2 &ViewPort2)
{
	if (ViewPort2.dwSize != sizeof(D3DVIEWPORT2))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error! Incorrect dwSize: " << ViewPort2.dwSize);
		return;
	}
	// Convert varables
	ViewPort7.dwX = ViewPort2.dwX;
	ViewPort7.dwY = ViewPort2.dwY;
	ViewPort7.dwWidth = ViewPort2.dwWidth;
	ViewPort7.dwHeight = ViewPort2.dwHeight;
	ViewPort7.dvMinZ = ViewPort2.dvMinZ;
	ViewPort7.dvMaxZ = ViewPort2.dvMaxZ;
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

void ConvertCaps(D3DPRIMCAPS &PrimCaps, D3DPRIMCAPS &PrimCaps2)
{
	if (PrimCaps.dwSize != sizeof(D3DPRIMCAPS) || PrimCaps2.dwSize != sizeof(D3DPRIMCAPS))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error! Incorrect dwSize: " << PrimCaps.dwSize << " " << PrimCaps2.dwSize);
		return;
	}
	// Convert varables
	memcpy(&PrimCaps, &PrimCaps2, sizeof(D3DPRIMCAPS));
}

void ConvertDeviceDesc(D3DDEVICEDESC &Desc, D3DDEVICEDESC7 &Desc7)
{
	if (Desc.dwSize != D3DDEVICEDESC1_SIZE && Desc.dwSize != D3DDEVICEDESC5_SIZE && Desc.dwSize != D3DDEVICEDESC6_SIZE)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error! Incorrect dwSize: " << Desc.dwSize);
		return;
	}
	// Convert varables
	Desc.dwFlags = D3DDD_COLORMODEL | D3DDD_DEVCAPS | D3DDD_LINECAPS | D3DDD_TRICAPS | D3DDD_DEVICERENDERBITDEPTH | D3DDD_DEVICEZBUFFERBITDEPTH;
	Desc.dcmColorModel = 2;
	Desc.dwDevCaps = Desc7.dwDevCaps | D3DDEVCAPS_HWTRANSFORMANDLIGHT;
	Desc.dtcTransformCaps.dwSize = sizeof(D3DTRANSFORMCAPS);
	Desc.dtcTransformCaps.dwCaps = D3DTRANSFORMCAPS_CLIP;
	Desc.bClipping = TRUE;
	Desc.dlcLightingCaps.dwSize = sizeof(D3DLIGHTINGCAPS);
	Desc.dlcLightingCaps.dwCaps = D3DLIGHTCAPS_POINT | D3DLIGHTCAPS_SPOT | D3DLIGHTCAPS_DIRECTIONAL;
	Desc.dlcLightingCaps.dwLightingModel = 1;
	Desc.dlcLightingCaps.dwNumLights = 8;
	Desc.dpcLineCaps.dwSize = sizeof(D3DPRIMCAPS);
	ConvertCaps(Desc.dpcLineCaps, Desc7.dpcLineCaps);
	Desc.dpcTriCaps.dwSize = sizeof(D3DPRIMCAPS);
	ConvertCaps(Desc.dpcTriCaps, Desc7.dpcTriCaps);
	Desc.dwDeviceRenderBitDepth = Desc7.dwDeviceRenderBitDepth;
	Desc.dwDeviceZBufferBitDepth = Desc7.dwDeviceZBufferBitDepth;
	Desc.dwMaxBufferSize = 0;
	Desc.dwMaxVertexCount = 65534;
	/* DIRECT3D_VERSION >= 0x0500 */
	if (Desc.dwSize < D3DDEVICEDESC5_SIZE)
	{
		return;
	}
	Desc.dwMinTextureWidth = Desc7.dwMinTextureWidth;
	Desc.dwMinTextureHeight = Desc7.dwMinTextureHeight;
	Desc.dwMaxTextureWidth = Desc7.dwMaxTextureWidth;
	Desc.dwMaxTextureHeight = Desc7.dwMaxTextureHeight;
	Desc.dwMinStippleWidth = (Desc7.dpcLineCaps.dwStippleWidth) ? 1 : 0;
	Desc.dwMaxStippleWidth = Desc7.dpcLineCaps.dwStippleWidth;
	Desc.dwMinStippleHeight = (Desc7.dpcLineCaps.dwStippleHeight) ? 1 : 0;
	Desc.dwMaxStippleHeight = Desc7.dpcLineCaps.dwStippleHeight;
	/* DIRECT3D_VERSION >= 0x0600 */
	if (Desc.dwSize < D3DDEVICEDESC6_SIZE)
	{
		return;
	}
	Desc.dwMaxTextureRepeat = Desc7.dwMaxTextureRepeat;
	Desc.dwMaxTextureAspectRatio = Desc7.dwMaxTextureAspectRatio;
	Desc.dwMaxAnisotropy = Desc7.dwMaxAnisotropy;
	Desc.dvGuardBandLeft = Desc7.dvGuardBandLeft;
	Desc.dvGuardBandTop = Desc7.dvGuardBandTop;
	Desc.dvGuardBandRight = Desc7.dvGuardBandRight;
	Desc.dvGuardBandBottom = Desc7.dvGuardBandBottom;
	Desc.dvExtentsAdjust = Desc7.dvExtentsAdjust;
	Desc.dwStencilCaps = Desc7.dwStencilCaps;
	Desc.dwFVFCaps = Desc7.dwFVFCaps;
	Desc.dwTextureOpCaps = Desc7.dwTextureOpCaps;
	Desc.wMaxTextureBlendStages = Desc7.wMaxTextureBlendStages;
	Desc.wMaxSimultaneousTextures = Desc7.wMaxSimultaneousTextures;
}

void ConvertDeviceDescSoft(D3DDEVICEDESC &Desc)
{
	if (Desc.dwSize != D3DDEVICEDESC1_SIZE && Desc.dwSize != D3DDEVICEDESC5_SIZE && Desc.dwSize != D3DDEVICEDESC6_SIZE)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error! Incorrect dwSize: " << Desc.dwSize);
		return;
	}
	// Convert varables
	Desc.dwFlags = D3DDD_COLORMODEL | D3DDD_DEVCAPS | D3DDD_TRANSFORMCAPS | D3DDD_LIGHTINGCAPS | D3DDD_BCLIPPING;
	Desc.dcmColorModel = 0;
	Desc.dwDevCaps = D3DDEVCAPS_FLOATTLVERTEX;
	Desc.dtcTransformCaps.dwSize = sizeof(D3DTRANSFORMCAPS);
	Desc.dtcTransformCaps.dwCaps = D3DTRANSFORMCAPS_CLIP;
	Desc.bClipping = TRUE;
	Desc.dlcLightingCaps.dwSize = sizeof(D3DLIGHTINGCAPS);
	Desc.dlcLightingCaps.dwCaps = D3DLIGHTCAPS_POINT | D3DLIGHTCAPS_SPOT | D3DLIGHTCAPS_DIRECTIONAL | D3DLIGHTCAPS_PARALLELPOINT;
	Desc.dlcLightingCaps.dwLightingModel = 1;
	Desc.dlcLightingCaps.dwNumLights = 0;
	ZeroMemory(&Desc.dpcLineCaps, sizeof(D3DPRIMCAPS));
	Desc.dpcLineCaps.dwSize = sizeof(D3DPRIMCAPS);
	ZeroMemory(&Desc.dpcTriCaps, sizeof(D3DPRIMCAPS));
	Desc.dpcTriCaps.dwSize = sizeof(D3DPRIMCAPS);
	Desc.dwDeviceRenderBitDepth = 0;
	Desc.dwDeviceZBufferBitDepth = 0;
	Desc.dwMaxBufferSize = 0;
	Desc.dwMaxVertexCount = 65534;
	/* DIRECT3D_VERSION >= 0x0500 */
	if (Desc.dwSize < D3DDEVICEDESC5_SIZE)
	{
		return;
	}
	Desc.dwMinTextureWidth = 0;
	Desc.dwMinTextureHeight = 0;
	Desc.dwMaxTextureWidth = 0;
	Desc.dwMaxTextureHeight = 0;
	Desc.dwMinStippleWidth = 0;
	Desc.dwMaxStippleWidth = 0;
	Desc.dwMinStippleHeight = 0;
	Desc.dwMaxStippleHeight = 0;
	/* DIRECT3D_VERSION >= 0x0600 */
	if (Desc.dwSize < D3DDEVICEDESC6_SIZE)
	{
		return;
	}
	Desc.dwMaxTextureRepeat = 0;
	Desc.dwMaxTextureAspectRatio = 0;
	Desc.dwMaxAnisotropy = 0;
	Desc.dvGuardBandLeft = 0;
	Desc.dvGuardBandTop = 0;
	Desc.dvGuardBandRight = 0;
	Desc.dvGuardBandBottom = 0;
	Desc.dvExtentsAdjust = 0;
	Desc.dwStencilCaps = 0;
	Desc.dwFVFCaps = 0;
	Desc.dwTextureOpCaps = 0;
	Desc.wMaxTextureBlendStages = 0;
	Desc.wMaxSimultaneousTextures = 0;
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
	Desc7.dwDevCaps = (Caps9.DevCaps & ~(D3DDEVCAPS_PUREDEVICE | D3DDEVCAPS_QUINTICRTPATCHES | D3DDEVCAPS_RTPATCHES | D3DDEVCAPS_RTPATCHHANDLEZERO | D3DDEVCAPS_NPATCHES)) |
		D3DDEVCAPS_FLOATTLVERTEX | D3DDEVCAPS_HWTRANSFORMANDLIGHT | 0x00100000L | 0x02000000L;
	Desc7.dwStencilCaps = Caps9.StencilCaps;
	Desc7.dwFVFCaps = (Caps9.FVFCaps & 0x8FFFF) | 0x00100000;
	Desc7.dwTextureOpCaps = (Caps9.TextureOpCaps & 0xFFFFFF) | 0x01000000 | 0x02000000;
	Desc7.dwVertexProcessingCaps = Caps9.VertexProcessingCaps & 0x3F;
	// Line Caps
	Desc7.dpcLineCaps.dwSize = sizeof(D3DPRIMCAPS);
	Desc7.dpcLineCaps.dwMiscCaps = (Caps9.PrimitiveMiscCaps & 0x72) | 0x00000200;
	Desc7.dpcLineCaps.dwRasterCaps = (Caps9.RasterCaps & 0x3FFFFF) | D3DPRASTERCAPS_SUBPIXEL | D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT | D3DPRASTERCAPS_ANTIALIASEDGES | D3DPRASTERCAPS_ZBIAS;
	Desc7.dpcLineCaps.dwZCmpCaps = Caps9.ZCmpCaps & 0xFF;
	Desc7.dpcLineCaps.dwSrcBlendCaps = Caps9.SrcBlendCaps & 0x1FFF;
	Desc7.dpcLineCaps.dwDestBlendCaps = Caps9.DestBlendCaps & 0x1FFF;
	Desc7.dpcLineCaps.dwAlphaCmpCaps = Caps9.AlphaCmpCaps & 0xFF;
	Desc7.dpcLineCaps.dwShadeCaps = (Caps9.ShadeCaps & 0x1FFFFF) | D3DPSHADECAPS_COLORFLATMONO | D3DPSHADECAPS_COLORFLATRGB | D3DPSHADECAPS_COLORGOURAUDMONO | D3DPSHADECAPS_SPECULARFLATRGB | 
		D3DPSHADECAPS_ALPHAFLATBLEND | D3DPSHADECAPS_FOGFLAT;
	Desc7.dpcLineCaps.dwTextureCaps = (Caps9.TextureCaps & 0x1FFF) | D3DPTEXTURECAPS_TRANSPARENCY;
	Desc7.dpcLineCaps.dwTextureFilterCaps = (Caps9.TextureFilterCaps & 0x1F373F) | D3DPTFILTERCAPS_NEAREST | D3DPTFILTERCAPS_LINEAR | D3DPTFILTERCAPS_MIPNEAREST | D3DPTFILTERCAPS_MIPLINEAR |
		D3DPTFILTERCAPS_LINEARMIPNEAREST | D3DPTFILTERCAPS_LINEARMIPLINEAR | D3DPTFILTERCAPS_MAGFPOINT | D3DPTFILTERCAPS_MAGFLINEAR;
	Desc7.dpcLineCaps.dwTextureBlendCaps = D3DPTBLENDCAPS_DECAL | D3DPTBLENDCAPS_MODULATE | D3DPTBLENDCAPS_DECALALPHA | D3DPTBLENDCAPS_MODULATEALPHA | D3DPTBLENDCAPS_COPY | D3DPTBLENDCAPS_ADD;
	Desc7.dpcLineCaps.dwTextureAddressCaps = Caps9.TextureAddressCaps & 0x1F;
	Desc7.dpcLineCaps.dwStippleWidth = 0;
	Desc7.dpcLineCaps.dwStippleHeight = 0;
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
		Desc7.dwDeviceRenderBitDepth = 3328;
		Desc7.dwDeviceZBufferBitDepth = 1536;
	}
	else if (Caps9.DeviceType == D3DDEVTYPE_HAL + 0x10)
	{
		Desc7.deviceGUID = IID_IDirect3DTnLHalDevice;
		Desc7.dwDevCaps |= D3DDEVCAPS_HWRASTERIZATION | D3DDEVCAPS_HWTRANSFORMANDLIGHT;
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
		LOG_LIMIT(100, __FUNCTION__ << " Error! Incorrect dwSize: " << ddpfPixelFormat.dwSize);
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
