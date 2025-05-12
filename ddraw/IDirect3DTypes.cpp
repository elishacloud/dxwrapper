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

void ConvertLight(D3DLIGHT7& Light7, const D3DLIGHT& Light)
{
	if (Light.dwSize != sizeof(D3DLIGHT) && Light.dwSize != sizeof(D3DLIGHT2))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << Light.dwSize);
		return;
	}
	const float DiffuseFactor = 1.0f;
	const float SpecularFactor = 0.1f;
	const float AmbientFactor = 0.3f;
	Light7.dltType = Light.dltType;
	Light7.dcvDiffuse = { Light.dcvColor.r * DiffuseFactor, Light.dcvColor.g * DiffuseFactor, Light.dcvColor.b * DiffuseFactor, Light.dcvColor.a };
	Light7.dcvSpecular = { Light.dcvColor.r * SpecularFactor, Light.dcvColor.g * SpecularFactor, Light.dcvColor.b * SpecularFactor, Light.dcvColor.a };
	Light7.dcvAmbient = { Light.dcvColor.r * AmbientFactor, Light.dcvColor.g * AmbientFactor, Light.dcvColor.b * AmbientFactor, Light.dcvColor.a };
	Light7.dvPosition = Light.dvPosition;
	Light7.dvDirection = Light.dvDirection;
	Light7.dvRange = Light.dvRange;
	Light7.dvFalloff = Light.dvFalloff;
	Light7.dvAttenuation0 = Light.dvAttenuation0;
	Light7.dvAttenuation1 = Light.dvAttenuation1;
	Light7.dvAttenuation2 = Light.dvAttenuation2;
	Light7.dvTheta = Light.dvTheta;
	Light7.dvPhi = Light.dvPhi;

	// Apply additional flags
	if (Light.dwSize == sizeof(D3DLIGHT2) && (((LPD3DLIGHT2)&Light)->dwFlags & D3DLIGHT_NO_SPECULAR))
	{
		// No specular reflection
		Light7.dcvSpecular = { 0.0f, 0.0f, 0.0f, 1.0f };
	}
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

void ConvertViewport(D3DVIEWPORT& ViewPort, const D3DVIEWPORT2& ViewPort2)
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

void ConvertViewport(D3DVIEWPORT2& ViewPort2, const D3DVIEWPORT& ViewPort)
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

void ConvertViewport(D3DVIEWPORT& ViewPort, const D3DVIEWPORT7& ViewPort7)
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

void ConvertViewport(D3DVIEWPORT2& ViewPort2, const D3DVIEWPORT7& ViewPort7)
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

void ConvertViewport(D3DVIEWPORT7& ViewPort7, const D3DVIEWPORT& ViewPort)
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

void ConvertViewport(D3DVIEWPORT7& ViewPort7, const D3DVIEWPORT2& ViewPort2)
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

void ConvertViewport(D3DVIEWPORT7& ViewPort, const D3DVIEWPORT7& ViewPort7)
{
	ViewPort.dwX = ViewPort7.dwX;
	ViewPort.dwY = ViewPort7.dwY;
	ViewPort.dwWidth = ViewPort7.dwWidth;
	ViewPort.dwHeight = ViewPort7.dwHeight;
	ViewPort.dvMinZ = ViewPort7.dvMinZ;
	ViewPort.dvMaxZ = ViewPort7.dvMaxZ;
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
		D3DPTEXTURECAPS_POW2 |
		(Caps9.TextureCaps &
			(D3DPTEXTURECAPS_PERSPECTIVE |
			D3DPTEXTURECAPS_ALPHA |
			D3DPTEXTURECAPS_SQUAREONLY |
			D3DPTEXTURECAPS_TEXREPEATNOTSCALEDBYSIZE |
			D3DPTEXTURECAPS_ALPHAPALETTE |
			D3DPTEXTURECAPS_NONPOW2CONDITIONAL |
			D3DPTEXTURECAPS_PROJECTED |
			D3DPTEXTURECAPS_CUBEMAP));
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
	Desc7.wMaxTextureBlendStages = (WORD)min(Caps9.MaxTextureBlendStages, USHRT_MAX);
	Desc7.wMaxSimultaneousTextures = (WORD)min(Caps9.MaxSimultaneousTextures, USHRT_MAX);
	Desc7.dwMaxActiveLights = Caps9.MaxActiveLights;
	Desc7.dvMaxVertexW = Caps9.MaxVertexW;
	Desc7.wMaxUserClipPlanes = (WORD)min(Caps9.MaxUserClipPlanes, USHRT_MAX);
	Desc7.wMaxVertexBlendMatrices = (WORD)min(Caps9.MaxVertexBlendMatrices, USHRT_MAX);

	// Specific settings
	if (Caps9.DeviceType == D3DDEVTYPE_REF)
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

void ConvertVertices(D3DLVERTEX* lFVF, const D3DLVERTEX9* lFVF9, DWORD NumVertices)
{
	for (UINT x = 0; x < NumVertices; x++)
	{
		lFVF[x].x = lFVF9[x].x;
		lFVF[x].y = lFVF9[x].y;
		lFVF[x].z = lFVF9[x].z;
		lFVF[x].color = lFVF9[x].diffuse;
		lFVF[x].specular = lFVF9[x].specular;
		lFVF[x].tu = lFVF9[x].tu;
		lFVF[x].tv = lFVF9[x].tv;
	}
}

void ConvertVertices(D3DLVERTEX9* lFVF9, const D3DLVERTEX* lFVF, DWORD NumVertices)
{
	for (UINT x = 0; x < NumVertices; x++)
	{
		lFVF9[x].x = lFVF[x].x;
		lFVF9[x].y = lFVF[x].y;
		lFVF9[x].z = lFVF[x].z;
		lFVF9[x].diffuse = lFVF[x].color;
		lFVF9[x].specular = lFVF[x].specular;
		lFVF9[x].tu = lFVF[x].tu;
		lFVF9[x].tv = lFVF[x].tv;
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

void ConvertVertex(BYTE* pDestVertex, DWORD DestFVF, const BYTE* pSrcVertex, DWORD SrcFVF)
{
	DWORD SrcOffset = 0;
	DWORD DestOffset = 0;

	// Copy Position XYZ
	*(D3DXVECTOR3*)pDestVertex = *(D3DXVECTOR3*)pSrcVertex;
	DestOffset += 3 * sizeof(float);

	// Update source offset for Position
	SrcOffset += 3 * sizeof(float);

	// Copy Position data (XYZW, XYZRHW, etc.)
	switch (DestFVF & D3DFVF_POSITION_MASK_9)
	{
	case D3DFVF_XYZW:
	case D3DFVF_XYZRHW:
		if ((DestFVF & D3DFVF_POSITION_MASK_9) == (SrcFVF & D3DFVF_POSITION_MASK_9))
		{
			*(float*)(pDestVertex + DestOffset) = *(float*)(pSrcVertex + SrcOffset);
		}
		DestOffset += sizeof(float);
		break;
	case D3DFVF_XYZB1:
	case D3DFVF_XYZB2:
	case D3DFVF_XYZB3:
	case D3DFVF_XYZB4:
	case D3DFVF_XYZB5:
	{
		// Get number of blending weights
		DWORD SrcNumBlending =
			(SrcFVF & D3DFVF_POSITION_MASK_9) == D3DFVF_XYZB1 ? 1 :
			(SrcFVF & D3DFVF_POSITION_MASK_9) == D3DFVF_XYZB2 ? 2 :
			(SrcFVF & D3DFVF_POSITION_MASK_9) == D3DFVF_XYZB3 ? 3 :
			(SrcFVF & D3DFVF_POSITION_MASK_9) == D3DFVF_XYZB4 ? 4 :
			(SrcFVF & D3DFVF_POSITION_MASK_9) == D3DFVF_XYZB5 ? 5 : 0;
		DWORD DestNumBlending =
			(DestFVF & D3DFVF_POSITION_MASK_9) == D3DFVF_XYZB1 ? 1 :
			(DestFVF & D3DFVF_POSITION_MASK_9) == D3DFVF_XYZB2 ? 2 :
			(DestFVF & D3DFVF_POSITION_MASK_9) == D3DFVF_XYZB3 ? 3 :
			(DestFVF & D3DFVF_POSITION_MASK_9) == D3DFVF_XYZB4 ? 4 :
			(DestFVF & D3DFVF_POSITION_MASK_9) == D3DFVF_XYZB5 ? 5 : 0;
		// Copy matching blending weights
		for (UINT x = 0; x < min(SrcNumBlending, DestNumBlending); x++)
		{
			*(float*)(pDestVertex + DestOffset + x * sizeof(float)) = *(float*)(pSrcVertex + SrcOffset + x * sizeof(float));
		}
		DestOffset += DestNumBlending * sizeof(float);
		break;
	}
	}

	// Update source offset for Position data
	switch (SrcFVF & D3DFVF_POSITION_MASK_9)
	{
	case D3DFVF_XYZW:
	case D3DFVF_XYZRHW:
	case D3DFVF_XYZB1:
		SrcOffset += sizeof(float);
		break;
	case D3DFVF_XYZB2:
		SrcOffset += 2 * sizeof(float);
		break;
	case D3DFVF_XYZB3:
		SrcOffset += 3 * sizeof(float);
		break;
	case D3DFVF_XYZB4:
		SrcOffset += 4 * sizeof(float);
		break;
	case D3DFVF_XYZB5:
		SrcOffset += 5 * sizeof(float);
		break;
	}

	// Normal
	if (DestFVF & D3DFVF_NORMAL)
	{
		if (SrcFVF & D3DFVF_NORMAL)
		{
			*(D3DXVECTOR3*)(pDestVertex + DestOffset) = *(D3DXVECTOR3*)(pSrcVertex + SrcOffset);
			SrcOffset += 3 * sizeof(float);
		}
		DestOffset += 3 * sizeof(float);
	}
	else if (SrcFVF & D3DFVF_NORMAL)
	{
		SrcOffset += 3 * sizeof(float);
	}

	// Point Size
	if (DestFVF & D3DFVF_PSIZE)
	{
		if (SrcFVF & D3DFVF_PSIZE)
		{
			*(float*)(pDestVertex + DestOffset) = *(float*)(pSrcVertex + SrcOffset);
			SrcOffset += sizeof(float);
		}
		DestOffset += sizeof(float);
	}
	else if (SrcFVF & D3DFVF_PSIZE)
	{
		SrcOffset += sizeof(float);
	}

	// Diffuse color
	if (DestFVF & D3DFVF_DIFFUSE)
	{
		if (SrcFVF & D3DFVF_DIFFUSE)
		{
			*(DWORD*)(pDestVertex + DestOffset) = *(DWORD*)(pSrcVertex + SrcOffset);
			SrcOffset += sizeof(DWORD);
		}
		DestOffset += sizeof(DWORD);
	}
	else if (SrcFVF & D3DFVF_DIFFUSE)
	{
		SrcOffset += sizeof(DWORD);
	}

	// Specular color
	if (DestFVF & D3DFVF_SPECULAR)
	{
		if (SrcFVF & D3DFVF_SPECULAR)
		{
			*(DWORD*)(pDestVertex + DestOffset) = *(DWORD*)(pSrcVertex + SrcOffset);
			SrcOffset += sizeof(DWORD);
		}
		DestOffset += sizeof(DWORD);
	}
	else if (SrcFVF & D3DFVF_SPECULAR)
	{
		SrcOffset += sizeof(DWORD);
	}

	// Texture coordinates
	int SrcNumTexCoords = (SrcFVF & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;
	int DestNumTexCoords = (DestFVF & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;
	int y = 0;
	for (int x = 0; x < DestNumTexCoords; x++)
	{
		// Get number of destination texture coordinates
		int DestCord = (DestFVF & (D3DFVF_TEXCOORDSIZE1(x) | D3DFVF_TEXCOORDSIZE2(x) | D3DFVF_TEXCOORDSIZE3(x) | D3DFVF_TEXCOORDSIZE4(x)));
		int DestSize =
			DestCord == D3DFVF_TEXCOORDSIZE1(x) ? 1 :
			DestCord == D3DFVF_TEXCOORDSIZE2(x) ? 2 :
			DestCord == D3DFVF_TEXCOORDSIZE3(x) ? 3 :
			DestCord == D3DFVF_TEXCOORDSIZE4(x) ? 4 : 0;
		// Find matching source texture coordinates
		while (y < SrcNumTexCoords)
		{
			int SrcCord = (SrcFVF & (D3DFVF_TEXCOORDSIZE1(y) | D3DFVF_TEXCOORDSIZE2(y) | D3DFVF_TEXCOORDSIZE3(y) | D3DFVF_TEXCOORDSIZE4(y)));
			int SrcSize =
				SrcCord == D3DFVF_TEXCOORDSIZE1(y) ? 1 :
				SrcCord == D3DFVF_TEXCOORDSIZE2(y) ? 2 :
				SrcCord == D3DFVF_TEXCOORDSIZE3(y) ? 3 :
				SrcCord == D3DFVF_TEXCOORDSIZE4(y) ? 4 : 0;
			// Copy matching texture coordinates
			if (SrcSize && DestSize == SrcSize)
			{
				for (int i = 0; i < SrcSize; i++)
				{
					*(float*)(pDestVertex + DestOffset + i * sizeof(float)) = *(float*)(pSrcVertex + SrcOffset + i * sizeof(float));
				}
				SrcOffset += SrcSize * sizeof(float);
				y++;
				break;
			}
			SrcOffset += SrcSize * sizeof(float);
			y++;
		}
		// Increase destination offset
		DestOffset += DestSize * sizeof(float);
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
	DWORD UnSupportedVertexTypes = dwVertexTypeDesc & ~(D3DFVF_POSITION_MASK_9 | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEXCOUNT_MASK);
	if (UnSupportedVertexTypes)
	{
		LOG_LIMIT(100, __FUNCTION__ " Warning: Unsupported FVF type: " << Logging::hex(UnSupportedVertexTypes));
	}

	return
		(((dwVertexTypeDesc & D3DFVF_POSITION_MASK) == D3DFVF_XYZ) ? sizeof(float) * 3 : 0) +
		(((dwVertexTypeDesc & D3DFVF_POSITION_MASK) == D3DFVF_XYZRHW) ? sizeof(float) * 4 : 0) +
		(((dwVertexTypeDesc & D3DFVF_POSITION_MASK) == D3DFVF_XYZB1) ? sizeof(float) * 4 : 0) +
		(((dwVertexTypeDesc & D3DFVF_POSITION_MASK) == D3DFVF_XYZB2) ? sizeof(float) * 5 : 0) +
		(((dwVertexTypeDesc & D3DFVF_POSITION_MASK) == D3DFVF_XYZB3) ? sizeof(float) * 6 : 0) +
		(((dwVertexTypeDesc & D3DFVF_POSITION_MASK) == D3DFVF_XYZB4) ? sizeof(float) * 6 + sizeof(DWORD) : 0) +
		(((dwVertexTypeDesc & D3DFVF_POSITION_MASK) == D3DFVF_XYZB5) ? sizeof(float) * 7 + sizeof(DWORD) : 0) +
		((dwVertexTypeDesc & D3DFVF_XYZW & ~D3DFVF_XYZ) ? sizeof(float) : 0) +
		((dwVertexTypeDesc & D3DFVF_NORMAL) ? sizeof(float) * 3 : 0) +
		((dwVertexTypeDesc & D3DFVF_DIFFUSE) ? sizeof(D3DCOLOR) : 0) +
		((dwVertexTypeDesc & D3DFVF_SPECULAR) ? sizeof(D3DCOLOR) : 0) +
		(((dwVertexTypeDesc & D3DFVF_TEXCOUNT_MASK) == D3DFVF_TEX1) ? sizeof(float) * 2 : 0) +
		(((dwVertexTypeDesc & D3DFVF_TEXCOUNT_MASK) == D3DFVF_TEX2) ? sizeof(float) * 4 : 0) +
		(((dwVertexTypeDesc & D3DFVF_TEXCOUNT_MASK) == D3DFVF_TEX3) ? sizeof(float) * 6 : 0) +
		(((dwVertexTypeDesc & D3DFVF_TEXCOUNT_MASK) == D3DFVF_TEX4) ? sizeof(float) * 8 : 0) +
		(((dwVertexTypeDesc & D3DFVF_TEXCOUNT_MASK) == D3DFVF_TEX5) ? sizeof(float) * 10 : 0) +
		(((dwVertexTypeDesc & D3DFVF_TEXCOUNT_MASK) == D3DFVF_TEX6) ? sizeof(float) * 12 : 0) +
		(((dwVertexTypeDesc & D3DFVF_TEXCOUNT_MASK) == D3DFVF_TEX7) ? sizeof(float) * 14 : 0) +
		(((dwVertexTypeDesc & D3DFVF_TEXCOUNT_MASK) == D3DFVF_TEX8) ? sizeof(float) * 16 : 0) +
		0;
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
