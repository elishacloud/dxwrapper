/**
* Copyright (C) 2025 Elisha Riedlinger
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
#include "d3d9\d3d9External.h"

void ConvertLight(D3DLIGHT7& Light7, const D3DLIGHT& Light)
{
	if (Light.dwSize != sizeof(D3DLIGHT) && Light.dwSize != sizeof(D3DLIGHT2))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << Light.dwSize);
		return;
	}

	Light7.dltType = Light.dltType;
	Light7.dcvDiffuse = Light.dcvColor;
	Light7.dcvSpecular = { 0.0f, 0.0f, 0.0f, 0.0f };
	const float scale = 0.5f;
	Light7.dcvAmbient.r = Light.dcvColor.r * scale;
	Light7.dcvAmbient.g = Light.dcvColor.g * scale;
	Light7.dcvAmbient.b = Light.dcvColor.b * scale;
	Light7.dcvAmbient.a = Light.dcvColor.a;
	Light7.dvPosition = Light.dvPosition;
	Light7.dvDirection = Light.dvDirection;
	Light7.dvRange = Light.dvRange;
	Light7.dvFalloff = Light.dvFalloff;
	Light7.dvAttenuation0 = Light.dvAttenuation0;
	Light7.dvAttenuation1 = Light.dvAttenuation1;
	Light7.dvAttenuation2 = Light.dvAttenuation2;
	Light7.dvTheta = Light.dvTheta;
	Light7.dvPhi = Light.dvPhi;
}

D3DLIGHT9 FixLight(const D3DLIGHT9& Light)
{
	D3DLIGHT9 result = Light;

	// Make spot light work more like it did in Direct3D7
	if (result.Type == D3DLIGHT_SPOT)
	{
		// Theta must be in the range from 0 through the value specified by Phi
		if (result.Theta <= result.Phi)
		{
			result.Theta /= 1.75f;
		}
	}

	return result;
}

void ConvertMaterial(D3DMATERIAL& Material, const D3DMATERIAL7& Material7)
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
	Material.hTexture = NULL;
	Material.dwRampSize = 0;
}

void ConvertMaterial(D3DMATERIAL7& Material7, const D3DMATERIAL& Material)
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

void ConvertViewport(D3DVIEWPORT& Viewport, const D3DVIEWPORT2& Viewport2)
{
	if (Viewport.dwSize != sizeof(D3DVIEWPORT) || Viewport2.dwSize != sizeof(D3DVIEWPORT2))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << Viewport.dwSize << " " << Viewport2.dwSize);
		return;
	}
	// Convert variables
	Viewport.dwX = Viewport2.dwX;
	Viewport.dwY = Viewport2.dwY;
	Viewport.dwWidth = Viewport2.dwWidth;
	Viewport.dwHeight = Viewport2.dwHeight;
	Viewport.dvMinZ = Viewport2.dvMinZ;
	Viewport.dvMaxZ = Viewport2.dvMaxZ;
	// Extra parameters
	Viewport.dvScaleX = 0;        /* Scale homogeneous to screen */
	Viewport.dvScaleY = 0;        /* Scale homogeneous to screen */
	Viewport.dvMaxX = 0;          /* Min/max homogeneous x coord */
	Viewport.dvMaxY = 0;          /* Min/max homogeneous y coord */
}

void ConvertViewport(D3DVIEWPORT2& Viewport2, const D3DVIEWPORT& Viewport)
{
	if (Viewport2.dwSize != sizeof(D3DVIEWPORT2) || Viewport.dwSize != sizeof(D3DVIEWPORT))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << Viewport2.dwSize << " " << Viewport.dwSize);
		return;
	}
	// Convert variables
	Viewport2.dwX = Viewport.dwX;
	Viewport2.dwY = Viewport.dwY;
	Viewport2.dwWidth = Viewport.dwWidth;
	Viewport2.dwHeight = Viewport.dwHeight;
	Viewport2.dvMinZ = Viewport.dvMinZ;
	Viewport2.dvMaxZ = Viewport.dvMaxZ;
	// Extra parameters
	Viewport2.dvClipX = 0;        /* Top left of clip volume */
	Viewport2.dvClipY = 0;
	Viewport2.dvClipWidth = 0;    /* Clip Volume Dimensions */
	Viewport2.dvClipHeight = 0;
}

void ConvertViewport(D3DVIEWPORT& Viewport, const D3DVIEWPORT7& Viewport7)
{
	if (Viewport.dwSize != sizeof(D3DVIEWPORT))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << Viewport.dwSize);
		return;
	}
	// Convert variables
	Viewport.dwX = Viewport7.dwX;
	Viewport.dwY = Viewport7.dwY;
	Viewport.dwWidth = Viewport7.dwWidth;
	Viewport.dwHeight = Viewport7.dwHeight;
	Viewport.dvMinZ = Viewport7.dvMinZ;
	Viewport.dvMaxZ = Viewport7.dvMaxZ;
	// Extra parameters
	Viewport.dvScaleX = 0;        /* Scale homogeneous to screen */
	Viewport.dvScaleY = 0;        /* Scale homogeneous to screen */
	Viewport.dvMaxX = 0;          /* Min/max homogeneous x coord */
	Viewport.dvMaxY = 0;          /* Min/max homogeneous y coord */
}

void ConvertViewport(D3DVIEWPORT2& Viewport2, const D3DVIEWPORT7& Viewport7)
{
	if (Viewport2.dwSize != sizeof(D3DVIEWPORT2))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << Viewport2.dwSize);
		return;
	}
	// Convert variables
	Viewport2.dwX = Viewport7.dwX;
	Viewport2.dwY = Viewport7.dwY;
	Viewport2.dwWidth = Viewport7.dwWidth;
	Viewport2.dwHeight = Viewport7.dwHeight;
	Viewport2.dvMinZ = Viewport7.dvMinZ;
	Viewport2.dvMaxZ = Viewport7.dvMaxZ;
	// Extra parameters
	Viewport2.dvClipX = 0;        /* Top left of clip volume */
	Viewport2.dvClipY = 0;
	Viewport2.dvClipWidth = 0;    /* Clip Volume Dimensions */
	Viewport2.dvClipHeight = 0;
}

void ConvertViewport(D3DVIEWPORT7& Viewport7, const D3DVIEWPORT& Viewport)
{
	if (Viewport.dwSize != sizeof(D3DVIEWPORT))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << Viewport.dwSize);
		return;
	}
	// Convert variables
	Viewport7.dwX = Viewport.dwX;
	Viewport7.dwY = Viewport.dwY;
	Viewport7.dwWidth = Viewport.dwWidth;
	Viewport7.dwHeight = Viewport.dwHeight;
	Viewport7.dvMinZ = Viewport.dvMinZ;
	Viewport7.dvMaxZ = Viewport.dvMaxZ;
}

void ConvertViewport(D3DVIEWPORT7& Viewport7, const D3DVIEWPORT2& Viewport2)
{
	if (Viewport2.dwSize != sizeof(D3DVIEWPORT2))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << Viewport2.dwSize);
		return;
	}
	// Convert variables
	Viewport7.dwX = Viewport2.dwX;
	Viewport7.dwY = Viewport2.dwY;
	Viewport7.dwWidth = Viewport2.dwWidth;
	Viewport7.dwHeight = Viewport2.dwHeight;
	Viewport7.dvMinZ = Viewport2.dvMinZ;
	Viewport7.dvMaxZ = Viewport2.dvMaxZ;
}

void ConvertViewport(D3DVIEWPORT7& Viewport, const D3DVIEWPORT7& Viewport7)
{
	Viewport.dwX = Viewport7.dwX;
	Viewport.dwY = Viewport7.dwY;
	Viewport.dwWidth = Viewport7.dwWidth;
	Viewport.dwHeight = Viewport7.dwHeight;
	Viewport.dvMinZ = Viewport7.dvMinZ;
	Viewport.dvMaxZ = Viewport7.dvMaxZ;
}

bool IsValidRenderState(D3DRENDERSTATETYPE dwRenderStateType, DWORD& Value, DWORD DirectXVersion)
{
	if (dwRenderStateType >= D3D_MAXRENDERSTATES)
	{
		Value = 0;
		return false;
	}

	if (DirectXVersion == 1)	// IDirect3D (DX 2 & 3)
	{
		if (dwRenderStateType == 0									// 0
			|| (dwRenderStateType > 39 && dwRenderStateType < 64)	// 40-63
			|| dwRenderStateType > 95)								// 96-256
		{
			return false;
		}
	}
	else if (DirectXVersion < 7)	// IDirect3D 2 & 3 (DX 5 & 6)
	{
		switch ((DWORD)dwRenderStateType)
		{
		case D3DRENDERSTATE_NONE:				// 0
			Value = 0;
			return false;
		case D3DRENDERSTATE_FLUSHBATCH:			// 50
			Value = (DWORD)-1;
			return false;
		}
	}
	else if (DirectXVersion == 7)	// IDirect3D 7 (DX 7)
	{
		switch ((DWORD)dwRenderStateType)
		{
		case D3DRENDERSTATE_TEXTUREHANDLE:		// 1
		case D3DRENDERSTATE_TEXTUREADDRESS:		// 3
		case D3DRENDERSTATE_WRAPU:				// 5
		case D3DRENDERSTATE_WRAPV:				// 6
		case D3DRENDERSTATE_MONOENABLE:			// 11
		case D3DRENDERSTATE_ROP2:				// 12
		case D3DRENDERSTATE_PLANEMASK:			// 13
		case D3DRENDERSTATE_TEXTUREMAG:			// 17
		case D3DRENDERSTATE_TEXTUREMIN:			// 18
		case D3DRENDERSTATE_TEXTUREMAPBLEND:	// 21
		case D3DRENDERSTATE_SUBPIXEL:			// 31
		case D3DRENDERSTATE_SUBPIXELX:			// 32
		case D3DRENDERSTATE_STIPPLEENABLE:		// 39
		case D3DRENDERSTATE_BORDERCOLOR:		// 43
		case D3DRENDERSTATE_TEXTUREADDRESSU:	// 44
		case D3DRENDERSTATE_TEXTUREADDRESSV:	// 45
		case D3DRENDERSTATE_MIPMAPLODBIAS:		// 46
		case D3DRENDERSTATE_ANISOTROPY:			// 49
		case D3DRENDERSTATE_TRANSLUCENTSORTINDEPENDENT:			// 51
			LOG_LIMIT(100, __FUNCTION__ << " Warning: ignoring undocumented DX7 Render state: " << dwRenderStateType);
			Value = 0;
			return false;
		}
		if (dwRenderStateType > 63 && dwRenderStateType < 96)	// 64-95
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: ignoring undocumented DX7 Render state: " << dwRenderStateType);
			Value = 0;
			return false;
		}
	}

	return true;
}

bool IsOutOfRangeRenderState(D3DRENDERSTATETYPE dwRenderStateType, DWORD DirectXVersion)
{
	switch (DirectXVersion)
	{
	case 2:
	case 3:
		if (dwRenderStateType > 135)
		{
			return true;
		}
		break;
	case 7:
		if (dwRenderStateType == 149 || dwRenderStateType == 150 || dwRenderStateType > 152)
		{
			return true;
		}
		break;
	}
	return false;
}

DWORD FixSamplerState(D3DSAMPLERSTATETYPE Type, DWORD Value)
{
	if (Type == D3DSAMP_MAGFILTER)
	{
		if (Value == D3DTFG_ANISOTROPIC)
		{
			return D3DTEXF_ANISOTROPIC;
		}
		else if (Value == D3DTFG_FLATCUBIC || Value == D3DTFG_GAUSSIANCUBIC)
		{
			return D3DTEXF_LINEAR;
		}
	}
	if (Type == D3DSAMP_MIPFILTER)
	{
		switch (Value)
		{
		default:
		case D3DTFP_NONE:
			return D3DTEXF_NONE;
		case D3DTFP_POINT:
			return D3DTEXF_POINT;
		case D3DTFP_LINEAR:
			return D3DTEXF_LINEAR;
		}
	}
	return Value;
}

bool IsValidTransformState(D3DTRANSFORMSTATETYPE State)
{
	switch ((DWORD)State)
	{
	case D3DTS_VIEW:
	case D3DTS_PROJECTION:
	case D3DTS_WORLD:
	case D3DTS_WORLD1:
	case D3DTS_WORLD2:
	case D3DTS_WORLD3:
		return true;

	case D3DTS_TEXTURE0:
	case D3DTS_TEXTURE1:
	case D3DTS_TEXTURE2:
	case D3DTS_TEXTURE3:
	case D3DTS_TEXTURE4:
	case D3DTS_TEXTURE5:
	case D3DTS_TEXTURE6:
	case D3DTS_TEXTURE7:
		return true;

	default:
		return false;
	}
}

D3DMATRIX FixMatrix(const D3DMATRIX& Matrix, D3DTRANSFORMSTATETYPE State, D3DVIEWPORT Viewport, bool ScaleMatrix)
{
	D3DMATRIX result = Matrix;

	if (ScaleMatrix && State == D3DTS_PROJECTION)
	{
		if (Viewport.dvScaleX != 0 && Viewport.dvScaleY != 0 && Viewport.dvMaxX != 0 && Viewport.dvMaxY != 0 && Viewport.dwWidth != 0 && Viewport.dwHeight != 0)
		{
			const float sx = Viewport.dvScaleX / (Viewport.dwWidth * 0.5f * Viewport.dvMaxX);
			const float sy = Viewport.dvScaleY / (Viewport.dwHeight * 0.5f * Viewport.dvMaxY);

			result._11 *= sx;
			result._22 *= sy;
		}
	}

	return result;
}

void ConvertDeviceDesc(D3DDEVICEDESC& Desc, const D3DDEVICEDESC7& Desc7)
{
	if (Desc.dwSize != D3DDEVICEDESC1_SIZE && Desc.dwSize != D3DDEVICEDESC5_SIZE && Desc.dwSize != D3DDEVICEDESC6_SIZE)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << Desc.dwSize);
		return;
	}

	// Initialize the output structure
	DWORD Size = Desc.dwSize;
	ZeroMemory(&Desc, Size);
	Desc.dwSize = Size;

	// Convert relevant fields
	Desc.dwFlags = D3DDD_COLORMODEL |
		D3DDD_DEVCAPS |
		D3DDD_TRANSFORMCAPS |
		D3DDD_LIGHTINGCAPS |
		D3DDD_BCLIPPING |
		D3DDD_LINECAPS |
		D3DDD_TRICAPS |
		D3DDD_DEVICERENDERBITDEPTH |
		D3DDD_DEVICEZBUFFERBITDEPTH |
		D3DDD_MAXBUFFERSIZE |
		D3DDD_MAXVERTEXCOUNT;
	Desc.dcmColorModel = D3DCOLOR_RGB;
	Desc.dwDevCaps = Desc7.dwDevCaps;
	Desc.dtcTransformCaps.dwSize = sizeof(D3DTRANSFORMCAPS);
	Desc.dtcTransformCaps.dwCaps = D3DTRANSFORMCAPS_CLIP;
	Desc.bClipping = TRUE;
	Desc.dlcLightingCaps.dwSize = sizeof(D3DLIGHTINGCAPS);
	Desc.dlcLightingCaps.dwCaps = D3DLIGHTCAPS_POINT | D3DLIGHTCAPS_SPOT | D3DLIGHTCAPS_DIRECTIONAL | D3DLIGHTCAPS_PARALLELPOINT | D3DLIGHTCAPS_GLSPOT;
	Desc.dlcLightingCaps.dwLightingModel = 1;
	Desc.dlcLightingCaps.dwNumLights = Desc7.dwMaxActiveLights;
	Desc.dpcLineCaps = Desc7.dpcLineCaps;
	Desc.dpcTriCaps = Desc7.dpcTriCaps;
	Desc.dwDeviceRenderBitDepth = Desc7.dwDeviceRenderBitDepth;
	Desc.dwDeviceZBufferBitDepth = Desc7.dwDeviceZBufferBitDepth;
	Desc.dwMaxBufferSize = MAX_EXECUTE_BUFFER_SIZE;
	Desc.dwMaxVertexCount = 65534;

	// Handle additional fields depending on the structure size
	if (Desc.dwSize >= D3DDEVICEDESC5_SIZE)
	{
		Desc.dwMinTextureWidth = Desc7.dwMinTextureWidth;
		Desc.dwMinTextureHeight = Desc7.dwMinTextureHeight;
		Desc.dwMaxTextureWidth = Desc7.dwMaxTextureWidth;
		Desc.dwMaxTextureHeight = Desc7.dwMaxTextureHeight;

		// Initialize fields specific to D3DDEVICEDESC6
		if (Desc.dwSize >= D3DDEVICEDESC6_SIZE)
		{
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
	}

	// Ignore fields specific to D3DDEVICEDESC7 that are not in D3DDEVICEDESC
	// Desc.dvMaxVertexW = Desc7.dvMaxVertexW;
	// Desc.wMaxUserClipPlanes = Desc7.wMaxUserClipPlanes;
	// Desc.wMaxVertexBlendMatrices = Desc7.wMaxVertexBlendMatrices;
	// Desc.dwVertexProcessingCaps = Desc7.dwVertexProcessingCaps;
}

void ConvertDeviceDesc(D3DDEVICEDESC7& Desc7, const D3DCAPS9& Caps9)
{
	// Initialize the output structure
	ZeroMemory(&Desc7, sizeof(D3DDEVICEDESC7));

	// Device capabilities
	Desc7.dwDevCaps =
		D3DDEVCAPS_FLOATTLVERTEX |
		D3DDEVCAPS_SEPARATETEXTUREMEMORIES |
		D3DDEVCAPS_EXECUTESYSTEMMEMORY |
		D3DDEVCAPS_EXECUTEVIDEOMEMORY |
		(Caps9.DevCaps &
			(D3DDEVCAPS_TLVERTEXSYSTEMMEMORY |
			D3DDEVCAPS_TLVERTEXVIDEOMEMORY |
			D3DDEVCAPS_TEXTURESYSTEMMEMORY |
			D3DDEVCAPS_TEXTUREVIDEOMEMORY |
			D3DDEVCAPS_DRAWPRIMTLVERTEX |
			D3DDEVCAPS_CANRENDERAFTERFLIP |
			D3DDEVCAPS_TEXTURENONLOCALVIDMEM |
			D3DDEVCAPS_DRAWPRIMITIVES2 |
			D3DDEVCAPS_DRAWPRIMITIVES2EX |
			D3DDEVCAPS_HWTRANSFORMANDLIGHT |
			D3DDEVCAPS_CANBLTSYSTONONLOCAL |
			D3DDEVCAPS_HWRASTERIZATION));

	// Stencil capabilities
	Desc7.dwStencilCaps =
		(Caps9.StencilCaps &
			(D3DSTENCILCAPS_KEEP |
			D3DSTENCILCAPS_ZERO |
			D3DSTENCILCAPS_REPLACE |
			D3DSTENCILCAPS_INCRSAT |
			D3DSTENCILCAPS_DECRSAT |
			D3DSTENCILCAPS_INVERT |
			D3DSTENCILCAPS_INCR |
			D3DSTENCILCAPS_DECR));

	// FVF capabilities
	Desc7.dwFVFCaps = (Caps9.FVFCaps & (D3DFVFCAPS_DONOTSTRIPELEMENTS));

	// Texture operation capabilities
	Desc7.dwTextureOpCaps =
		(Caps9.TextureOpCaps &
			(D3DTEXOPCAPS_DISABLE |
			D3DTEXOPCAPS_SELECTARG1 |
			D3DTEXOPCAPS_SELECTARG2 |
			D3DTEXOPCAPS_MODULATE |
			D3DTEXOPCAPS_MODULATE2X |
			D3DTEXOPCAPS_MODULATE4X |
			D3DTEXOPCAPS_ADD |
			D3DTEXOPCAPS_ADDSIGNED |
			D3DTEXOPCAPS_ADDSIGNED2X |
			D3DTEXOPCAPS_SUBTRACT |
			D3DTEXOPCAPS_ADDSMOOTH |
			D3DTEXOPCAPS_BLENDDIFFUSEALPHA |
			D3DTEXOPCAPS_BLENDTEXTUREALPHA |
			D3DTEXOPCAPS_BLENDFACTORALPHA |
			D3DTEXOPCAPS_BLENDTEXTUREALPHAPM |
			D3DTEXOPCAPS_BLENDCURRENTALPHA |
			D3DTEXOPCAPS_PREMODULATE |
			D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR |
			D3DTEXOPCAPS_MODULATECOLOR_ADDALPHA |
			D3DTEXOPCAPS_MODULATEINVALPHA_ADDCOLOR |
			D3DTEXOPCAPS_MODULATEINVCOLOR_ADDALPHA |
			D3DTEXOPCAPS_BUMPENVMAP |
			D3DTEXOPCAPS_BUMPENVMAPLUMINANCE |
			D3DTEXOPCAPS_DOTPRODUCT3 |
			D3DTEXOPCAPS_MULTIPLYADD |
			D3DTEXOPCAPS_LERP));

	// Vertex processing capabilities
	Desc7.dwVertexProcessingCaps =
		D3DVTXPCAPS_VERTEXFOG |
		(Caps9.VertexProcessingCaps &
			(D3DVTXPCAPS_TEXGEN |
			D3DVTXPCAPS_MATERIALSOURCE7 |
			D3DVTXPCAPS_DIRECTIONALLIGHTS |
			D3DVTXPCAPS_POSITIONALLIGHTS |
			D3DVTXPCAPS_LOCALVIEWER));

	// Line capabilities
	Desc7.dpcLineCaps.dwSize = sizeof(D3DPRIMCAPS);
	Desc7.dpcLineCaps.dwMiscCaps =
		(Caps9.PrimitiveMiscCaps &
			(D3DPMISCCAPS_MASKPLANES |
			D3DPMISCCAPS_MASKZ |
			D3DPMISCCAPS_LINEPATTERNREP |
			D3DPMISCCAPS_CONFORMANT |
			D3DPMISCCAPS_CULLNONE |
			D3DPMISCCAPS_CULLCW |
			D3DPMISCCAPS_CULLCCW));
	Desc7.dpcLineCaps.dwRasterCaps =
		D3DPRASTERCAPS_ROP2 |
		D3DPRASTERCAPS_XOR |
		D3DPRASTERCAPS_PAT |
		D3DPRASTERCAPS_SUBPIXEL |
		D3DPRASTERCAPS_SUBPIXELX |
		D3DPRASTERCAPS_STIPPLE |
		D3DPRASTERCAPS_ANTIALIASSORTDEPENDENT |
		D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT |
		D3DPRASTERCAPS_ANTIALIASEDGES |
		D3DPRASTERCAPS_ZBIAS |
		D3DPRASTERCAPS_TRANSLUCENTSORTINDEPENDENT |
		(Caps9.RasterCaps &
			(D3DPRASTERCAPS_DITHER |
			D3DPRASTERCAPS_ZTEST |
			D3DPRASTERCAPS_FOGVERTEX |
			D3DPRASTERCAPS_FOGTABLE |
			D3DPRASTERCAPS_MIPMAPLODBIAS |
			D3DPRASTERCAPS_ZBUFFERLESSHSR |
			D3DPRASTERCAPS_FOGRANGE |
			D3DPRASTERCAPS_ANISOTROPY |
			D3DPRASTERCAPS_WBUFFER |
			D3DPRASTERCAPS_WFOG |
			D3DPRASTERCAPS_ZFOG));
	Desc7.dpcLineCaps.dwZCmpCaps =
		(Caps9.ZCmpCaps &
			(D3DPCMPCAPS_NEVER |
			D3DPCMPCAPS_LESS |
			D3DPCMPCAPS_EQUAL |
			D3DPCMPCAPS_LESSEQUAL |
			D3DPCMPCAPS_GREATER |
			D3DPCMPCAPS_NOTEQUAL |
			D3DPCMPCAPS_GREATEREQUAL |
			D3DPCMPCAPS_ALWAYS));
	Desc7.dpcLineCaps.dwAlphaCmpCaps =
		(Caps9.AlphaCmpCaps &
			(D3DPCMPCAPS_NEVER |
			D3DPCMPCAPS_LESS |
			D3DPCMPCAPS_EQUAL |
			D3DPCMPCAPS_LESSEQUAL |
			D3DPCMPCAPS_GREATER |
			D3DPCMPCAPS_NOTEQUAL |
			D3DPCMPCAPS_GREATEREQUAL |
			D3DPCMPCAPS_ALWAYS));
	Desc7.dpcLineCaps.dwSrcBlendCaps =
		(Caps9.SrcBlendCaps &
			(D3DPBLENDCAPS_ZERO |
			D3DPBLENDCAPS_ONE |
			D3DPBLENDCAPS_SRCCOLOR |
			D3DPBLENDCAPS_INVSRCCOLOR |
			D3DPBLENDCAPS_SRCALPHA |
			D3DPBLENDCAPS_INVSRCALPHA |
			D3DPBLENDCAPS_DESTALPHA |
			D3DPBLENDCAPS_INVDESTALPHA |
			D3DPBLENDCAPS_DESTCOLOR |
			D3DPBLENDCAPS_INVDESTCOLOR |
			D3DPBLENDCAPS_SRCALPHASAT |
			D3DPBLENDCAPS_BOTHSRCALPHA |
			D3DPBLENDCAPS_BOTHINVSRCALPHA));
	Desc7.dpcLineCaps.dwDestBlendCaps =
		(Caps9.DestBlendCaps &
			(D3DPBLENDCAPS_ZERO |
			D3DPBLENDCAPS_ONE |
			D3DPBLENDCAPS_SRCCOLOR |
			D3DPBLENDCAPS_INVSRCCOLOR |
			D3DPBLENDCAPS_SRCALPHA |
			D3DPBLENDCAPS_INVSRCALPHA |
			D3DPBLENDCAPS_DESTALPHA |
			D3DPBLENDCAPS_INVDESTALPHA |
			D3DPBLENDCAPS_DESTCOLOR |
			D3DPBLENDCAPS_INVDESTCOLOR |
			D3DPBLENDCAPS_SRCALPHASAT |
			D3DPBLENDCAPS_BOTHSRCALPHA |
			D3DPBLENDCAPS_BOTHINVSRCALPHA));
	Desc7.dpcLineCaps.dwShadeCaps =
		D3DPSHADECAPS_COLORFLATMONO |
		D3DPSHADECAPS_COLORFLATRGB |
		D3DPSHADECAPS_COLORGOURAUDMONO |
		D3DPSHADECAPS_COLORPHONGMONO |
		D3DPSHADECAPS_COLORPHONGRGB |
		D3DPSHADECAPS_SPECULARFLATMONO |
		D3DPSHADECAPS_SPECULARFLATRGB |
		D3DPSHADECAPS_SPECULARGOURAUDMONO |
		D3DPSHADECAPS_SPECULARPHONGMONO |
		D3DPSHADECAPS_SPECULARPHONGRGB |
		D3DPSHADECAPS_ALPHAFLATBLEND |
		D3DPSHADECAPS_ALPHAFLATSTIPPLED |
		D3DPSHADECAPS_ALPHAGOURAUDSTIPPLED |
		D3DPSHADECAPS_ALPHAPHONGBLEND |
		D3DPSHADECAPS_ALPHAPHONGSTIPPLED |
		D3DPSHADECAPS_FOGFLAT |
		D3DPSHADECAPS_FOGPHONG |
		(Caps9.ShadeCaps &
			(D3DPSHADECAPS_COLORGOURAUDRGB |
			D3DPSHADECAPS_SPECULARGOURAUDRGB |
			D3DPSHADECAPS_ALPHAGOURAUDBLEND |
			D3DPSHADECAPS_FOGGOURAUD));
	Desc7.dpcLineCaps.dwTextureCaps =
		D3DPTEXTURECAPS_TRANSPARENCY |
		D3DPTEXTURECAPS_BORDER |
		D3DPTEXTURECAPS_COLORKEYBLEND |
		((Caps9.TextureCaps & D3DPTEXTURECAPS_POW2) ? D3DPTEXTURECAPS_POW2 : D3DPTEXTURECAPS_POW2 | D3DPTEXTURECAPS_NONPOW2CONDITIONAL) |
		(Caps9.TextureCaps &
			(D3DPTEXTURECAPS_PERSPECTIVE |
			D3DPTEXTURECAPS_ALPHA |
			D3DPTEXTURECAPS_SQUAREONLY |
			D3DPTEXTURECAPS_TEXREPEATNOTSCALEDBYSIZE |
			D3DPTEXTURECAPS_ALPHAPALETTE |
			D3DPTEXTURECAPS_NONPOW2CONDITIONAL |
			D3DPTEXTURECAPS_PROJECTED |
			D3DPTEXTURECAPS_CUBEMAP));
	Desc7.dpcLineCaps.dwTextureCaps = m_IDirect3D9Ex::AdjustPOW2Caps(Desc7.dpcLineCaps.dwTextureCaps);
	Desc7.dpcLineCaps.dwTextureFilterCaps =
		D3DPTFILTERCAPS_NEAREST |
		D3DPTFILTERCAPS_LINEAR |
		D3DPTFILTERCAPS_MIPNEAREST |
		D3DPTFILTERCAPS_MIPLINEAR |
		D3DPTFILTERCAPS_LINEARMIPNEAREST |
		D3DPTFILTERCAPS_LINEARMIPLINEAR |
		(Caps9.TextureFilterCaps &
			(D3DPTFILTERCAPS_MINFPOINT |
			D3DPTFILTERCAPS_MINFLINEAR |
			D3DPTFILTERCAPS_MINFANISOTROPIC |
			D3DPTFILTERCAPS_MIPFPOINT |
			D3DPTFILTERCAPS_MIPFLINEAR |
			D3DPTFILTERCAPS_MAGFPOINT |
			D3DPTFILTERCAPS_MAGFLINEAR |
			D3DPTFILTERCAPS_MAGFANISOTROPIC |
			D3DPTFILTERCAPS_MAGFAFLATCUBIC |
			D3DPTFILTERCAPS_MAGFGAUSSIANCUBIC));
	Desc7.dpcLineCaps.dwTextureBlendCaps =
		D3DPTBLENDCAPS_DECAL |
		D3DPTBLENDCAPS_MODULATE |
		D3DPTBLENDCAPS_DECALALPHA |
		D3DPTBLENDCAPS_MODULATEALPHA |
		D3DPTBLENDCAPS_DECALMASK |
		D3DPTBLENDCAPS_MODULATEMASK |
		D3DPTBLENDCAPS_COPY |
		D3DPTBLENDCAPS_ADD;
	Desc7.dpcLineCaps.dwTextureAddressCaps =
		(Caps9.TextureAddressCaps &
			(D3DPTADDRESSCAPS_WRAP |
			D3DPTADDRESSCAPS_MIRROR |
			D3DPTADDRESSCAPS_CLAMP |
			D3DPTADDRESSCAPS_BORDER |
			D3DPTADDRESSCAPS_INDEPENDENTUV));
	Desc7.dpcLineCaps.dwStippleWidth = 8;
	Desc7.dpcLineCaps.dwStippleHeight = 8;

	// Triangle capabilities (same as line caps)
	Desc7.dpcTriCaps = Desc7.dpcLineCaps;

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
	Desc7.wMaxTextureBlendStages = (WORD)min(Caps9.MaxTextureBlendStages, D3DHAL_TSS_MAXSTAGES);
	Desc7.wMaxSimultaneousTextures = (WORD)min(Caps9.MaxSimultaneousTextures, D3DHAL_TSS_MAXSTAGES);
	Desc7.dwMaxActiveLights = min(Caps9.MaxActiveLights, MaxActiveLights);
	Desc7.dvMaxVertexW = Caps9.MaxVertexW;
	Desc7.wMaxUserClipPlanes = (WORD)min(Caps9.MaxUserClipPlanes, MaxClipPlaneIndex);
	Desc7.wMaxVertexBlendMatrices = (WORD)min(Caps9.MaxVertexBlendMatrices, USHRT_MAX);

	// Specific settings
	if (Caps9.DeviceType == D3DDEVTYPE_REF || Caps9.DeviceType == D3DDEVTYPE_NULLREF)
	{
		Desc7.deviceGUID = IID_IDirect3DRGBDevice;
		Desc7.dwDevCaps &= ~(D3DDEVCAPS_HWTRANSFORMANDLIGHT | D3DDEVCAPS_HWRASTERIZATION);
		Desc7.dwDeviceRenderBitDepth = DDBD_8 | DDBD_16 | DDBD_24 | DDBD_32;
		Desc7.dwDeviceZBufferBitDepth = DDBD_16 | DDBD_24 | DDBD_32;
	}
	else if (Caps9.DeviceType == D3DDEVTYPE_HAL)
	{
		Desc7.deviceGUID = IID_IDirect3DHALDevice;
		Desc7.dwDevCaps = (Desc7.dwDevCaps & ~(D3DDEVCAPS_HWTRANSFORMANDLIGHT)) | D3DDEVCAPS_HWRASTERIZATION;
		Desc7.dwDeviceRenderBitDepth = DDBD_8 | DDBD_16 | DDBD_32;
		Desc7.dwDeviceZBufferBitDepth = DDBD_16 | DDBD_24 | DDBD_32;
	}
	else if (Caps9.DeviceType == D3DDEVTYPE_TNLHAL)
	{
		Desc7.deviceGUID = IID_IDirect3DTnLHalDevice;
		Desc7.dwDevCaps |= D3DDEVCAPS_HWTRANSFORMANDLIGHT | D3DDEVCAPS_HWRASTERIZATION;
		Desc7.dwDeviceRenderBitDepth = DDBD_8 | DDBD_16 | DDBD_32;
		Desc7.dwDeviceZBufferBitDepth = DDBD_16 | DDBD_24 | DDBD_32;
	}
}

void ConvertLVertex(DXLVERTEX7* lFVF7, const DXLVERTEX9* lFVF9, DWORD NumVertices)
{
	for (UINT x = 0; x < NumVertices; x++)
	{
		lFVF7[x].xyz = lFVF9[x].xyz;
		lFVF7[x].ctuv = lFVF9[x].ctuv;
	}
}

void ConvertLVertex(DXLVERTEX9* lFVF9, const DXLVERTEX7* lFVF7, DWORD NumVertices)
{
	for (UINT x = 0; x < NumVertices; x++)
	{
		lFVF9[x].xyz = lFVF7[x].xyz;
		lFVF9[x].ctuv = lFVF7[x].ctuv;
	}
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

const int o0_0f = 0x00000000; // float(0.0f) as int
const int o1_0f = 0x3F800000; // float(1.0f) as int
struct XYZ_16 { int x, y, z, w; };

#define DEFINE_XYZ_STRUCT(size) \
	struct XYZ_##size { int x, y, z, w; DWORD a[((size - 16) / 4)]; };

DEFINE_XYZ_STRUCT(20)
DEFINE_XYZ_STRUCT(24)
DEFINE_XYZ_STRUCT(28)
DEFINE_XYZ_STRUCT(32)
DEFINE_XYZ_STRUCT(36)
DEFINE_XYZ_STRUCT(40)
DEFINE_XYZ_STRUCT(44)
DEFINE_XYZ_STRUCT(48)
DEFINE_XYZ_STRUCT(52)
DEFINE_XYZ_STRUCT(56)
DEFINE_XYZ_STRUCT(60)
DEFINE_XYZ_STRUCT(64)
DEFINE_XYZ_STRUCT(68)
DEFINE_XYZ_STRUCT(72)
DEFINE_XYZ_STRUCT(76)
DEFINE_XYZ_STRUCT(80)
DEFINE_XYZ_STRUCT(84)
DEFINE_XYZ_STRUCT(88)
DEFINE_XYZ_STRUCT(92)
DEFINE_XYZ_STRUCT(96)
DEFINE_XYZ_STRUCT(100)
DEFINE_XYZ_STRUCT(104)
DEFINE_XYZ_STRUCT(108)
DEFINE_XYZ_STRUCT(112)
DEFINE_XYZ_STRUCT(116)
DEFINE_XYZ_STRUCT(120)
DEFINE_XYZ_STRUCT(124)
DEFINE_XYZ_STRUCT(128)
DEFINE_XYZ_STRUCT(132)
DEFINE_XYZ_STRUCT(136)
DEFINE_XYZ_STRUCT(140)
DEFINE_XYZ_STRUCT(144)
DEFINE_XYZ_STRUCT(148)
DEFINE_XYZ_STRUCT(152)

template <typename T>
void ClampVerticesX(T* pDestVertex, DWORD dwNumVertices)
{
	for (DWORD x = 0; x < dwNumVertices; x++)
	{
		pDestVertex[x].z = min(pDestVertex[x].z, o1_0f);
	}
}

void ClampVertices(BYTE* pVertexData, DWORD Stride, DWORD dwNumVertices)
{
	if (!Config.DdrawClampVertexZDepth)
	{
		return;
	}
	if (Stride == 16) ClampVerticesX(reinterpret_cast<XYZ_16*>(pVertexData), dwNumVertices);
	else if (Stride == 20) ClampVerticesX(reinterpret_cast<XYZ_20*>(pVertexData), dwNumVertices);
	else if (Stride == 24) ClampVerticesX(reinterpret_cast<XYZ_24*>(pVertexData), dwNumVertices);
	else if (Stride == 28) ClampVerticesX(reinterpret_cast<XYZ_28*>(pVertexData), dwNumVertices);
	else if (Stride == 32) ClampVerticesX(reinterpret_cast<XYZ_32*>(pVertexData), dwNumVertices);
	else if (Stride == 36) ClampVerticesX(reinterpret_cast<XYZ_36*>(pVertexData), dwNumVertices);
	else if (Stride == 40) ClampVerticesX(reinterpret_cast<XYZ_40*>(pVertexData), dwNumVertices);
	else if (Stride == 44) ClampVerticesX(reinterpret_cast<XYZ_44*>(pVertexData), dwNumVertices);
	else if (Stride == 48) ClampVerticesX(reinterpret_cast<XYZ_48*>(pVertexData), dwNumVertices);
	else if (Stride == 52) ClampVerticesX(reinterpret_cast<XYZ_52*>(pVertexData), dwNumVertices);
	else if (Stride == 56) ClampVerticesX(reinterpret_cast<XYZ_56*>(pVertexData), dwNumVertices);
	else if (Stride == 60) ClampVerticesX(reinterpret_cast<XYZ_60*>(pVertexData), dwNumVertices);
	else if (Stride == 64) ClampVerticesX(reinterpret_cast<XYZ_64*>(pVertexData), dwNumVertices);
	else if (Stride == 68) ClampVerticesX(reinterpret_cast<XYZ_68*>(pVertexData), dwNumVertices);
	else if (Stride == 72) ClampVerticesX(reinterpret_cast<XYZ_72*>(pVertexData), dwNumVertices);
	else if (Stride == 76) ClampVerticesX(reinterpret_cast<XYZ_76*>(pVertexData), dwNumVertices);
	else if (Stride == 80) ClampVerticesX(reinterpret_cast<XYZ_80*>(pVertexData), dwNumVertices);
	else if (Stride == 84) ClampVerticesX(reinterpret_cast<XYZ_84*>(pVertexData), dwNumVertices);
	else if (Stride == 88) ClampVerticesX(reinterpret_cast<XYZ_88*>(pVertexData), dwNumVertices);
	else if (Stride == 92) ClampVerticesX(reinterpret_cast<XYZ_92*>(pVertexData), dwNumVertices);
	else if (Stride == 96) ClampVerticesX(reinterpret_cast<XYZ_96*>(pVertexData), dwNumVertices);
	else if (Stride == 100) ClampVerticesX(reinterpret_cast<XYZ_100*>(pVertexData), dwNumVertices);
	else if (Stride == 104) ClampVerticesX(reinterpret_cast<XYZ_104*>(pVertexData), dwNumVertices);
	else if (Stride == 108) ClampVerticesX(reinterpret_cast<XYZ_108*>(pVertexData), dwNumVertices);
	else if (Stride == 112) ClampVerticesX(reinterpret_cast<XYZ_112*>(pVertexData), dwNumVertices);
	else if (Stride == 116) ClampVerticesX(reinterpret_cast<XYZ_116*>(pVertexData), dwNumVertices);
	else if (Stride == 120) ClampVerticesX(reinterpret_cast<XYZ_120*>(pVertexData), dwNumVertices);
	else if (Stride == 124) ClampVerticesX(reinterpret_cast<XYZ_124*>(pVertexData), dwNumVertices);
	else if (Stride == 128) ClampVerticesX(reinterpret_cast<XYZ_128*>(pVertexData), dwNumVertices);
	else if (Stride == 132) ClampVerticesX(reinterpret_cast<XYZ_132*>(pVertexData), dwNumVertices);
	else if (Stride == 136) ClampVerticesX(reinterpret_cast<XYZ_136*>(pVertexData), dwNumVertices);
	else if (Stride == 140) ClampVerticesX(reinterpret_cast<XYZ_140*>(pVertexData), dwNumVertices);
	else if (Stride == 144) ClampVerticesX(reinterpret_cast<XYZ_144*>(pVertexData), dwNumVertices);
	else if (Stride == 148) ClampVerticesX(reinterpret_cast<XYZ_148*>(pVertexData), dwNumVertices);
	else if (Stride == 152) ClampVerticesX(reinterpret_cast<XYZ_152*>(pVertexData), dwNumVertices);
	else
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Vertex buffer stride not supported: " << Stride);
	}
}

void ConvertVertex(BYTE* pDestVertex, DWORD DestFVF, const BYTE* pSrcVertex, DWORD SrcFVF)
{
	DWORD SrcOffset = 0;
	DWORD DestOffset = 0;

	DWORD SrcPosFVF = (SrcFVF & D3DFVF_POSITION_MASK);
	DWORD DestPosFVF = (DestFVF & D3DFVF_POSITION_MASK);

	// Get number of blending weights
	DWORD SrcNumBlending = GetBlendCount(SrcPosFVF);
	DWORD DestNumBlending = GetBlendCount(DestPosFVF);

	// Copy Position XYZ
	if (SrcPosFVF && DestPosFVF)
	{
		*(D3DXVECTOR3*)pDestVertex = *(D3DXVECTOR3*)pSrcVertex;
	}

	// Copy Position RHW
	if (DestPosFVF == D3DFVF_XYZRHW && DestPosFVF == SrcPosFVF)
	{
		*(float*)(pDestVertex + sizeof(D3DXVECTOR3)) = *(float*)(pSrcVertex + sizeof(D3DXVECTOR3));
	}

	// Copy matching blending weights
	if (SrcNumBlending && DestNumBlending)
	{
		memcpy(pDestVertex + sizeof(D3DXVECTOR3), pSrcVertex + sizeof(D3DXVECTOR3), min(SrcNumBlending, DestNumBlending) * sizeof(float));
	}

	// Update source offset for Position
	SrcOffset += GetVertexPositionStride(SrcPosFVF);

	// Update destination offset for Position
	DestOffset += GetVertexPositionStride(DestPosFVF);

	// Reserved
	if (DestFVF & D3DFVF_RESERVED1)
	{
		if (SrcFVF & D3DFVF_RESERVED1)
		{
			SrcOffset += sizeof(DWORD);
		}
		DestOffset += sizeof(DWORD);
	}
	else if (SrcFVF & D3DFVF_RESERVED1)
	{
		SrcOffset += sizeof(DWORD);
	}

	// Normal
	if (DestFVF & D3DFVF_NORMAL)
	{
		if (SrcFVF & D3DFVF_NORMAL)
		{
			*(D3DXVECTOR3*)(pDestVertex + DestOffset) = *(D3DXVECTOR3*)(pSrcVertex + SrcOffset);
			SrcOffset += sizeof(D3DXVECTOR3);
		}
		DestOffset += sizeof(D3DXVECTOR3);
	}
	else if (SrcFVF & D3DFVF_NORMAL)
	{
		SrcOffset += sizeof(D3DXVECTOR3);
	}

	// Diffuse color
	if (DestFVF & D3DFVF_DIFFUSE)
	{
		if (SrcFVF & D3DFVF_DIFFUSE)
		{
			*(D3DCOLOR*)(pDestVertex + DestOffset) = *(D3DCOLOR*)(pSrcVertex + SrcOffset);
			SrcOffset += sizeof(D3DCOLOR);
		}
		DestOffset += sizeof(D3DCOLOR);
	}
	else if (SrcFVF & D3DFVF_DIFFUSE)
	{
		SrcOffset += sizeof(D3DCOLOR);
	}

	// Specular color
	if (DestFVF & D3DFVF_SPECULAR)
	{
		if (SrcFVF & D3DFVF_SPECULAR)
		{
			*(D3DCOLOR*)(pDestVertex + DestOffset) = *(D3DCOLOR*)(pSrcVertex + SrcOffset);
			SrcOffset += sizeof(D3DCOLOR);
		}
		DestOffset += sizeof(D3DCOLOR);
	}
	else if (SrcFVF & D3DFVF_SPECULAR)
	{
		SrcOffset += sizeof(D3DCOLOR);
	}

	// Texture coordinates
	int SrcNumTexCoords = D3DFVF_TEXCOUNT(SrcFVF);
	int DestNumTexCoords = D3DFVF_TEXCOUNT(DestFVF);

	if (SrcNumTexCoords > D3DDP_MAXTEXCOORD || DestNumTexCoords > D3DDP_MAXTEXCOORD)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: texCount " << SrcNumTexCoords << " -> " << DestNumTexCoords << " exceeds D3DDP_MAXTEXCOORD!");
		return;
	}

	int y = 0;
	for (int x = 0; x < DestNumTexCoords; x++)
	{
		int DestTexStride = GetTexStride(DestFVF, x);

		// Find matching source texture coordinates
		while (y < SrcNumTexCoords)
		{
			int SrcTexStride = GetTexStride(SrcFVF, y);

			// Copy matching texture coordinates
			if (DestTexStride && DestTexStride == SrcTexStride)
			{
				memcpy(pDestVertex + DestOffset, pSrcVertex + SrcOffset, DestTexStride);
				SrcOffset += SrcTexStride;
				y++;
				break;
			}
			SrcOffset += SrcTexStride;
			y++;
		}
		// Increase destination offset
		DestOffset += DestTexStride;
	}
}

DWORD ConvertVertexTypeToFVF(D3DVERTEXTYPE d3dVertexType)
{
	switch (d3dVertexType)
	{
	case D3DVT_VERTEX:
		return D3DFVF_VERTEX;
	case D3DVT_LVERTEX:
		return D3DFVF_LVERTEX;
	case D3DVT_TLVERTEX:
		return D3DFVF_TLVERTEX;
	}
	return 0;
}

bool IsValidFVF(DWORD dwVertexTypeDesc)
{
	// Must specify position format
	DWORD posType = dwVertexTypeDesc & D3DFVF_POSITION_MASK;
	if (posType != D3DFVF_XYZ &&
		posType != D3DFVF_XYZRHW &&
		(posType < D3DFVF_XYZB1 || posType > D3DFVF_XYZB5))
	{
		return false;
	}

	// Reject reserved/invalid bits (only allow known FVF bits)
	if (dwVertexTypeDesc & ~D3DFVF_SUPPORTED_BIT_MASK)
	{
		return false;
	}

	// Reject if XYZRHW and reserved1 or normal are set
	if ((dwVertexTypeDesc & D3DFVF_XYZRHW) && (dwVertexTypeDesc & (D3DFVF_RESERVED1 | D3DFVF_NORMAL)))
	{
		return false;
	}

	// Validate texture count is 0–8
	DWORD texCount = D3DFVF_TEXCOUNT(dwVertexTypeDesc);
	if (texCount > D3DDP_MAXTEXCOORD)
	{
		return false;
	}

	// Validate each texture coordinate size
	for (DWORD t = 0; t < texCount; ++t)
	{
		DWORD sizeFlag = (dwVertexTypeDesc >> (16 + t * 2)) & 0x3;
		if (sizeFlag > 3) return false;
	}

	return true;
}

UINT GetBlendCount(DWORD dwVertexTypeDesc)
{
	switch (dwVertexTypeDesc & D3DFVF_POSITION_MASK)
	{
	case D3DFVF_XYZB1:   return 1;
	case D3DFVF_XYZB2:   return 2;
	case D3DFVF_XYZB3:   return 3;
	case D3DFVF_XYZB4:   return 4;
	case D3DFVF_XYZB5:   return 5;
	}
	return 0;
}

UINT GetVertexPositionStride(DWORD dwVertexTypeDesc)
{
	switch (dwVertexTypeDesc & D3DFVF_POSITION_MASK)
	{
	case D3DFVF_XYZ:     return sizeof(float) * 3;
	case D3DFVF_XYZRHW:  return sizeof(float) * 4;
	case D3DFVF_XYZB1:   return sizeof(float) * 4;
	case D3DFVF_XYZB2:   return sizeof(float) * 5;
	case D3DFVF_XYZB3:   return sizeof(float) * 6;
	case D3DFVF_XYZB4:   return sizeof(float) * 6 + sizeof(DWORD);
	case D3DFVF_XYZB5:   return sizeof(float) * 7 + sizeof(DWORD);
	}
	return 0;
}

UINT GetTexStride(DWORD dwVertexTypeDesc, DWORD t)
{
	DWORD sizeFlag = (dwVertexTypeDesc >> (16 + t * 2)) & 0x3;

	switch (sizeFlag)
	{
	default:
	case 0:   return sizeof(float) * 2;
	case 1:   return sizeof(float) * 1;
	case 2:   return sizeof(float) * 3;
	case 3:   return sizeof(float) * 4;
	}
}

UINT GetVertexTextureStride(DWORD dwVertexTypeDesc)
{
	const DWORD texCount = D3DFVF_TEXCOUNT(dwVertexTypeDesc);

	if (texCount > D3DDP_MAXTEXCOORD)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: texCount " << texCount << " exceeds D3DDP_MAXTEXCOORD!");
		return 0;
	}

	// Compute tex stride
	DWORD texStride = 0;
	for (DWORD t = 0; t < texCount; ++t)
	{
		texStride += GetTexStride(dwVertexTypeDesc, t);
	}

	return texStride;
}

UINT GetVertexStride(DWORD dwVertexTypeDesc)
{
	// Reserved:
	// #define D3DFVF_RESERVED0        0x001  // (DX7)
	// #define D3DFVF_RESERVED0        0x001  // (DX9)

	// #define D3DFVF_RESERVED1        0x020  // (DX7)
	// #define D3DFVF_PSIZE            0x020  // (DX9)

	// #define D3DFVF_RESERVED2        0xf000  // 4 reserved bits (DX7)
	// #define D3DFVF_RESERVED2        0x6000  // 2 reserved bits (DX9)

	// Check for unsupported vertex types
	const DWORD UnSupportedVertexTypes = dwVertexTypeDesc & ~D3DFVF_SUPPORTED_BIT_MASK;
	if (UnSupportedVertexTypes)
	{
		LOG_LIMIT(100, __FUNCTION__ " Warning: Unsupported FVF type: " << Logging::hex(UnSupportedVertexTypes));
	}

	return
		GetVertexPositionStride(dwVertexTypeDesc) +
		((dwVertexTypeDesc & D3DFVF_RESERVED1) ? sizeof(DWORD) : 0) +
		((dwVertexTypeDesc & D3DFVF_NORMAL) ? sizeof(float) * 3 : 0) +
		((dwVertexTypeDesc & D3DFVF_DIFFUSE) ? sizeof(D3DCOLOR) : 0) +
		((dwVertexTypeDesc & D3DFVF_SPECULAR) ? sizeof(D3DCOLOR) : 0) +
		GetVertexTextureStride(dwVertexTypeDesc);
}

UINT GetNumberOfPrimitives(D3DPRIMITIVETYPE dptPrimitiveType, DWORD dwVertexCount)
{
	if (!dptPrimitiveType || dptPrimitiveType > 6)
	{
		LOG_LIMIT(100, __FUNCTION__ " Warning: Unsupported primitive type: " << dptPrimitiveType);
	}

	return
		(dptPrimitiveType == D3DPT_POINTLIST) ? dwVertexCount :
		(dptPrimitiveType == D3DPT_LINELIST) ? dwVertexCount / 2 :
		(dptPrimitiveType == D3DPT_LINESTRIP) ? dwVertexCount - 1 :
		(dptPrimitiveType == D3DPT_TRIANGLELIST) ? dwVertexCount / 3 :
		(dptPrimitiveType == D3DPT_TRIANGLESTRIP) ? dwVertexCount - 2 :
		(dptPrimitiveType == D3DPT_TRIANGLEFAN) ? dwVertexCount - 2 :
		0;
}
