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

void ConvertMaterial(D3DMATERIAL &Material, D3DMATERIAL7 &Material7)
{
	Material.dwSize = sizeof(D3DMATERIAL);
	Material.diffuse = Material7.diffuse;
	Material.dcvDiffuse = Material7.dcvDiffuse;
	Material.ambient = Material7.ambient;
	Material.dcvAmbient = Material7.dcvAmbient;
	Material.specular = Material7.specular;
	Material.dcvSpecular = Material7.dcvSpecular;
	Material.emissive = Material7.emissive;
	Material.dcvEmissive = Material7.dcvEmissive;
	Material.power = Material7.power;
	Material.dvPower = Material7.dvPower;
	// Extra parameters
	Material.hTexture = 0;    /* Handle to texture map */
	Material.dwRampSize = 0;
}

void ConvertMaterial(D3DMATERIAL7 &Material7, D3DMATERIAL &Material)
{
	Material7.diffuse = Material7.diffuse;
	Material7.dcvDiffuse = Material7.dcvDiffuse;
	Material7.ambient = Material7.ambient;
	Material7.dcvAmbient = Material7.dcvAmbient;
	Material7.specular = Material7.specular;
	Material7.dcvSpecular = Material7.dcvSpecular;
	Material7.emissive = Material7.emissive;
	Material7.dcvEmissive = Material7.dcvEmissive;
	Material7.power = Material7.power;
	Material.dvPower = Material7.dvPower;
}

void ConvertViewport(D3DVIEWPORT &ViewPort, D3DVIEWPORT &ViewPort2)
{
	memcpy(&ViewPort, &ViewPort2, sizeof(D3DVIEWPORT));
}

void ConvertViewport(D3DVIEWPORT2 &ViewPort, D3DVIEWPORT2 &ViewPort2)
{
	memcpy(&ViewPort, &ViewPort2, sizeof(D3DVIEWPORT2));
}

void ConvertViewport(D3DVIEWPORT &ViewPort, D3DVIEWPORT2 &ViewPort2)
{
	ViewPort.dwSize = 5 * 8;
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
	ViewPort2.dwSize = 5 * 8;
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
	ViewPort.dwSize = 5 * 8;
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
	ViewPort2.dwSize = 5 * 8;
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
	ViewPort7.dwX = ViewPort.dwX;
	ViewPort7.dwY = ViewPort.dwY;
	ViewPort7.dwWidth = ViewPort.dwWidth;
	ViewPort7.dwHeight = ViewPort.dwHeight;
	ViewPort7.dvMinZ = ViewPort.dvMinZ;
	ViewPort7.dvMaxZ = ViewPort.dvMaxZ;
}

void ConvertViewport(D3DVIEWPORT7 &ViewPort7, D3DVIEWPORT2 &ViewPort2)
{
	ViewPort7.dwX = ViewPort2.dwX;
	ViewPort7.dwY = ViewPort2.dwY;
	ViewPort7.dwWidth = ViewPort2.dwWidth;
	ViewPort7.dwHeight = ViewPort2.dwHeight;
	ViewPort7.dvMinZ = ViewPort2.dvMinZ;
	ViewPort7.dvMaxZ = ViewPort2.dvMaxZ;
}

void ConvertViewport(D3DVIEWPORT7 &ViewPort, D3DVIEWPORT7 &ViewPort7)
{
	CopyMemory(&ViewPort, &ViewPort7, sizeof(D3DVIEWPORT7));
}

void ConvertDeviceDesc(D3DDEVICEDESC &Desc, D3DDEVICEDESC7 &Desc7)
{
	if (Desc.dwSize == 0)
	{
		Desc.dwSize = sizeof(D3DDEVICEDESC);
	}
	if (Desc.dwSize > 0)
	{
		Desc.dwDevCaps = Desc7.dwDevCaps;
		CopyMemory(&Desc.dpcLineCaps, &Desc7.dpcLineCaps, sizeof(D3DPRIMCAPS));
		CopyMemory(&Desc.dpcTriCaps, &Desc7.dpcTriCaps, sizeof(D3DPRIMCAPS));
		Desc.dwDeviceRenderBitDepth = Desc7.dwDeviceRenderBitDepth;
		Desc.dwDeviceZBufferBitDepth = Desc7.dwDeviceZBufferBitDepth;
		Desc.dwMaxTextureRepeat = Desc7.dwMaxTextureRepeat;
		Desc.dwFlags = (D3DDD_DEVCAPS | D3DDD_LINECAPS | D3DDD_TRICAPS | D3DDD_DEVICERENDERBITDEPTH | D3DDD_DEVICEZBUFFERBITDEPTH);
		// Extra parameters
		Desc.dcmColorModel = 0;                 /* Color model of device */
		Desc.dtcTransformCaps.dwCaps = 0;       /* Capabilities of transform */
		Desc.dtcTransformCaps.dwSize = sizeof(D3DTRANSFORMCAPS);
		Desc.bClipping = 0;                     /* Device can do 3D clipping */
		Desc.dlcLightingCaps.dwCaps = 0;        /* Capabilities of lighting */
		Desc.dlcLightingCaps.dwLightingModel = 0;
		Desc.dlcLightingCaps.dwNumLights = 0;
		Desc.dlcLightingCaps.dwSize = sizeof(D3DLIGHTINGCAPS);
		Desc.dwMaxBufferSize = 0;               /* Maximum execute buffer size */
		Desc.dwMaxVertexCount = 0;              /* Maximum vertex count */
	}

	if (Desc.dwSize > 14 * 8) /* DIRECT3D_VERSION >= 0x0500 */
	{
		Desc.dwMinTextureWidth = Desc7.dwMinTextureWidth;
		Desc.dwMinTextureHeight = Desc7.dwMinTextureHeight;
		Desc.dwMaxTextureWidth = Desc7.dwMaxTextureWidth;
		Desc.dwMaxTextureHeight = Desc7.dwMaxTextureHeight;
		// Extra parameters
		Desc.dwMinStippleWidth = 0;
		Desc.dwMaxStippleWidth = 0;
		Desc.dwMinStippleHeight = 0;
		Desc.dwMaxStippleHeight = 0;
	}

	if (Desc.dwSize > 22 * 8) /* DIRECT3D_VERSION >= 0x0600 */
	{
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
