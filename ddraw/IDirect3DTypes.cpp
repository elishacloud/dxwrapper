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
	// Extra parameters
	ViewPort7.dvMinZ = 0;         /* Min/max of clip Volume */
	ViewPort7.dvMaxZ = 1.0f;
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
	if (Desc.dwSize > 0)
	{
		Desc.dwDevCaps = Desc7.dwDevCaps;
		CopyMemory(&Desc.dpcLineCaps, &Desc7.dpcLineCaps, sizeof(D3DPRIMCAPS));
		CopyMemory(&Desc.dpcTriCaps, &Desc7.dpcTriCaps, sizeof(D3DPRIMCAPS));
		Desc.dwDeviceRenderBitDepth = Desc7.dwDeviceRenderBitDepth;
		Desc.dwDeviceZBufferBitDepth = Desc7.dwDeviceZBufferBitDepth;
		Desc.dwMaxTextureRepeat = Desc7.dwMaxTextureRepeat;
		Desc.dwFlags = D3DDD_DEVCAPS | D3DDD_LINECAPS | D3DDD_TRICAPS | D3DDD_DEVICERENDERBITDEPTH | D3DDD_DEVICEZBUFFERBITDEPTH;
		// Extra parameters
		Desc.dcmColorModel = 0;                 /* Color model of device */
		Desc.dtcTransformCaps.dwCaps = 0;       /* Capabilities of transform */
		Desc.dtcTransformCaps.dwSize = 0;
		Desc.bClipping = 0;                     /* Device can do 3D clipping */
		Desc.dlcLightingCaps.dwCaps = 0;        /* Capabilities of lighting */
		Desc.dlcLightingCaps.dwLightingModel = 0;
		Desc.dlcLightingCaps.dwNumLights = 0;
		Desc.dlcLightingCaps.dwSize = 0;
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
