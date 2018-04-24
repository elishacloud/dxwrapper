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
		Desc.dwFlags = 0;
		Desc.dcmColorModel = 0;
		Desc.dtcTransformCaps.dwCaps = 0;
		Desc.dtcTransformCaps.dwSize = 0;
		Desc.bClipping = 0;
		Desc.dlcLightingCaps.dwCaps = 0;
		Desc.dlcLightingCaps.dwLightingModel = 0;
		Desc.dlcLightingCaps.dwNumLights = 0;
		Desc.dlcLightingCaps.dwSize = 0;
		Desc.dwMaxBufferSize = 0;
		Desc.dwMaxVertexCount = 0;
	}

	if (Desc.dwSize > 14 * 8) /* DIRECT3D_VERSION >= 0x0500 */
	{
		Desc.dwMinTextureWidth = Desc7.dwMinTextureWidth;
		Desc.dwMinTextureHeight = Desc7.dwMinTextureHeight;
		Desc.dwMaxTextureWidth = Desc7.dwMaxTextureWidth;
		Desc.dwMaxTextureHeight = Desc7.dwMaxTextureHeight;
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
