/**
* Copyright (C) 2022 Elisha Riedlinger
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

void ConvertMaterial(D3DMATERIAL &Material, D3DMATERIAL &Material2)
{
	if (Material.dwSize != sizeof(D3DMATERIAL) || Material2.dwSize != sizeof(D3DMATERIAL))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << Material.dwSize << " " << Material2.dwSize);
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
		LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << Material.dwSize);
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
		LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << Material.dwSize);
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
		LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << ViewPort.dwSize << " " << ViewPort2.dwSize);
		return;
	}
	memcpy(&ViewPort, &ViewPort2, sizeof(D3DVIEWPORT));
}

void ConvertViewport(D3DVIEWPORT2 &ViewPort, D3DVIEWPORT2 &ViewPort2)
{
	if (ViewPort.dwSize != sizeof(D3DVIEWPORT2) || ViewPort2.dwSize != sizeof(D3DVIEWPORT2))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << ViewPort.dwSize << " " << ViewPort2.dwSize);
		return;
	}
	memcpy(&ViewPort, &ViewPort2, sizeof(D3DVIEWPORT2));
}

void ConvertViewport(D3DVIEWPORT &ViewPort, D3DVIEWPORT2 &ViewPort2)
{
	if (ViewPort.dwSize != sizeof(D3DVIEWPORT) || ViewPort2.dwSize != sizeof(D3DVIEWPORT2))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << ViewPort.dwSize << " " << ViewPort2.dwSize);
		return;
	}
	// Convert variables
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
		LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << ViewPort2.dwSize << " " << ViewPort.dwSize);
		return;
	}
	// Convert variables
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
		LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << ViewPort.dwSize);
		return;
	}
	// Convert variables
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
		LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << ViewPort2.dwSize);
		return;
	}
	// Convert variables
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
		LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << ViewPort.dwSize);
		return;
	}
	// Convert variables
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
		LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << ViewPort2.dwSize);
		return;
	}
	// Convert variables
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
		LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << PrimCaps.dwSize << " " << PrimCaps2.dwSize);
		return;
	}
	// Convert variables
	memcpy(&PrimCaps, &PrimCaps2, sizeof(D3DPRIMCAPS));
}

void ConvertDeviceDesc(D3DDEVICEDESC &Desc, D3DDEVICEDESC7 &Desc7)
{
	if (Desc.dwSize != D3DDEVICEDESC1_SIZE && Desc.dwSize != D3DDEVICEDESC5_SIZE && Desc.dwSize != D3DDEVICEDESC6_SIZE)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << Desc.dwSize);
		return;
	}
	// Convert variables
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
		LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << Desc.dwSize);
		return;
	}
	// Convert variables
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
	Desc7.wMaxTextureBlendStages = (WORD)min(Caps9.MaxTextureBlendStages, USHRT_MAX);
	Desc7.wMaxSimultaneousTextures = (WORD)min(Caps9.MaxSimultaneousTextures, USHRT_MAX);
	Desc7.dwMaxActiveLights = Caps9.MaxActiveLights;
	Desc7.dvMaxVertexW = Caps9.MaxVertexW;
	Desc7.wMaxUserClipPlanes = (WORD)min(Caps9.MaxUserClipPlanes, USHRT_MAX);
	Desc7.wMaxVertexBlendMatrices = (WORD)min(Caps9.MaxVertexBlendMatrices, USHRT_MAX);
	// Caps
	Desc7.dwDevCaps = (Caps9.DevCaps & ~(D3DDEVCAPS_PUREDEVICE | D3DDEVCAPS_QUINTICRTPATCHES | D3DDEVCAPS_RTPATCHES | D3DDEVCAPS_RTPATCHHANDLEZERO | D3DDEVCAPS_NPATCHES)) |
		D3DDEVCAPS_FLOATTLVERTEX;
	Desc7.dwStencilCaps = Caps9.StencilCaps;
	Desc7.dwFVFCaps = Caps9.FVFCaps & ~(D3DFVFCAPS_PSIZE);
	Desc7.dwTextureOpCaps = Caps9.TextureOpCaps & ~(D3DTEXOPCAPS_MULTIPLYADD | D3DTEXOPCAPS_LERP);
	Desc7.dwVertexProcessingCaps = (Caps9.VertexProcessingCaps & (D3DVTXPCAPS_TEXGEN | D3DVTXPCAPS_MATERIALSOURCE7 | D3DVTXPCAPS_DIRECTIONALLIGHTS | D3DVTXPCAPS_POSITIONALLIGHTS | D3DVTXPCAPS_LOCALVIEWER)) |
		D3DVTXPCAPS_VERTEXFOG;
	// Line Caps
	Desc7.dpcLineCaps.dwSize = sizeof(D3DPRIMCAPS);
	Desc7.dpcLineCaps.dwMiscCaps = Caps9.PrimitiveMiscCaps & (D3DPMISCCAPS_MASKZ | D3DPMISCCAPS_CULLNONE | D3DPMISCCAPS_CULLCW | D3DPMISCCAPS_CULLCCW);
	Desc7.dpcLineCaps.dwRasterCaps = (Caps9.RasterCaps & ~(D3DPRASTERCAPS_COLORPERSPECTIVE | D3DPRASTERCAPS_SCISSORTEST | D3DPRASTERCAPS_SLOPESCALEDEPTHBIAS | D3DPRASTERCAPS_DEPTHBIAS | D3DPRASTERCAPS_MULTISAMPLE_TOGGLE)) |
		D3DPRASTERCAPS_SUBPIXEL | D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT | D3DPRASTERCAPS_ANTIALIASEDGES | D3DPRASTERCAPS_ZBIAS;
	Desc7.dpcLineCaps.dwZCmpCaps = Caps9.ZCmpCaps;
	Desc7.dpcLineCaps.dwAlphaCmpCaps = Caps9.AlphaCmpCaps;
	Desc7.dpcLineCaps.dwSrcBlendCaps = Caps9.SrcBlendCaps & ~(D3DPBLENDCAPS_BLENDFACTOR);
	Desc7.dpcLineCaps.dwDestBlendCaps = Caps9.DestBlendCaps & ~(D3DPBLENDCAPS_BLENDFACTOR);
	Desc7.dpcLineCaps.dwShadeCaps = Caps9.ShadeCaps |
		D3DPSHADECAPS_COLORFLATMONO | D3DPSHADECAPS_COLORFLATRGB | D3DPSHADECAPS_COLORGOURAUDMONO | D3DPSHADECAPS_SPECULARFLATRGB | D3DPSHADECAPS_ALPHAFLATBLEND | D3DPSHADECAPS_FOGFLAT;
	Desc7.dpcLineCaps.dwTextureCaps = (Caps9.TextureCaps & (D3DPTEXTURECAPS_PERSPECTIVE | D3DPTEXTURECAPS_POW2 | D3DPTEXTURECAPS_ALPHA | D3DPTEXTURECAPS_SQUAREONLY | D3DPTEXTURECAPS_TEXREPEATNOTSCALEDBYSIZE |
		D3DPTEXTURECAPS_ALPHAPALETTE | D3DPTEXTURECAPS_NONPOW2CONDITIONAL | D3DPTEXTURECAPS_PROJECTED | D3DPTEXTURECAPS_CUBEMAP)) |
		D3DPTEXTURECAPS_TRANSPARENCY;
	Desc7.dpcLineCaps.dwTextureFilterCaps = (Caps9.TextureFilterCaps & ~(D3DPTFILTERCAPS_MINFPYRAMIDALQUAD | D3DPTFILTERCAPS_MINFGAUSSIANQUAD | D3DPTFILTERCAPS_CONVOLUTIONMONO)) |
		D3DPTFILTERCAPS_NEAREST | D3DPTFILTERCAPS_LINEAR | D3DPTFILTERCAPS_MIPNEAREST | D3DPTFILTERCAPS_MIPLINEAR | D3DPTFILTERCAPS_LINEARMIPNEAREST | D3DPTFILTERCAPS_LINEARMIPLINEAR;
	Desc7.dpcLineCaps.dwTextureBlendCaps = D3DPTBLENDCAPS_DECAL | D3DPTBLENDCAPS_MODULATE | D3DPTBLENDCAPS_DECALALPHA | D3DPTBLENDCAPS_MODULATEALPHA | D3DPTBLENDCAPS_COPY | D3DPTBLENDCAPS_ADD;
	Desc7.dpcLineCaps.dwTextureAddressCaps = Caps9.TextureAddressCaps & ~(D3DPTADDRESSCAPS_MIRRORONCE);
	Desc7.dpcLineCaps.dwStippleWidth = 0;
	Desc7.dpcLineCaps.dwStippleHeight = 0;
	// Tri Caps same as Line Caps
	memcpy(&Desc7.dpcTriCaps, &Desc7.dpcLineCaps, sizeof(D3DPRIMCAPS));
	// Specific settings
	if (Caps9.DeviceType == D3DDEVTYPE_REF)
	{
		Desc7.deviceGUID = IID_IDirect3DRGBDevice;
		Desc7.dwDevCaps &= ~(D3DDEVCAPS_HWTRANSFORMANDLIGHT);
		Desc7.dpcLineCaps.dwStippleWidth = 4;
		Desc7.dpcLineCaps.dwStippleHeight = 4;
		Desc7.dpcTriCaps.dwStippleWidth = 4;
		Desc7.dpcTriCaps.dwStippleHeight = 4;
		Desc7.dwDeviceRenderBitDepth = DDBD_8 | DDBD_16 | DDBD_24 | DDBD_32;
		Desc7.dwDeviceZBufferBitDepth = DDBD_16;
	}
	else if (Caps9.DeviceType == D3DDEVTYPE_HAL)
	{
		Desc7.deviceGUID = IID_IDirect3DHALDevice;
		Desc7.dwDevCaps = (Desc7.dwDevCaps | D3DDEVCAPS_HWRASTERIZATION) & ~(D3DDEVCAPS_HWTRANSFORMANDLIGHT);
		Desc7.dwDeviceRenderBitDepth = DDBD_8 | DDBD_16 | DDBD_32;
		Desc7.dwDeviceZBufferBitDepth = DDBD_16 | DDBD_24;
	}
	else if (Caps9.DeviceType == D3DDEVTYPE_HAL + 0x10)
	{
		Desc7.deviceGUID = IID_IDirect3DTnLHalDevice;
		Desc7.dwDevCaps |= Desc7.dwDevCaps | D3DDEVCAPS_HWRASTERIZATION;
		Desc7.dwDeviceRenderBitDepth = DDBD_8 | DDBD_16 | DDBD_32;
		Desc7.dwDeviceZBufferBitDepth = DDBD_16 | DDBD_24;
	}
	// Reserved
	Desc7.dwReserved1 = 0;
	Desc7.dwReserved2 = 0;
	Desc7.dwReserved3 = 0;
	Desc7.dwReserved4 = 0;
}

bool CheckTextureStageStateType(D3DTEXTURESTAGESTATETYPE dwState)
{
	switch (dwState)
	{
	case D3DTSS_COLOROP:
	case D3DTSS_COLORARG1:
	case D3DTSS_COLORARG2:
	case D3DTSS_ALPHAOP:
	case D3DTSS_ALPHAARG1:
	case D3DTSS_ALPHAARG2:
	case D3DTSS_BUMPENVMAT00:
	case D3DTSS_BUMPENVMAT01:
	case D3DTSS_BUMPENVMAT10:
	case D3DTSS_BUMPENVMAT11:
	case D3DTSS_TEXCOORDINDEX:
	case D3DTSS_BUMPENVLSCALE:
	case D3DTSS_BUMPENVLOFFSET:
	case D3DTSS_TEXTURETRANSFORMFLAGS:
	case D3DTSS_COLORARG0:
	case D3DTSS_ALPHAARG0:
	case D3DTSS_RESULTARG:
	case D3DTSS_CONSTANT:
		return true;
	default:
		return false;
	}
}

bool CheckRenderStateType(D3DRENDERSTATETYPE dwRenderStateType)
{
	switch ((DWORD)dwRenderStateType)
	{
	case D3DRS_ZENABLE:
	case D3DRS_FILLMODE:
	case D3DRS_SHADEMODE:
	case D3DRS_ZWRITEENABLE:
	case D3DRS_ALPHATESTENABLE:
	case D3DRS_LASTPIXEL:
	case D3DRS_SRCBLEND:
	case D3DRS_DESTBLEND:
	case D3DRS_CULLMODE:
	case D3DRS_ZFUNC:
	case D3DRS_ALPHAREF:
	case D3DRS_ALPHAFUNC:
	case D3DRS_DITHERENABLE:
	case D3DRS_ALPHABLENDENABLE:
	case D3DRS_FOGENABLE:
	case D3DRS_SPECULARENABLE:
	case D3DRS_FOGCOLOR:
	case D3DRS_FOGTABLEMODE:
	case D3DRS_FOGSTART:
	case D3DRS_FOGEND:
	case D3DRS_FOGDENSITY:
	case D3DRS_RANGEFOGENABLE:
	case D3DRS_STENCILENABLE:
	case D3DRS_STENCILFAIL:
	case D3DRS_STENCILZFAIL:
	case D3DRS_STENCILPASS:
	case D3DRS_STENCILFUNC:
	case D3DRS_STENCILREF:
	case D3DRS_STENCILMASK:
	case D3DRS_STENCILWRITEMASK:
	case D3DRS_TEXTUREFACTOR:
	case D3DRS_WRAP0:
	case D3DRS_WRAP1:
	case D3DRS_WRAP2:
	case D3DRS_WRAP3:
	case D3DRS_WRAP4:
	case D3DRS_WRAP5:
	case D3DRS_WRAP6:
	case D3DRS_WRAP7:
	case D3DRS_CLIPPING:
	case D3DRS_LIGHTING:
	case D3DRS_AMBIENT:
	case D3DRS_FOGVERTEXMODE:
	case D3DRS_COLORVERTEX:
	case D3DRS_LOCALVIEWER:
	case D3DRS_NORMALIZENORMALS:
	case D3DRS_DIFFUSEMATERIALSOURCE:
	case D3DRS_SPECULARMATERIALSOURCE:
	case D3DRS_AMBIENTMATERIALSOURCE:
	case D3DRS_EMISSIVEMATERIALSOURCE:
	case D3DRS_VERTEXBLEND:
	case D3DRS_CLIPPLANEENABLE:
	case D3DRS_POINTSIZE:
	case D3DRS_POINTSIZE_MIN:
	case D3DRS_POINTSPRITEENABLE:
	case D3DRS_POINTSCALEENABLE:
	case D3DRS_POINTSCALE_A:
	case D3DRS_POINTSCALE_B:
	case D3DRS_POINTSCALE_C:
	case D3DRS_MULTISAMPLEANTIALIAS:
	case D3DRS_MULTISAMPLEMASK:
	case D3DRS_PATCHEDGESTYLE:
	case D3DRS_DEBUGMONITORTOKEN:
	case D3DRS_POINTSIZE_MAX:
	case D3DRS_INDEXEDVERTEXBLENDENABLE:
	case D3DRS_COLORWRITEENABLE:
	case D3DRS_TWEENFACTOR:
	case D3DRS_BLENDOP:
	case D3DRS_POSITIONDEGREE:
	case D3DRS_NORMALDEGREE:
	case D3DRS_SCISSORTESTENABLE:
	case D3DRS_SLOPESCALEDEPTHBIAS:
	case D3DRS_ANTIALIASEDLINEENABLE:
	case D3DRS_MINTESSELLATIONLEVEL:
	case D3DRS_MAXTESSELLATIONLEVEL:
	case D3DRS_ADAPTIVETESS_X:
	case D3DRS_ADAPTIVETESS_Y:
	case D3DRS_ADAPTIVETESS_Z:
	case D3DRS_ADAPTIVETESS_W:
	case D3DRS_ENABLEADAPTIVETESSELLATION:
	case D3DRS_TWOSIDEDSTENCILMODE:
	case D3DRS_CCW_STENCILFAIL:
	case D3DRS_CCW_STENCILZFAIL:
	case D3DRS_CCW_STENCILPASS:
	case D3DRS_CCW_STENCILFUNC:
	case D3DRS_COLORWRITEENABLE1:
	case D3DRS_COLORWRITEENABLE2:
	case D3DRS_COLORWRITEENABLE3:
	case D3DRS_BLENDFACTOR:
	case D3DRS_SRGBWRITEENABLE:
	case D3DRS_DEPTHBIAS:
	case D3DRS_WRAP8:
	case D3DRS_WRAP9:
	case D3DRS_WRAP10:
	case D3DRS_WRAP11:
	case D3DRS_WRAP12:
	case D3DRS_WRAP13:
	case D3DRS_WRAP14:
	case D3DRS_WRAP15:
	case D3DRS_SEPARATEALPHABLENDENABLE:
	case D3DRS_SRCBLENDALPHA:
	case D3DRS_DESTBLENDALPHA:
	case D3DRS_BLENDOPALPHA:
		return true;
	default:
		return false;
	}
}
