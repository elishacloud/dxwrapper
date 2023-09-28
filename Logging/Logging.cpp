/**
* Copyright (C) 2023 Elisha Riedlinger
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

#define INITGUID
#define DIRECTINPUT_VERSION 0x0800

#include <d3d9.h>
#include <d3d9types.h>
#include <ddraw.h>
#include <ddrawex.h>
#include <d3d.h>
#include <d3dhal.h>
#include "ddraw\IDirectDrawTypes.h"
#include <dinput.h>
#include <dsound.h>
#include <MMSystem.h>
#include <mmstream.h>
#include <amstream.h>
#include <ddstream.h>
#include <mmdeviceapi.h>
#include "IClassFactory\IClassFactory.h"
#include "Logging.h"

std::ofstream LOG;

// Get wrapper file name
void Logging::InitLog()
{
	if (!EnableLogging)
	{
		return;
	}

	static char wrappername[MAX_PATH];

	// Get module name
	HMODULE hModule = NULL;
	GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCTSTR)InitLog, &hModule);
	GetModuleFileName(hModule, wrappername, MAX_PATH);

	// Get process name
	char processname[MAX_PATH];
	GetModuleFileName(nullptr, processname, MAX_PATH);

	// Check if module name is the same as process name
	if (_stricmp(strrchr(wrappername, '\\') + 1, strrchr(processname, '\\') + 1) == 0)
	{
		strcpy_s(strrchr(wrappername, '\\') + 1, MAX_PATH - strlen(wrappername), "dxwrapper.dll");
	}

	// Remove extension and add dash (-)
	strcpy_s(strrchr(wrappername, '.'), MAX_PATH - strlen(wrappername), "-");

	// Add process name
	strcat_s(wrappername, MAX_PATH, strrchr(processname, '\\') + 1);

	// Change extension to .log
	strcpy_s(strrchr(wrappername, '.'), MAX_PATH - strlen(wrappername), ".log");

	// Set lower case
	for (int z = 0; z < MAX_PATH && wrappername[z] != '\0'; z++) { wrappername[z] = (char)tolower(wrappername[z]); }

	Open(wrappername);
}

std::ostream& operator<<(std::ostream& os, const D3DFORMAT& format)
{
	switch ((DWORD)format)
	{
	case 0:
		return os << "D3DFMT_UNKNOWN";
	case D3DFMT_B8G8R8:
		return os << "D3DFMT_B8G8R8";
	case 20:
		return os << "D3DFMT_R8G8B8";
	case 21:
		return os << "D3DFMT_A8R8G8B8";
	case 22:
		return os << "D3DFMT_X8R8G8B8";
	case 23:
		return os << "D3DFMT_R5G6B5";
	case 24:
		return os << "D3DFMT_X1R5G5B5";
	case 25:
		return os << "D3DFMT_A1R5G5B5";
	case 26:
		return os << "D3DFMT_A4R4G4B4";
	case 27:
		return os << "D3DFMT_R3G3B2";
	case 28:
		return os << "D3DFMT_A8";
	case 29:
		return os << "D3DFMT_A8R3G3B2";
	case 30:
		return os << "D3DFMT_X4R4G4B4";
	case 31:
		return os << "D3DFMT_A2B10G10R10";
	case 32:
		return os << "D3DFMT_A8B8G8R8";
	case 33:
		return os << "D3DFMT_X8B8G8R8";
	case 34:
		return os << "D3DFMT_G16R16";
	case 35:
		return os << "D3DFMT_A2R10G10B10";
	case 36:
		return os << "D3DFMT_A16B16G16R16";
	case 40:
		return os << "D3DFMT_A8P8";
	case 41:
		return os << "D3DFMT_P8";
	case 50:
		return os << "D3DFMT_L8";
	case 51:
		return os << "D3DFMT_A8L8";
	case 52:
		return os << "D3DFMT_A4L4";
	case 60:
		return os << "D3DFMT_V8U8";
	case 61:
		return os << "D3DFMT_L6V5U5";
	case 62:
		return os << "D3DFMT_X8L8V8U8";
	case 63:
		return os << "D3DFMT_Q8W8V8U8";
	case 64:
		return os << "D3DFMT_V16U16";
	case 67:
		return os << "D3DFMT_A2W10V10U10";
	case MAKEFOURCC('U', 'Y', 'V', 'Y'):
		return os << "D3DFMT_UYVY";
	case MAKEFOURCC('R', 'G', 'B', 'G'):
		return os << "D3DFMT_R8G8_B8G8";
	case MAKEFOURCC('Y', 'U', 'Y', '2'):
		return os << "D3DFMT_YUY2";
	case MAKEFOURCC('Y', 'V', '1', '2'):
		return os << "D3DFMT_YV12";
	case MAKEFOURCC('A', 'Y', 'U', 'V'):
		return os << "D3DFMT_AYUV";
	case MAKEFOURCC('G', 'R', 'G', 'B'):
		return os << "D3DFMT_G8R8_G8B8";
	case MAKEFOURCC('D', 'X', 'T', '1'):
		return os << "D3DFMT_DXT1";
	case MAKEFOURCC('D', 'X', 'T', '2'):
		return os << "D3DFMT_DXT2";
	case MAKEFOURCC('D', 'X', 'T', '3'):
		return os << "D3DFMT_DXT3";
	case MAKEFOURCC('D', 'X', 'T', '4'):
		return os << "D3DFMT_DXT4";
	case MAKEFOURCC('D', 'X', 'T', '5'):
		return os << "D3DFMT_DXT5";
	case 70:
		return os << "D3DFMT_D16_LOCKABLE";
	case 71:
		return os << "D3DFMT_D32";
	case 72:
		return os << "D3DFMT_S1D15";
	case 73:
		return os << "D3DFMT_D15S1";
	case 74:
		return os << "D3DFMT_S8D24";
	case 75:
		return os << "D3DFMT_D24S8";
	case 76:
		return os << "D3DFMT_X8D24";
	case 77:
		return os << "D3DFMT_D24X8";
	case 78:
		return os << "D3DFMT_X4S4D24";
	case 79:
		return os << "D3DFMT_D24X4S4";
	case 80:
		return os << "D3DFMT_D16";
	case 82:
		return os << "D3DFMT_D32F_LOCKABLE";
	case 83:
		return os << "D3DFMT_D24FS8";
	case 84:
		return os << "D3DFMT_D32_LOCKABLE";
	case 85:
		return os << "D3DFMT_S8_LOCKABLE";
	case 81:
		return os << "D3DFMT_L16";
	case 100:
		return os << "D3DFMT_VERTEXDATA";
	case 101:
		return os << "D3DFMT_INDEX16";
	case 102:
		return os << "D3DFMT_INDEX32";
	case 110:
		return os << "D3DFMT_Q16W16V16U16";
	case MAKEFOURCC('M', 'E', 'T', '1'):
		return os << "D3DFMT_MULTI2_ARGB8";
	case 111:
		return os << "D3DFMT_R16F";
	case 112:
		return os << "D3DFMT_G16R16F";
	case 113:
		return os << "D3DFMT_A16B16G16R16F";
	case 114:
		return os << "D3DFMT_R32F";
	case 115:
		return os << "D3DFMT_G32R32F";
	case 116:
		return os << "D3DFMT_A32B32G32R32F";
	case 117:
		return os << "D3DFMT_CxV8U8";
	case 118:
		return os << "D3DFMT_A1";
	case 119:
		return os << "D3DFMT_A2B10G10R10_XR_BIAS";
	case 199:
		return os << "D3DFMT_BINARYBUFFER";
	case 0x7fffffff:
		return os << "D3DFMT_FORCE_DWORD";
	default:
		if (format & 0xFF000000)
		{
			return os << (DDFOURCC)format;
		}
	}

	return os << (DWORD)format;
}

std::ostream& operator<<(std::ostream& os, const D3DRESOURCETYPE& Resource)
{
	switch (Resource)
	{
	case D3DRTYPE_SURFACE:
		return os << "D3DRTYPE_SURFACE";
	case D3DRTYPE_VOLUME:
		return os << "D3DRTYPE_VOLUME";
	case D3DRTYPE_TEXTURE:
		return os << "D3DRTYPE_TEXTURE";
	case D3DRTYPE_VOLUMETEXTURE:
		return os << "D3DRTYPE_VOLUMETEXTURE";
	case D3DRTYPE_CUBETEXTURE:
		return os << "D3DRTYPE_CUBETEXTURE";
	case D3DRTYPE_VERTEXBUFFER:
		return os << "D3DRTYPE_VERTEXBUFFER";
	case D3DRTYPE_INDEXBUFFER:
		return os << "D3DRTYPE_INDEXBUFFER";
	case D3DRTYPE_FORCE_DWORD:
		return os << "D3DRTYPE_FORCE_DWORD";
	}

	return os << (DWORD)Resource;
}

std::ostream& operator<<(std::ostream& os, const DDCAPS& cp)
{
	DDCAPS caps = {};
	memcpy(&caps, &cp, min(sizeof(DDCAPS), cp.dwSize));
	return Logging::LogStruct(os)
		<< Logging::hex(caps.dwCaps)
		<< Logging::hex(caps.dwCaps2)
		<< Logging::hex(caps.dwCKeyCaps)
		<< Logging::hex(caps.dwFXCaps)
		<< Logging::hex(caps.dwFXAlphaCaps)
		<< Logging::hex(caps.dwPalCaps)
		<< Logging::hex(caps.dwSVCaps)
		<< caps.dwAlphaBltConstBitDepths
		<< caps.dwAlphaBltPixelBitDepths
		<< caps.dwAlphaBltSurfaceBitDepths
		<< caps.dwAlphaOverlayConstBitDepths
		<< caps.dwAlphaOverlayPixelBitDepths
		<< caps.dwAlphaOverlaySurfaceBitDepths
		<< caps.dwZBufferBitDepths
		<< caps.dwVidMemTotal
		<< caps.dwVidMemFree
		<< caps.dwMaxVisibleOverlays
		<< caps.dwCurrVisibleOverlays
		<< caps.dwNumFourCCCodes
		<< caps.dwAlignBoundarySrc
		<< caps.dwAlignSizeSrc
		<< caps.dwAlignBoundaryDest
		<< caps.dwAlignSizeDest
		<< caps.dwAlignStrideAlign
		<< "{" << caps.dwRops[0]
		<< caps.dwRops[1]
		<< caps.dwRops[2]
		<< caps.dwRops[3]
		<< caps.dwRops[4]
		<< caps.dwRops[5]
		<< caps.dwRops[6]
		<< caps.dwRops[7] << "}"
		<< caps.ddsOldCaps
		<< caps.dwMinOverlayStretch
		<< caps.dwMaxOverlayStretch
		<< caps.dwMinLiveVideoStretch
		<< caps.dwMaxLiveVideoStretch
		<< caps.dwMinHwCodecStretch
		<< caps.dwMaxHwCodecStretch
		<< caps.dwReserved1
		<< caps.dwReserved2
		<< caps.dwReserved3
		<< Logging::hex(caps.dwSVBCaps)
		<< Logging::hex(caps.dwSVBCKeyCaps)
		<< Logging::hex(caps.dwSVBFXCaps)
		<< "{" << caps.dwSVBRops[0]
		<< caps.dwSVBRops[1]
		<< caps.dwSVBRops[2]
		<< caps.dwSVBRops[3]
		<< caps.dwSVBRops[4]
		<< caps.dwSVBRops[5]
		<< caps.dwSVBRops[6]
		<< caps.dwSVBRops[7] << "}"
		<< Logging::hex(caps.dwVSBCaps)
		<< Logging::hex(caps.dwVSBCKeyCaps)
		<< Logging::hex(caps.dwVSBFXCaps)
		<< "{" << caps.dwVSBRops[0]
		<< caps.dwVSBRops[1]
		<< caps.dwVSBRops[2]
		<< caps.dwVSBRops[3]
		<< caps.dwVSBRops[4]
		<< caps.dwVSBRops[5]
		<< caps.dwVSBRops[6]
		<< caps.dwVSBRops[7] << "}"
		<< Logging::hex(caps.dwSSBCaps)
		<< Logging::hex(caps.dwSSBCKeyCaps)
		<< Logging::hex(caps.dwSSBFXCaps)
		<< "{" << caps.dwSSBRops[0]
		<< caps.dwSSBRops[1]
		<< caps.dwSSBRops[2]
		<< caps.dwSSBRops[3]
		<< caps.dwSSBRops[4]
		<< caps.dwSSBRops[5]
		<< caps.dwSSBRops[6]
		<< caps.dwSSBRops[7] << "}"
		<< caps.dwMaxVideoPorts
		<< caps.dwCurrVideoPorts
		<< Logging::hex(caps.dwSVBCaps2)
		<< Logging::hex(caps.dwNLVBCaps)
		<< Logging::hex(caps.dwNLVBCaps2)
		<< Logging::hex(caps.dwNLVBCKeyCaps)
		<< Logging::hex(caps.dwNLVBFXCaps)
		<< "{" << caps.dwNLVBRops[0]
		<< caps.dwNLVBRops[1]
		<< caps.dwNLVBRops[2]
		<< caps.dwNLVBRops[3]
		<< caps.dwNLVBRops[4]
		<< caps.dwNLVBRops[5]
		<< caps.dwNLVBRops[6]
		<< caps.dwNLVBRops[7] << "}"
		<< caps.ddsCaps;
}

std::ostream& operator<<(std::ostream& os, const DDSCAPS& dwCaps)
{
	return Logging::LogStructNull(os)
		<< ((dwCaps.dwCaps & DDSCAPS_3DDEVICE) ? " DDSCAPS_3DDEVICE " : "")
		<< ((dwCaps.dwCaps & DDSCAPS_ALLOCONLOAD) ? " DDSCAPS_ALLOCONLOAD " : "")
		<< ((dwCaps.dwCaps & DDSCAPS_ALPHA) ? " DDSCAPS_ALPHA " : "")
		<< ((dwCaps.dwCaps & DDSCAPS_BACKBUFFER) ? " DDSCAPS_BACKBUFFER " : "")
		<< ((dwCaps.dwCaps & DDSCAPS_COMPLEX) ? " DDSCAPS_COMPLEX " : "")
		<< ((dwCaps.dwCaps & DDSCAPS_FLIP) ? " DDSCAPS_FLIP " : "")
		<< ((dwCaps.dwCaps & DDSCAPS_FRONTBUFFER) ? " DDSCAPS_FRONTBUFFER " : "")
		<< ((dwCaps.dwCaps & DDSCAPS_HWCODEC) ? " DDSCAPS_HWCODEC " : "")
		<< ((dwCaps.dwCaps & DDSCAPS_LIVEVIDEO) ? " DDSCAPS_LIVEVIDEO " : "")
		<< ((dwCaps.dwCaps & DDSCAPS_LOCALVIDMEM) ? " DDSCAPS_LOCALVIDMEM " : "")
		<< ((dwCaps.dwCaps & DDSCAPS_MIPMAP) ? " DDSCAPS_MIPMAP " : "")
		<< ((dwCaps.dwCaps & DDSCAPS_MODEX) ? " DDSCAPS_MODEX " : "")
		<< ((dwCaps.dwCaps & DDSCAPS_NONLOCALVIDMEM) ? " DDSCAPS_NONLOCALVIDMEM " : "")
		<< ((dwCaps.dwCaps & DDSCAPS_OFFSCREENPLAIN) ? " DDSCAPS_OFFSCREENPLAIN " : "")
		<< ((dwCaps.dwCaps & DDSCAPS_OPTIMIZED) ? " DDSCAPS_OPTIMIZED " : "")
		<< ((dwCaps.dwCaps & DDSCAPS_OVERLAY) ? " DDSCAPS_OVERLAY " : "")
		<< ((dwCaps.dwCaps & DDSCAPS_OWNDC) ? " DDSCAPS_OWNDC " : "")
		<< ((dwCaps.dwCaps & DDSCAPS_PALETTE) ? " DDSCAPS_PALETTE " : "")
		<< ((dwCaps.dwCaps & DDSCAPS_PRIMARYSURFACE) ? " DDSCAPS_PRIMARYSURFACE " : "")
		<< ((dwCaps.dwCaps & DDSCAPS_STANDARDVGAMODE) ? " DDSCAPS_STANDARDVGAMODE " : "")
		<< ((dwCaps.dwCaps & DDSCAPS_SYSTEMMEMORY) ? " DDSCAPS_SYSTEMMEMORY " : "")
		<< ((dwCaps.dwCaps & DDSCAPS_TEXTURE) ? " DDSCAPS_TEXTURE " : "")
		<< ((dwCaps.dwCaps & DDSCAPS_VIDEOMEMORY) ? " DDSCAPS_VIDEOMEMORY " : "")
		<< ((dwCaps.dwCaps & DDSCAPS_VIDEOPORT) ? " DDSCAPS_VIDEOPORT " : "")
		<< ((dwCaps.dwCaps & DDSCAPS_VISIBLE) ? " DDSCAPS_VISIBLE " : "")
		<< ((dwCaps.dwCaps & DDSCAPS_WRITEONLY) ? " DDSCAPS_WRITEONLY " : "")
		<< ((dwCaps.dwCaps & DDSCAPS_ZBUFFER) ? " DDSCAPS_ZBUFFER " : "");
}

std::ostream& operator<<(std::ostream& os, const DDSCAPS2& caps)
{
	return Logging::LogStruct(os)
		<< *(DDSCAPS*)&caps
		<< Logging::hex(caps.dwCaps2)
		<< Logging::hex(caps.dwCaps3)
		<< Logging::hex(caps.dwCaps4);
}

std::ostream& operator<<(std::ostream& os, const DDFOURCC& dwFourCC)
{
	if (dwFourCC)
	{
		unsigned char ch0 = dwFourCC & 0xFF;
		unsigned char ch1 = (dwFourCC >> 8) & 0xFF;
		unsigned char ch2 = (dwFourCC >> 16) & 0xFF;
		unsigned char ch3 = (dwFourCC >> 24) & 0xFF;

		return os << "MAKEFOURCC('" << (char)ch0 << "', '" << (char)ch1 << "', '" << (char)ch2 << "', '" << (char)ch3 << "')";
	}

	return os << (DWORD)dwFourCC;
}

std::ostream& operator<<(std::ostream& os, const DDPIXELFORMAT& pf)
{
	return Logging::LogStruct(os)
		<< Logging::hex(pf.dwFlags)
		<< (DDFOURCC)pf.dwFourCC
		<< pf.dwRGBBitCount
		<< Logging::hex(pf.dwRBitMask)
		<< Logging::hex(pf.dwGBitMask)
		<< Logging::hex(pf.dwBBitMask)
		<< Logging::hex(pf.dwRGBAlphaBitMask);
}

std::ostream& operator<<(std::ostream& os, const DDCOLORKEY& ck)
{
	return Logging::LogStruct(os)
		<< Logging::hex(ck.dwColorSpaceLowValue)
		<< Logging::hex(ck.dwColorSpaceHighValue);
}

std::ostream& operator<<(std::ostream& os, const DDSURFACEDESC& sd)
{
	return Logging::LogStruct(os)
		<< Logging::hex(sd.dwFlags)
		<< sd.dwWidth
		<< sd.dwHeight
		<< sd.lPitch
		<< sd.dwBackBufferCount
		<< sd.dwMipMapCount
		<< sd.dwAlphaBitDepth
		<< sd.dwReserved
		<< sd.lpSurface
		<< sd.ddckCKDestOverlay
		<< sd.ddckCKDestBlt
		<< sd.ddckCKSrcOverlay
		<< sd.ddckCKSrcBlt
		<< sd.ddpfPixelFormat
		<< sd.ddsCaps;
}

std::ostream& operator<<(std::ostream& os, const DDSURFACEDESC2& sd)
{
	return Logging::LogStruct(os)
		<< Logging::hex(sd.dwFlags)
		<< sd.dwWidth
		<< sd.dwHeight
		<< sd.lPitch
		<< sd.dwBackBufferCount
		<< sd.dwMipMapCount
		<< sd.dwAlphaBitDepth
		<< sd.dwReserved
		<< sd.lpSurface
		<< sd.ddckCKDestOverlay
		<< sd.ddckCKDestBlt
		<< sd.ddckCKSrcOverlay
		<< sd.ddckCKSrcBlt
		<< sd.ddpfPixelFormat
		<< sd.dwFVF
		<< sd.ddsCaps
		<< sd.dwTextureStage;
}
std::ostream& operator<<(std::ostream& os, const D3DCOLORVALUE& data)
{
	return Logging::LogStruct(os)
		<< data.r
		<< data.g
		<< data.b
		<< data.a;
}

std::ostream& operator<<(std::ostream& os, const D3DDP_PTRSTRIDE& data)
{
	return Logging::LogStruct(os)
		<< data.lpvData
		<< data.dwStride;
}

std::ostream& operator<<(std::ostream& os, const D3DDRAWPRIMITIVESTRIDEDDATA& data)
{
	return Logging::LogStruct(os)
		<< data.position
		<< data.normal
		<< data.diffuse
		<< data.specular
		<< Logging::array(data.textureCoords, D3DDP_MAXTEXCOORD);
}

std::ostream& operator<<(std::ostream& os, const D3DEXECUTEBUFFERDESC& data)
{
	return Logging::LogStruct(os)
		<< Logging::hex(data.dwFlags)
		<< Logging::hex(data.dwCaps)
		<< data.dwBufferSize
		<< data.lpData;
}

std::ostream& operator<<(std::ostream& os, const D3DEXECUTEDATA& data)
{
	return Logging::LogStruct(os)
		<< data.dwVertexOffset
		<< data.dwVertexCount
		<< data.dwInstructionOffset
		<< data.dwInstructionLength
		<< data.dwHVertexOffset
		<< data.dsStatus;
}

std::ostream& operator<<(std::ostream& os, const D3DLIGHT& data)
{
	D3DLIGHT2 light = {};
	reinterpret_cast<D3DLIGHT&>(light) = data;
	return os << light;
}

std::ostream& operator<<(std::ostream& os, const D3DLIGHT2& data)
{
	return Logging::LogStruct(os)
		<< data.dltType
		<< data.dcvColor
		<< data.dvPosition
		<< data.dvDirection
		<< data.dvRange
		<< data.dvFalloff
		<< data.dvAttenuation0
		<< data.dvAttenuation1
		<< data.dvAttenuation2
		<< data.dvTheta
		<< data.dvPhi
		<< Logging::hex(data.dwFlags);
}

std::ostream& operator<<(std::ostream& os, const D3DLIGHT7& data)
{
	return Logging::LogStruct(os)
		<< data.dltType
		<< data.dcvDiffuse
		<< data.dcvSpecular
		<< data.dcvAmbient
		<< data.dvPosition
		<< data.dvDirection
		<< data.dvRange
		<< data.dvFalloff
		<< data.dvAttenuation0
		<< data.dvAttenuation1
		<< data.dvAttenuation2
		<< data.dvTheta
		<< data.dvPhi;
}

std::ostream& operator<<(std::ostream& os, const D3DMATERIAL& data)
{
	return Logging::LogStruct(os)
		<< data.diffuse
		<< data.ambient
		<< data.specular
		<< data.emissive
		<< data.power
		<< Logging::hex(data.hTexture)
		<< data.dwRampSize;
}

std::ostream& operator<<(std::ostream& os, const D3DMATERIAL7& data)
{
	D3DMATERIAL material = {};
	reinterpret_cast<D3DMATERIAL7&>(material) = data;
	return os << material;
}

std::ostream& operator<<(std::ostream& os, const D3DRECT& data)
{
	return Logging::LogStruct(os)
		<< data.x1
		<< data.y1
		<< data.x2
		<< data.y2;
}

std::ostream& operator<<(std::ostream& os, const D3DSTATUS& data)
{
	return Logging::LogStruct(os)
		<< Logging::hex(data.dwFlags)
		<< Logging::hex(data.dwStatus)
		<< data.drExtent;
}

std::ostream& operator<<(std::ostream& os, const D3DCLIPSTATUS& data)
{
	return Logging::LogStruct(os)
		<< Logging::hex(data.dwFlags)
		<< Logging::hex(data.dwStatus)
		<< data.minx
		<< data.maxx
		<< data.miny
		<< data.maxy
		<< data.minz
		<< data.maxz;
}

std::ostream& operator<<(std::ostream& os, const D3DVERTEXBUFFERDESC& data)
{
	return Logging::LogStruct(os)
		<< data.dwSize
		<< Logging::hex(data.dwCaps)
		<< Logging::hex(data.dwFVF)
		<< data.dwNumVertices;
}

std::ostream& operator<<(std::ostream& os, const D3DPRIMCAPS& dpc)
{
	return Logging::LogStruct(os)
		<< dpc.dwSize
		<< Logging::hex(dpc.dwMiscCaps)
		<< Logging::hex(dpc.dwRasterCaps)
		<< Logging::hex(dpc.dwZCmpCaps)
		<< Logging::hex(dpc.dwSrcBlendCaps)
		<< Logging::hex(dpc.dwDestBlendCaps)
		<< Logging::hex(dpc.dwAlphaCmpCaps)
		<< Logging::hex(dpc.dwShadeCaps)
		<< Logging::hex(dpc.dwTextureCaps)
		<< Logging::hex(dpc.dwTextureFilterCaps)
		<< Logging::hex(dpc.dwTextureBlendCaps)
		<< Logging::hex(dpc.dwTextureAddressCaps)
		<< dpc.dwStippleWidth
		<< dpc.dwStippleHeight;
}

std::ostream& operator<<(std::ostream& os, const D3DTRANSFORMCAPS& tc)
{
	return Logging::LogStruct(os)
		<< tc.dwSize
		<< Logging::hex(tc.dwCaps);
}

std::ostream& operator<<(std::ostream& os, const D3DLIGHTINGCAPS& lc)
{
	return Logging::LogStruct(os)
		<< lc.dwSize
		<< Logging::hex(lc.dwCaps)
		<< lc.dwLightingModel
		<< lc.dwNumLights;
}

std::ostream& operator<<(std::ostream& os, const D3DDEVICEDESC& dd)
{
	return Logging::LogStruct(os)
		<< dd.dwSize
		<< Logging::hex(dd.dwFlags)
		<< dd.dcmColorModel
		<< Logging::hex(dd.dwDevCaps)
		<< dd.dtcTransformCaps
		<< dd.bClipping
		<< dd.dlcLightingCaps
		<< dd.dpcLineCaps
		<< dd.dpcTriCaps
		<< dd.dwDeviceRenderBitDepth
		<< dd.dwDeviceZBufferBitDepth
		<< dd.dwMaxBufferSize
		<< dd.dwMaxVertexCount
		<< dd.dwMinTextureWidth
		<< dd.dwMinTextureHeight
		<< dd.dwMaxTextureWidth
		<< dd.dwMaxTextureHeight
		<< dd.dwMinStippleWidth
		<< dd.dwMinStippleHeight
		<< dd.dwMaxStippleWidth
		<< dd.dwMaxStippleHeight
		<< dd.dwMaxTextureRepeat
		<< dd.dwMaxTextureAspectRatio
		<< dd.dwMaxAnisotropy
		<< dd.dvGuardBandLeft
		<< dd.dvGuardBandTop
		<< dd.dvGuardBandRight
		<< dd.dvGuardBandBottom
		<< dd.dvExtentsAdjust
		<< Logging::hex(dd.dwStencilCaps)
		<< Logging::hex(dd.dwFVFCaps)
		<< Logging::hex(dd.dwTextureOpCaps)
		<< dd.wMaxTextureBlendStages
		<< dd.wMaxSimultaneousTextures;
}

std::ostream& operator<<(std::ostream& os, const D3DDEVICEDESC7& dd)
{
	return Logging::LogStruct(os)
		<< Logging::hex(dd.dwDevCaps)
		<< dd.dpcLineCaps
		<< dd.dpcTriCaps
		<< dd.dwDeviceRenderBitDepth
		<< dd.dwDeviceZBufferBitDepth
		<< dd.dwMinTextureWidth
		<< dd.dwMinTextureHeight
		<< dd.dwMaxTextureWidth
		<< dd.dwMaxTextureHeight
		<< dd.dwMaxTextureRepeat
		<< dd.dwMaxTextureAspectRatio
		<< dd.dwMaxAnisotropy
		<< dd.dvGuardBandLeft
		<< dd.dvGuardBandTop
		<< dd.dvGuardBandRight
		<< dd.dvGuardBandBottom
		<< dd.dvExtentsAdjust
		<< Logging::hex(dd.dwStencilCaps)
		<< Logging::hex(dd.dwFVFCaps)
		<< Logging::hex(dd.dwTextureOpCaps)
		<< dd.wMaxTextureBlendStages
		<< dd.wMaxSimultaneousTextures
		<< dd.dwMaxActiveLights
		<< dd.dvMaxVertexW
		<< dd.deviceGUID
		<< dd.wMaxUserClipPlanes
		<< dd.wMaxVertexBlendMatrices
		<< Logging::hex(dd.dwVertexProcessingCaps)
		<< dd.dwReserved1
		<< dd.dwReserved2
		<< dd.dwReserved3
		<< dd.dwReserved4;
}

std::ostream& operator<<(std::ostream& os, const D3DPRESENT_PARAMETERS& pp)
{
	return Logging::LogStruct(os)
		<< pp.BackBufferWidth
		<< pp.BackBufferHeight
		<< pp.BackBufferFormat
		<< pp.BackBufferCount
		<< pp.MultiSampleType
		<< pp.MultiSampleQuality
		<< pp.SwapEffect
		<< pp.hDeviceWindow
		<< pp.Windowed
		<< pp.EnableAutoDepthStencil
		<< pp.AutoDepthStencilFormat
		<< Logging::hex(pp.Flags)
		<< pp.FullScreen_RefreshRateInHz
		<< Logging::hex(pp.PresentationInterval);
}

#ifndef _D3D8_H_
DEFINE_GUID(IID_IDirect3D8, 0x1dd9e8da, 0x1c77, 0x4d40, 0xb0, 0xcf, 0x98, 0xfe, 0xfd, 0xff, 0x95, 0x12);
DEFINE_GUID(IID_IDirect3DDevice8, 0x7385e5df, 0x8fe8, 0x41d5, 0x86, 0xb6, 0xd7, 0xb4, 0x85, 0x47, 0xb6, 0xcf);
DEFINE_GUID(IID_IDirect3DResource8, 0x1b36bb7b, 0x9b7, 0x410a, 0xb4, 0x45, 0x7d, 0x14, 0x30, 0xd7, 0xb3, 0x3f);
DEFINE_GUID(IID_IDirect3DBaseTexture8, 0xb4211cfa, 0x51b9, 0x4a9f, 0xab, 0x78, 0xdb, 0x99, 0xb2, 0xbb, 0x67, 0x8e);
DEFINE_GUID(IID_IDirect3DTexture8, 0xe4cdd575, 0x2866, 0x4f01, 0xb1, 0x2e, 0x7e, 0xec, 0xe1, 0xec, 0x93, 0x58);
DEFINE_GUID(IID_IDirect3DCubeTexture8, 0x3ee5b968, 0x2aca, 0x4c34, 0x8b, 0xb5, 0x7e, 0x0c, 0x3d, 0x19, 0xb7, 0x50);
DEFINE_GUID(IID_IDirect3DVolumeTexture8, 0x4b8aaafa, 0x140f, 0x42ba, 0x91, 0x31, 0x59, 0x7e, 0xaf, 0xaa, 0x2e, 0xad);
DEFINE_GUID(IID_IDirect3DVertexBuffer8, 0x8aeeeac7, 0x05f9, 0x44d4, 0xb5, 0x91, 0x00, 0x0b, 0x0d, 0xf1, 0xcb, 0x95);
DEFINE_GUID(IID_IDirect3DIndexBuffer8, 0x0e689c9a, 0x053d, 0x44a0, 0x9d, 0x92, 0xdb, 0x0e, 0x3d, 0x75, 0x0f, 0x86);
DEFINE_GUID(IID_IDirect3DSurface8, 0xb96eebca, 0xb326, 0x4ea5, 0x88, 0x2f, 0x2f, 0xf5, 0xba, 0xe0, 0x21, 0xdd);
DEFINE_GUID(IID_IDirect3DVolume8, 0xbd7349f5, 0x14f1, 0x42e4, 0x9c, 0x79, 0x97, 0x23, 0x80, 0xdb, 0x40, 0xc0);
DEFINE_GUID(IID_IDirect3DSwapChain8, 0x928c088b, 0x76b9, 0x4c6b, 0xa5, 0x36, 0xa5, 0x90, 0x85, 0x38, 0x76, 0xcd);
#endif
// DirectShow
DEFINE_GUID(IID_IAMMultiMediaStream, 0xbebe595c, 0x9a6f, 0x11d0, 0x8f, 0xde, 0x00, 0xc0, 0x4f, 0xd9, 0x18, 0x9d);
DEFINE_GUID(IID_IAMMediaStream, 0xbebe595d, 0x9a6f, 0x11d0, 0x8f, 0xde, 0x00, 0xc0, 0x4f, 0xd9, 0x18, 0x9d);
DEFINE_GUID(IID_IMediaStreamFilter, 0xbebe595e, 0x9a6f, 0x11d0, 0x8f, 0xde, 0x00, 0xc0, 0x4f, 0xd9, 0x18, 0x9d);
DEFINE_GUID(IID_IDirectDrawMediaSampleAllocator, 0xab6b4afc, 0xf6e4, 0x11d0, 0x90, 0x0d, 0x00, 0xc0, 0x4f, 0xd9, 0x18, 0x9d);
DEFINE_GUID(IID_IDirectDrawMediaSample, 0xab6b4afe, 0xf6e4, 0x11d0, 0x90, 0x0d, 0x00, 0xc0, 0x4f, 0xd9, 0x18, 0x9d);
DEFINE_GUID(IID_IAMMediaTypeStream, 0xab6b4afa, 0xf6e4, 0x11d0, 0x90, 0x0d, 0x00, 0xc0, 0x4f, 0xd9, 0x18, 0x9d);
DEFINE_GUID(IID_IAMMediaTypeSample, 0xab6b4afb, 0xf6e4, 0x11d0, 0x90, 0x0d, 0x00, 0xc0, 0x4f, 0xd9, 0x18, 0x9d);
DEFINE_GUID(IID_IDirectDrawStreamSample, 0xf4104fcf, 0x9a70, 0x11d0, 0x8f, 0xde, 0x00, 0xc0, 0x4f, 0xd9, 0x18, 0x9d);
DEFINE_GUID(IID_IDirectDrawMediaStream, 0xf4104fce, 0x9a70, 0x11d0, 0x8f, 0xde, 0x00, 0xc0, 0x4f, 0xd9, 0x18, 0x9d);
DEFINE_GUID(IID_IDDVideoAcceleratorContainer, 0xACA12120, 0x3356, 0x11D1, 0x8F, 0xCF, 0x00, 0xC0, 0x4F, 0xC2, 0x9B, 0x4E);
DEFINE_GUID(IID_IDirectDrawVideoAccelerator, 0xC9B2D740, 0x3356, 0x11D1, 0x8F, 0xCF, 0x00, 0xC0, 0x4F, 0xC2, 0x9B, 0x4E);
DEFINE_GUID(CLSID_MMDeviceEnumerator, 0xbcde0395, 0xe52f, 0x467c, 0x8e, 0x3d, 0xc4, 0x57, 0x92, 0x91, 0x69, 0x2e);
DEFINE_GUID(IID_IMMNotificationClient, 0x7991eec9, 0x7e89, 0x4d85, 0x83, 0x90, 0x6c, 0x70, 0x3c, 0xec, 0x60, 0xc0);
DEFINE_GUID(IID_IMMDevice, 0xd666063f, 0x1587, 0x4e43, 0x81, 0xf1, 0xb9, 0x48, 0xe8, 0x07, 0x36, 0x3f);
DEFINE_GUID(IID_IMMDeviceCollection, 0x0bd7a1be, 0x7a1a, 0x44db, 0x83, 0x97, 0xcc, 0x53, 0x92, 0x38, 0x7b, 0x5e);
DEFINE_GUID(IID_IMMEndpoint, 0x1be09788, 0x6894, 0x4089, 0x85, 0x86, 0x9a, 0x2a, 0x6c, 0x26, 0x5a, 0xc5);
DEFINE_GUID(IID_IMMDeviceEnumerator, 0xa95664d2, 0x9614, 0x4f35, 0xa7, 0x46, 0xde, 0x8d, 0xb6, 0x36, 0x17, 0xe6);
DEFINE_GUID(IID_IMMDeviceActivator, 0x3b0d0ea4, 0xd0a9, 0x4b0e, 0x93, 0x5b, 0x09, 0x51, 0x67, 0x46, 0xfa, 0xc0);
DEFINE_GUID(IID_IActivateAudioInterfaceCompletionHandler, 0x41d949ab, 0x9862, 0x444a, 0x80, 0xf6, 0xc2, 0x61, 0x33, 0x4d, 0xa5, 0xeb);
DEFINE_GUID(IID_IActivateAudioInterfaceAsyncOperation, 0x72a22d78, 0xcde4, 0x431d, 0xb8, 0xcc, 0x84, 0x3a, 0x71, 0x19, 0x9b, 0x6d);
DEFINE_GUID(IID_IMultiMediaStream, 0xb502d1bc, 0x9a57, 0x11d0, 0x8f, 0xde, 0x00, 0xc0, 0x4f, 0xd9, 0x18, 0x9d);
DEFINE_GUID(IID_IMediaStream, 0xb502d1bd, 0x9a57, 0x11d0, 0x8f, 0xde, 0x00, 0xc0, 0x4f, 0xd9, 0x18, 0x9d);
DEFINE_GUID(IID_IStreamSample, 0xb502d1be, 0x9a57, 0x11d0, 0x8f, 0xde, 0x00, 0xc0, 0x4f, 0xd9, 0x18, 0x9d);

std::ostream& operator<<(std::ostream& os, REFIID riid)
{
#define CHECK_REFIID(riidPrefix, riidName) \
	if (riid == riidPrefix ## _ ## riidName) \
	{ \
		return os << #riidPrefix << "_" << #riidName; \
	}

	CHECK_REFIID(IID, IUnknown);
	CHECK_REFIID(IID, IClassFactory);
	// ddraw
	CHECK_REFIID(CLSID, DirectDraw);
	CHECK_REFIID(CLSID, DirectDraw7);
	CHECK_REFIID(CLSID, DirectDrawClipper);
	CHECK_REFIID(IID, IDirectDraw);
	CHECK_REFIID(IID, IDirectDraw2);
	CHECK_REFIID(IID, IDirectDraw4);
	CHECK_REFIID(IID, IDirectDraw7);
	CHECK_REFIID(IID, IDirectDrawSurface);
	CHECK_REFIID(IID, IDirectDrawSurface2);
	CHECK_REFIID(IID, IDirectDrawSurface3);
	CHECK_REFIID(IID, IDirectDrawSurface4);
	CHECK_REFIID(IID, IDirectDrawSurface7);
	CHECK_REFIID(IID, IDirectDrawPalette);
	CHECK_REFIID(IID, IDirectDrawClipper);
	CHECK_REFIID(IID, IDirectDrawColorControl);
	CHECK_REFIID(IID, IDirectDrawGammaControl);
	// ddrawex
	CHECK_REFIID(IID, IDirectDraw3);
	CHECK_REFIID(CLSID, DirectDrawFactory);
	CHECK_REFIID(IID, IDirectDrawFactory);
	// d3d
	CHECK_REFIID(IID, IDirect3D);
	CHECK_REFIID(IID, IDirect3D2);
	CHECK_REFIID(IID, IDirect3D3);
	CHECK_REFIID(IID, IDirect3D7);
	CHECK_REFIID(IID, IDirect3DRampDevice);
	CHECK_REFIID(IID, IDirect3DRGBDevice);
	CHECK_REFIID(IID, IDirect3DHALDevice);
	CHECK_REFIID(IID, IDirect3DMMXDevice);
	CHECK_REFIID(IID, IDirect3DRefDevice);
	CHECK_REFIID(IID, IDirect3DNullDevice);
	CHECK_REFIID(IID, IDirect3DTnLHalDevice);
	CHECK_REFIID(IID, IDirect3DDevice);
	CHECK_REFIID(IID, IDirect3DDevice2);
	CHECK_REFIID(IID, IDirect3DDevice3);
	CHECK_REFIID(IID, IDirect3DDevice7);
	CHECK_REFIID(IID, IDirect3DTexture);
	CHECK_REFIID(IID, IDirect3DTexture2);
	CHECK_REFIID(IID, IDirect3DLight);
	CHECK_REFIID(IID, IDirect3DMaterial);
	CHECK_REFIID(IID, IDirect3DMaterial2);
	CHECK_REFIID(IID, IDirect3DMaterial3);
	CHECK_REFIID(IID, IDirect3DExecuteBuffer);
	CHECK_REFIID(IID, IDirect3DViewport);
	CHECK_REFIID(IID, IDirect3DViewport2);
	CHECK_REFIID(IID, IDirect3DViewport3);
	CHECK_REFIID(IID, IDirect3DVertexBuffer);
	CHECK_REFIID(IID, IDirect3DVertexBuffer7);
	// d3d8
	CHECK_REFIID(IID, IDirect3D8);
	CHECK_REFIID(IID, IDirect3DDevice8);
	CHECK_REFIID(IID, IDirect3DResource8);
	CHECK_REFIID(IID, IDirect3DBaseTexture8);
	CHECK_REFIID(IID, IDirect3DTexture8);
	CHECK_REFIID(IID, IDirect3DCubeTexture8);
	CHECK_REFIID(IID, IDirect3DVolumeTexture8);
	CHECK_REFIID(IID, IDirect3DVertexBuffer8);
	CHECK_REFIID(IID, IDirect3DIndexBuffer8);
	CHECK_REFIID(IID, IDirect3DSurface8);
	CHECK_REFIID(IID, IDirect3DVolume8);
	CHECK_REFIID(IID, IDirect3DSwapChain8);
	// d3d9
	CHECK_REFIID(IID, IDirect3D9);
	CHECK_REFIID(IID, IDirect3DDevice9);
	CHECK_REFIID(IID, IDirect3DResource9);
	CHECK_REFIID(IID, IDirect3DBaseTexture9);
	CHECK_REFIID(IID, IDirect3DTexture9);
	CHECK_REFIID(IID, IDirect3DCubeTexture9);
	CHECK_REFIID(IID, IDirect3DVolumeTexture9);
	CHECK_REFIID(IID, IDirect3DVertexBuffer9);
	CHECK_REFIID(IID, IDirect3DIndexBuffer9);
	CHECK_REFIID(IID, IDirect3DSurface9);
	CHECK_REFIID(IID, IDirect3DVolume9);
	CHECK_REFIID(IID, IDirect3DSwapChain9);
	CHECK_REFIID(IID, IDirect3DVertexDeclaration9);
	CHECK_REFIID(IID, IDirect3DVertexShader9);
	CHECK_REFIID(IID, IDirect3DPixelShader9);
	CHECK_REFIID(IID, IDirect3DStateBlock9);
	CHECK_REFIID(IID, IDirect3DQuery9);
	CHECK_REFIID(IID, HelperName);
	CHECK_REFIID(IID, IDirect3D9Ex);
	CHECK_REFIID(IID, IDirect3DDevice9Ex);
	CHECK_REFIID(IID, IDirect3DSwapChain9Ex);
	CHECK_REFIID(IID, IDirect3D9ExOverlayExtension);
	CHECK_REFIID(IID, IDirect3DDevice9Video);
	CHECK_REFIID(IID, IDirect3DAuthenticatedChannel9);
	CHECK_REFIID(IID, IDirect3DCryptoSession9);
	// dinput
	CHECK_REFIID(CLSID, DirectInput);
	CHECK_REFIID(CLSID, DirectInputDevice);
	CHECK_REFIID(CLSID, DirectInput8);
	CHECK_REFIID(CLSID, DirectInputDevice8);
	CHECK_REFIID(IID, IDirectInputA);
	CHECK_REFIID(IID, IDirectInputW);
	CHECK_REFIID(IID, IDirectInput2A);
	CHECK_REFIID(IID, IDirectInput2W);
	CHECK_REFIID(IID, IDirectInput7A);
	CHECK_REFIID(IID, IDirectInput7W);
	CHECK_REFIID(IID, IDirectInput8A);
	CHECK_REFIID(IID, IDirectInput8W);
	CHECK_REFIID(IID, IDirectInputDeviceA);
	CHECK_REFIID(IID, IDirectInputDeviceW);
	CHECK_REFIID(IID, IDirectInputDevice2A);
	CHECK_REFIID(IID, IDirectInputDevice2W);
	CHECK_REFIID(IID, IDirectInputDevice7A);
	CHECK_REFIID(IID, IDirectInputDevice7W);
	CHECK_REFIID(IID, IDirectInputDevice8A);
	CHECK_REFIID(IID, IDirectInputDevice8W);
	CHECK_REFIID(IID, IDirectInputEffect);
	// Predefined object types
	CHECK_REFIID(GUID, XAxis);
	CHECK_REFIID(GUID, YAxis);
	CHECK_REFIID(GUID, ZAxis);
	CHECK_REFIID(GUID, RxAxis);
	CHECK_REFIID(GUID, RyAxis);
	CHECK_REFIID(GUID, RzAxis);
	CHECK_REFIID(GUID, Slider);
	CHECK_REFIID(GUID, Button);
	CHECK_REFIID(GUID, Key);
	CHECK_REFIID(GUID, POV);
	CHECK_REFIID(GUID, Unknown);
	// Predefined product GUIDs
	CHECK_REFIID(GUID, SysMouse);
	CHECK_REFIID(GUID, SysKeyboard);
	CHECK_REFIID(GUID, Joystick);
	CHECK_REFIID(GUID, SysMouseEm);
	CHECK_REFIID(GUID, SysMouseEm2);
	CHECK_REFIID(GUID, SysKeyboardEm);
	CHECK_REFIID(GUID, SysKeyboardEm2);
	// Predefined force feedback effects
	CHECK_REFIID(GUID, ConstantForce);
	CHECK_REFIID(GUID, RampForce);
	CHECK_REFIID(GUID, Square);
	CHECK_REFIID(GUID, Sine);
	CHECK_REFIID(GUID, Triangle);
	CHECK_REFIID(GUID, SawtoothUp);
	CHECK_REFIID(GUID, SawtoothDown);
	CHECK_REFIID(GUID, Spring);
	CHECK_REFIID(GUID, Damper);
	CHECK_REFIID(GUID, Inertia);
	CHECK_REFIID(GUID, Friction);
	CHECK_REFIID(GUID, CustomForce);
	// DirectShow
	CHECK_REFIID(CLSID, AMMultiMediaStream);
	CHECK_REFIID(CLSID, AMDirectDrawStream);
	CHECK_REFIID(CLSID, AMAudioStream);
	CHECK_REFIID(CLSID, AMAudioData);
	CHECK_REFIID(CLSID, AMMediaTypeStream);
	CHECK_REFIID(IID, IAMMultiMediaStream);
	CHECK_REFIID(IID, IAMMediaStream);
	CHECK_REFIID(IID, IMediaStreamFilter);
	CHECK_REFIID(IID, IDirectDrawMediaSampleAllocator);
	CHECK_REFIID(IID, IDirectDrawMediaSample);
	CHECK_REFIID(IID, IAMMediaTypeStream);
	CHECK_REFIID(IID, IAMMediaTypeSample);
	CHECK_REFIID(IID, IDirectDrawStreamSample);
	CHECK_REFIID(IID, IDirectDrawMediaStream);
	CHECK_REFIID(IID, IDDVideoAcceleratorContainer);
	CHECK_REFIID(IID, IDirectDrawVideoAccelerator);
	// multimedia
	CHECK_REFIID(CLSID, MMDeviceEnumerator);
	CHECK_REFIID(IID, IMMNotificationClient);
	CHECK_REFIID(IID, IMMDevice);
	CHECK_REFIID(IID, IMMDeviceCollection);
	CHECK_REFIID(IID, IMMEndpoint);
	CHECK_REFIID(IID, IMMDeviceEnumerator);
	CHECK_REFIID(IID, IMMDeviceActivator);
	CHECK_REFIID(IID, IActivateAudioInterfaceCompletionHandler);
	CHECK_REFIID(IID, IActivateAudioInterfaceAsyncOperation);
	CHECK_REFIID(IID, IMultiMediaStream);
	CHECK_REFIID(IID, IMediaStream);
	CHECK_REFIID(IID, IStreamSample);
	// dxwrapper specific
	CHECK_REFIID(IID, GetRealInterface);
	CHECK_REFIID(IID, GetInterfaceX);

	UINT x = 0;
	char buffer[(sizeof(IID) * 2) + 5] = { '\0' };
	for (size_t j : {3, 2, 1, 0, 0xFF, 5, 4, 0xFF, 7, 6, 0xFF, 8, 9, 0xFF, 10, 11, 12, 13, 14, 15})
	{
		if (j == 0xFF)
		{
			buffer[x] = '-';
		}
		else
		{
			sprintf_s(buffer + x, 3, "%02X", ((byte*)&riid)[j]);
			x++;
		}
		x++;
	}

	return Logging::LogStruct(os) << buffer;
}

std::ostream& operator<<(std::ostream& os, const DDERR& ErrCode)
{
#define VISIT_DDERR_CODES(visit) \
	visit(DD_OK) \
	visit(DDERR_ALREADYINITIALIZED) \
	visit(DDERR_CANNOTATTACHSURFACE) \
	visit(DDERR_CANNOTDETACHSURFACE) \
	visit(DDERR_CURRENTLYNOTAVAIL) \
	visit(DDERR_EXCEPTION) \
	visit(DDERR_GENERIC) \
	visit(DDERR_HEIGHTALIGN) \
	visit(DDERR_INCOMPATIBLEPRIMARY) \
	visit(DDERR_INVALIDCAPS) \
	visit(DDERR_INVALIDCLIPLIST) \
	visit(DDERR_INVALIDMODE) \
	visit(DDERR_INVALIDOBJECT) \
	visit(DDERR_INVALIDPARAMS) \
	visit(DDERR_INVALIDPIXELFORMAT) \
	visit(DDERR_INVALIDRECT) \
	visit(DDERR_LOCKEDSURFACES) \
	visit(DDERR_NO3D) \
	visit(DDERR_NOALPHAHW) \
	visit(DDERR_NOSTEREOHARDWARE) \
	visit(DDERR_NOSURFACELEFT) \
	visit(DDERR_NOCLIPLIST) \
	visit(DDERR_NOCOLORCONVHW) \
	visit(DDERR_NOCOOPERATIVELEVELSET) \
	visit(DDERR_NOCOLORKEY) \
	visit(DDERR_NOCOLORKEYHW) \
	visit(DDERR_NODIRECTDRAWSUPPORT) \
	visit(DDERR_NOEXCLUSIVEMODE) \
	visit(DDERR_NOFLIPHW) \
	visit(DDERR_NOGDI) \
	visit(DDERR_NOMIRRORHW) \
	visit(DDERR_NOTFOUND) \
	visit(DDERR_NOOVERLAYHW) \
	visit(DDERR_OVERLAPPINGRECTS) \
	visit(DDERR_NORASTEROPHW) \
	visit(DDERR_NOROTATIONHW) \
	visit(DDERR_NOSTRETCHHW) \
	visit(DDERR_NOT4BITCOLOR) \
	visit(DDERR_NOT4BITCOLORINDEX) \
	visit(DDERR_NOT8BITCOLOR) \
	visit(DDERR_NOTEXTUREHW) \
	visit(DDERR_NOVSYNCHW) \
	visit(DDERR_NOZBUFFERHW) \
	visit(DDERR_NOZOVERLAYHW) \
	visit(DDERR_OUTOFCAPS) \
	visit(DDERR_OUTOFMEMORY) \
	visit(DDERR_OUTOFVIDEOMEMORY) \
	visit(DDERR_OVERLAYCANTCLIP) \
	visit(DDERR_OVERLAYCOLORKEYONLYONEACTIVE) \
	visit(DDERR_PALETTEBUSY) \
	visit(DDERR_COLORKEYNOTSET) \
	visit(DDERR_SURFACEALREADYATTACHED) \
	visit(DDERR_SURFACEALREADYDEPENDENT) \
	visit(DDERR_SURFACEBUSY) \
	visit(DDERR_CANTLOCKSURFACE) \
	visit(DDERR_SURFACEISOBSCURED) \
	visit(DDERR_SURFACELOST) \
	visit(DDERR_SURFACENOTATTACHED) \
	visit(DDERR_TOOBIGHEIGHT) \
	visit(DDERR_TOOBIGSIZE) \
	visit(DDERR_TOOBIGWIDTH) \
	visit(DDERR_UNSUPPORTED) \
	visit(DDERR_UNSUPPORTEDFORMAT) \
	visit(DDERR_UNSUPPORTEDMASK) \
	visit(DDERR_INVALIDSTREAM) \
	visit(DDERR_VERTICALBLANKINPROGRESS) \
	visit(DDERR_WASSTILLDRAWING) \
	visit(DDERR_DDSCAPSCOMPLEXREQUIRED) \
	visit(DDERR_XALIGN) \
	visit(DDERR_INVALIDDIRECTDRAWGUID) \
	visit(DDERR_DIRECTDRAWALREADYCREATED) \
	visit(DDERR_NODIRECTDRAWHW) \
	visit(DDERR_PRIMARYSURFACEALREADYEXISTS) \
	visit(DDERR_NOEMULATION) \
	visit(DDERR_REGIONTOOSMALL) \
	visit(DDERR_CLIPPERISUSINGHWND) \
	visit(DDERR_NOCLIPPERATTACHED) \
	visit(DDERR_NOHWND) \
	visit(DDERR_HWNDSUBCLASSED) \
	visit(DDERR_HWNDALREADYSET) \
	visit(DDERR_NOPALETTEATTACHED) \
	visit(DDERR_NOPALETTEHW) \
	visit(DDERR_BLTFASTCANTCLIP) \
	visit(DDERR_NOBLTHW) \
	visit(DDERR_NODDROPSHW) \
	visit(DDERR_OVERLAYNOTVISIBLE) \
	visit(DDERR_NOOVERLAYDEST) \
	visit(DDERR_INVALIDPOSITION) \
	visit(DDERR_NOTAOVERLAYSURFACE) \
	visit(DDERR_EXCLUSIVEMODEALREADYSET) \
	visit(DDERR_NOTFLIPPABLE) \
	visit(DDERR_CANTDUPLICATE) \
	visit(DDERR_NOTLOCKED) \
	visit(DDERR_CANTCREATEDC) \
	visit(DDERR_NODC) \
	visit(DDERR_WRONGMODE) \
	visit(DDERR_IMPLICITLYCREATED) \
	visit(DDERR_NOTPALETTIZED) \
	visit(DDERR_UNSUPPORTEDMODE) \
	visit(DDERR_NOMIPMAPHW) \
	visit(DDERR_INVALIDSURFACETYPE) \
	visit(DDERR_NOOPTIMIZEHW) \
	visit(DDERR_NOTLOADED) \
	visit(DDERR_NOFOCUSWINDOW) \
	visit(DDERR_NOTONMIPMAPSUBLEVEL) \
	visit(DDERR_DCALREADYCREATED) \
	visit(DDERR_NONONLOCALVIDMEM) \
	visit(DDERR_CANTPAGELOCK) \
	visit(DDERR_CANTPAGEUNLOCK) \
	visit(DDERR_NOTPAGELOCKED) \
	visit(DDERR_MOREDATA) \
	visit(DDERR_EXPIRED) \
	visit(DDERR_TESTFINISHED) \
	visit(DDERR_NEWMODE) \
	visit(DDERR_D3DNOTINITIALIZED) \
	visit(DDERR_VIDEONOTACTIVE) \
	visit(DDERR_NOMONITORINFORMATION) \
	visit(DDERR_NODRIVERSUPPORT) \
	visit(DDERR_DEVICEDOESNTOWNSURFACE) \
	visit(DDERR_NOTINITIALIZED) \
	visit(DDERR_LOADFAILED) \
	visit(DDERR_BADVERSIONINFO) \
	visit(DDERR_BADPROCADDRESS) \
	visit(DDERR_LEGACYUSAGE) \
	visit(D3DERR_BADMAJORVERSION) \
	visit(D3DERR_BADMINORVERSION) \
	visit(D3DERR_INVALID_DEVICE) \
	visit(D3DERR_INITFAILED) \
	visit(D3DERR_DEVICEAGGREGATED) \
	visit(D3DERR_EXECUTE_CREATE_FAILED) \
	visit(D3DERR_EXECUTE_DESTROY_FAILED) \
	visit(D3DERR_EXECUTE_LOCK_FAILED) \
	visit(D3DERR_EXECUTE_UNLOCK_FAILED) \
	visit(D3DERR_EXECUTE_LOCKED) \
	visit(D3DERR_EXECUTE_NOT_LOCKED) \
	visit(D3DERR_EXECUTE_FAILED) \
	visit(D3DERR_EXECUTE_CLIPPED_FAILED) \
	visit(D3DERR_TEXTURE_NO_SUPPORT) \
	visit(D3DERR_TEXTURE_CREATE_FAILED) \
	visit(D3DERR_TEXTURE_DESTROY_FAILED) \
	visit(D3DERR_TEXTURE_LOCK_FAILED) \
	visit(D3DERR_TEXTURE_UNLOCK_FAILED) \
	visit(D3DERR_TEXTURE_LOAD_FAILED) \
	visit(D3DERR_TEXTURE_SWAP_FAILED) \
	visit(D3DERR_TEXTURE_LOCKED) \
	visit(D3DERR_TEXTURE_NOT_LOCKED) \
	visit(D3DERR_TEXTURE_GETSURF_FAILED) \
	visit(D3DERR_MATRIX_CREATE_FAILED) \
	visit(D3DERR_MATRIX_DESTROY_FAILED) \
	visit(D3DERR_MATRIX_SETDATA_FAILED) \
	visit(D3DERR_MATRIX_GETDATA_FAILED) \
	visit(D3DERR_SETVIEWPORTDATA_FAILED) \
	visit(D3DERR_INVALIDCURRENTVIEWPORT) \
	visit(D3DERR_INVALIDPRIMITIVETYPE) \
	visit(D3DERR_INVALIDVERTEXTYPE) \
	visit(D3DERR_TEXTURE_BADSIZE) \
	visit(D3DERR_INVALIDRAMPTEXTURE) \
	visit(D3DERR_MATERIAL_CREATE_FAILED) \
	visit(D3DERR_MATERIAL_DESTROY_FAILED) \
	visit(D3DERR_MATERIAL_SETDATA_FAILED) \
	visit(D3DERR_MATERIAL_GETDATA_FAILED) \
	visit(D3DERR_INVALIDPALETTE) \
	visit(D3DERR_ZBUFF_NEEDS_SYSTEMMEMORY) \
	visit(D3DERR_ZBUFF_NEEDS_VIDEOMEMORY) \
	visit(D3DERR_SURFACENOTINVIDMEM) \
	visit(D3DERR_LIGHT_SET_FAILED) \
	visit(D3DERR_LIGHTHASVIEWPORT) \
	visit(D3DERR_LIGHTNOTINTHISVIEWPORT) \
	visit(D3DERR_SCENE_IN_SCENE) \
	visit(D3DERR_SCENE_NOT_IN_SCENE) \
	visit(D3DERR_SCENE_BEGIN_FAILED) \
	visit(D3DERR_SCENE_END_FAILED) \
	visit(D3DERR_INBEGIN) \
	visit(D3DERR_NOTINBEGIN) \
	visit(D3DERR_NOVIEWPORTS) \
	visit(D3DERR_VIEWPORTDATANOTSET) \
	visit(D3DERR_VIEWPORTHASNODEVICE) \
	visit(D3DERR_NOCURRENTVIEWPORT) \
	visit(D3DERR_INVALIDVERTEXFORMAT) \
	visit(D3DERR_COLORKEYATTACHED) \
	visit(D3DERR_VERTEXBUFFEROPTIMIZED) \
	visit(D3DERR_VBUF_CREATE_FAILED) \
	visit(D3DERR_VERTEXBUFFERLOCKED) \
	visit(D3DERR_VERTEXBUFFERUNLOCKFAILED) \
	visit(D3DERR_ZBUFFER_NOTPRESENT) \
	visit(D3DERR_STENCILBUFFER_NOTPRESENT) \
	visit(D3DERR_WRONGTEXTUREFORMAT) \
	visit(D3DERR_UNSUPPORTEDCOLOROPERATION) \
	visit(D3DERR_UNSUPPORTEDCOLORARG) \
	visit(D3DERR_UNSUPPORTEDALPHAOPERATION) \
	visit(D3DERR_UNSUPPORTEDALPHAARG) \
	visit(D3DERR_TOOMANYOPERATIONS) \
	visit(D3DERR_CONFLICTINGTEXTUREFILTER) \
	visit(D3DERR_UNSUPPORTEDFACTORVALUE) \
	visit(D3DERR_CONFLICTINGRENDERSTATE) \
	visit(D3DERR_UNSUPPORTEDTEXTUREFILTER) \
	visit(D3DERR_TOOMANYPRIMITIVES) \
	visit(D3DERR_INVALIDMATRIX) \
	visit(D3DERR_TOOMANYVERTICES) \
	visit(D3DERR_CONFLICTINGTEXTUREPALETTE) \
	visit(D3DERR_CONFLICTINGTEXTUREPALETTE) \
	visit(D3DERR_INVALIDSTATEBLOCK) \
	visit(D3DERR_INBEGINSTATEBLOCK) \
	visit(D3DERR_NOTINBEGINSTATEBLOCK) \
	visit(D3DERR_COMMAND_UNPARSED) \
	visit(D3DERR_DRIVERINTERNALERROR) \
	visit(D3DERR_NOTFOUND) \
	visit(D3DERR_MOREDATA) \
	visit(D3DERR_DEVICELOST) \
	visit(D3DERR_DEVICENOTRESET) \
	visit(D3DERR_NOTAVAILABLE) \
	visit(D3DERR_OUTOFVIDEOMEMORY) \
	visit(D3DERR_INVALIDDEVICE) \
	visit(D3DERR_INVALIDCALL) \
	visit(D3DERR_DRIVERINVALIDCALL) \
	visit(D3DERR_WASSTILLDRAWING) \
	visit(D3DOK_NOAUTOGEN) \
	visit(D3DERR_DEVICEREMOVED) \
	visit(D3DERR_DEVICEHUNG) \
	visit(D3DERR_UNSUPPORTEDOVERLAY) \
	visit(D3DERR_UNSUPPORTEDOVERLAYFORMAT) \
	visit(D3DERR_CANNOTPROTECTCONTENT) \
	visit(D3DERR_UNSUPPORTEDCRYPTO) \
	visit(D3DERR_PRESENT_STATISTICS_DISJOINT) \
	visit(E_NOINTERFACE) \
	visit(E_POINTER) \
	visit(S_NOT_RESIDENT) \
	visit(S_RESIDENT_IN_SHARED_MEMORY) \
	visit(S_PRESENT_MODE_CHANGED) \
	visit(S_PRESENT_OCCLUDED) \
	visit(D3D_OK)

#define VISIT_DDERR_RETURN(x) \
	if (ErrCode == x) \
	{ \
		return os << #x; \
	}

	VISIT_DDERR_CODES(VISIT_DDERR_RETURN);

	return os << Logging::hex((DWORD)ErrCode);
}

std::ostream& operator<<(std::ostream& os, const D3DERR& ErrCode)
{
	return os << (DDERR)ErrCode;
}

std::ostream& operator<<(std::ostream& os, const DIERR& ErrCode)
{
#define VISIT_DIERR_CODES(visit) \
	visit(DI_OK) \
	visit(DI_POLLEDDEVICE) \
	visit(DI_DOWNLOADSKIPPED) \
	visit(DI_EFFECTRESTARTED) \
	visit(DI_TRUNCATED) \
	visit(DI_SETTINGSNOTSAVED) \
	visit(DI_TRUNCATEDANDRESTARTED) \
	visit(DI_WRITEPROTECT) \
	visit(DIERR_OLDDIRECTINPUTVERSION) \
	visit(DIERR_BETADIRECTINPUTVERSION) \
	visit(DIERR_BADDRIVERVER) \
	visit(DIERR_DEVICENOTREG) \
	visit(DIERR_NOTFOUND) \
	visit(DIERR_OBJECTNOTFOUND) \
	visit(DIERR_INVALIDPARAM) \
	visit(DIERR_NOINTERFACE) \
	visit(DIERR_GENERIC) \
	visit(DIERR_OUTOFMEMORY) \
	visit(DIERR_UNSUPPORTED) \
	visit(DIERR_NOTINITIALIZED) \
	visit(DIERR_ALREADYINITIALIZED) \
	visit(DIERR_NOAGGREGATION) \
	visit(DIERR_OTHERAPPHASPRIO) \
	visit(DIERR_INPUTLOST) \
	visit(DIERR_ACQUIRED) \
	visit(DIERR_NOTACQUIRED) \
	visit(DIERR_READONLY) \
	visit(DIERR_HANDLEEXISTS) \
	visit(DIERR_INSUFFICIENTPRIVS) \
	visit(DIERR_DEVICEFULL) \
	visit(DIERR_MOREDATA) \
	visit(DIERR_NOTDOWNLOADED) \
	visit(DIERR_HASEFFECTS) \
	visit(DIERR_NOTEXCLUSIVEACQUIRED) \
	visit(DIERR_INCOMPLETEEFFECT) \
	visit(DIERR_NOTBUFFERED) \
	visit(DIERR_EFFECTPLAYING) \
	visit(DIERR_UNPLUGGED) \
	visit(DIERR_REPORTFULL) \
	visit(DIERR_MAPFILEFAIL) \
	visit(E_PENDING) \
	visit(E_NOINTERFACE) \
	visit(E_POINTER) \
	visit(E_HANDLE)

#define VISIT_DIERR_RETURN(x) \
	if (ErrCode == x) \
	{ \
		return os << #x; \
	}

	if (ErrCode == S_FALSE)
	{
		return os << "'DI_NOTATTACHED' or 'DI_BUFFEROVERFLOW' or 'DI_PROPNOEFFECT' or 'DI_NOEFFECT'";
	}

	VISIT_DIERR_CODES(VISIT_DIERR_RETURN);

	return os << Logging::hex((DWORD)ErrCode);
}

std::ostream& operator<<(std::ostream& os, const DSERR& ErrCode)
{
#define VISIT_DSERR_CODES(visit) \
	visit(DS_OK) \
	visit(DS_NO_VIRTUALIZATION) \
	visit(DSERR_ALLOCATED) \
	visit(DSERR_CONTROLUNAVAIL) \
	visit(DSERR_INVALIDPARAM) \
	visit(DSERR_INVALIDCALL) \
	visit(DSERR_GENERIC) \
	visit(DSERR_PRIOLEVELNEEDED) \
	visit(DSERR_OUTOFMEMORY) \
	visit(DSERR_BADFORMAT) \
	visit(DSERR_UNSUPPORTED) \
	visit(DSERR_NODRIVER) \
	visit(DSERR_ALREADYINITIALIZED) \
	visit(DSERR_NOAGGREGATION) \
	visit(DSERR_BUFFERLOST) \
	visit(DSERR_OTHERAPPHASPRIO) \
	visit(DSERR_UNINITIALIZED) \
	visit(DSERR_NOINTERFACE) \
	visit(DSERR_ACCESSDENIED) \
	visit(DSERR_BUFFERTOOSMALL) \
	visit(DSERR_DS8_REQUIRED) \
	visit(DSERR_SENDLOOP) \
	visit(DSERR_BADSENDBUFFERGUID) \
	visit(DSERR_OBJECTNOTFOUND) \
	visit(DSERR_FXUNAVAILABLE) \
	visit(E_NOINTERFACE) \
	visit(E_POINTER)

#define VISIT_DSERR_RETURN(x) \
	if (ErrCode == x) \
	{ \
		return os << #x; \
	}

	VISIT_DSERR_CODES(VISIT_DSERR_RETURN);

	return os << Logging::hex((DWORD)ErrCode);
}

std::ostream& operator<<(std::ostream& os, const WMMSG& Id)
{
#define VISIT_WMMSG_CODES(visit) \
    visit(0, "WM_NULL") \
    visit(1, "WM_CREATE") \
    visit(2, "WM_DESTROY") \
    visit(3, "WM_MOVE") \
    visit(5, "WM_SIZE") \
    visit(6, "WM_ACTIVATE") \
    visit(7, "WM_SETFOCUS") \
    visit(8, "WM_KILLFOCUS") \
    visit(10, "WM_ENABLE") \
    visit(11, "WM_SETREDRAW") \
    visit(12, "WM_SETTEXT") \
    visit(13, "WM_GETTEXT") \
    visit(14, "WM_GETTEXTLENGTH") \
    visit(15, "WM_PAINT") \
    visit(16, "WM_CLOSE") \
    visit(17, "WM_QUERYENDSESSION") \
    visit(18, "WM_QUIT") \
    visit(19, "WM_QUERYOPEN") \
    visit(20, "WM_ERASEBKGND") \
    visit(21, "WM_SYSCOLORCHANGE") \
    visit(22, "WM_ENDSESSION") \
    visit(24, "WM_SHOWWINDOW") \
    visit(25, "WM_CTLCOLOR") \
    visit(26, "WM_WININICHANGE") \
    visit(27, "WM_DEVMODECHANGE") \
    visit(28, "WM_ACTIVATEAPP") \
    visit(29, "WM_FONTCHANGE") \
    visit(30, "WM_TIMECHANGE") \
    visit(31, "WM_CANCELMODE") \
    visit(32, "WM_SETCURSOR") \
    visit(33, "WM_MOUSEACTIVATE") \
    visit(34, "WM_CHILDACTIVATE") \
    visit(35, "WM_QUEUESYNC") \
    visit(36, "WM_GETMINMAXINFO") \
    visit(38, "WM_PAINTICON") \
    visit(39, "WM_ICONERASEBKGND") \
    visit(40, "WM_NEXTDLGCTL") \
    visit(42, "WM_SPOOLERSTATUS") \
    visit(43, "WM_DRAWITEM") \
    visit(44, "WM_MEASUREITEM") \
    visit(45, "WM_DELETEITEM") \
    visit(46, "WM_VKEYTOITEM") \
    visit(47, "WM_CHARTOITEM") \
    visit(48, "WM_SETFONT") \
    visit(49, "WM_GETFONT") \
    visit(50, "WM_SETHOTKEY") \
    visit(51, "WM_GETHOTKEY") \
    visit(55, "WM_QUERYDRAGICON") \
    visit(57, "WM_COMPAREITEM") \
    visit(61, "WM_GETOBJECT") \
    visit(65, "WM_COMPACTING") \
    visit(68, "WM_COMMNOTIFY") \
    visit(70, "WM_WINDOWPOSCHANGING") \
    visit(71, "WM_WINDOWPOSCHANGED") \
    visit(72, "WM_POWER") \
    visit(73, "WM_COPYGLOBALDATA") \
    visit(74, "WM_COPYDATA") \
    visit(75, "WM_CANCELJOURNAL") \
    visit(78, "WM_NOTIFY") \
    visit(80, "WM_INPUTLANGCHANGEREQUEST") \
    visit(81, "WM_INPUTLANGCHANGE") \
    visit(82, "WM_TCARD") \
    visit(83, "WM_HELP") \
    visit(84, "WM_USERCHANGED") \
    visit(85, "WM_NOTIFYFORMAT") \
    visit(123, "WM_CONTEXTMENU") \
    visit(124, "WM_STYLECHANGING") \
    visit(125, "WM_STYLECHANGED") \
    visit(126, "WM_DISPLAYCHANGE") \
    visit(127, "WM_GETICON") \
    visit(128, "WM_SETICON") \
    visit(129, "WM_NCCREATE") \
    visit(130, "WM_NCDESTROY") \
    visit(131, "WM_NCCALCSIZE") \
    visit(132, "WM_NCHITTEST") \
    visit(133, "WM_NCPAINT") \
    visit(134, "WM_NCACTIVATE") \
    visit(135, "WM_GETDLGCODE") \
    visit(136, "WM_SYNCPAINT") \
    visit(160, "WM_NCMOUSEMOVE") \
    visit(161, "WM_NCLBUTTONDOWN") \
    visit(162, "WM_NCLBUTTONUP") \
    visit(163, "WM_NCLBUTTONDBLCLK") \
    visit(164, "WM_NCRBUTTONDOWN") \
    visit(165, "WM_NCRBUTTONUP") \
    visit(166, "WM_NCRBUTTONDBLCLK") \
    visit(167, "WM_NCMBUTTONDOWN") \
    visit(168, "WM_NCMBUTTONUP") \
    visit(169, "WM_NCMBUTTONDBLCLK") \
    visit(171, "WM_NCXBUTTONDOWN") \
    visit(172, "WM_NCXBUTTONUP") \
    visit(173, "WM_NCXBUTTONDBLCLK") \
    visit(176, "EM_GETSEL") \
    visit(177, "EM_SETSEL") \
    visit(178, "EM_GETRECT") \
    visit(179, "EM_SETRECT") \
    visit(180, "EM_SETRECTNP") \
    visit(181, "EM_SCROLL") \
    visit(182, "EM_LINESCROLL") \
    visit(183, "EM_SCROLLCARET") \
    visit(185, "EM_GETMODIFY") \
    visit(187, "EM_SETMODIFY") \
    visit(188, "EM_GETLINECOUNT") \
    visit(189, "EM_LINEINDEX") \
    visit(190, "EM_SETHANDLE") \
    visit(191, "EM_GETHANDLE") \
    visit(192, "EM_GETTHUMB") \
    visit(193, "EM_LINELENGTH") \
    visit(194, "EM_REPLACESEL") \
    visit(195, "EM_SETFONT") \
    visit(196, "EM_GETLINE") \
    visit(197, "EM_LIMITTEXT, EM_SETLIMITTEXT") \
    visit(198, "EM_CANUNDO") \
    visit(199, "EM_UNDO") \
    visit(200, "EM_FMTLINES") \
    visit(201, "EM_LINEFROMCHAR") \
    visit(202, "EM_SETWORDBREAK") \
    visit(203, "EM_SETTABSTOPS") \
    visit(204, "EM_SETPASSWORDCHAR") \
    visit(205, "EM_EMPTYUNDOBUFFER") \
    visit(206, "EM_GETFIRSTVISIBLELINE") \
    visit(207, "EM_SETREADONLY") \
    visit(209, "EM_SETWORDBREAKPROC, EM_GETWORDBREAKPROC") \
    visit(210, "EM_GETPASSWORDCHAR") \
    visit(211, "EM_SETMARGINS") \
    visit(212, "EM_GETMARGINS") \
    visit(213, "EM_GETLIMITTEXT") \
    visit(214, "EM_POSFROMCHAR") \
    visit(215, "EM_CHARFROMPOS") \
    visit(216, "EM_SETIMESTATUS") \
    visit(217, "EM_GETIMESTATUS") \
    visit(224, "SBM_SETPOS") \
    visit(225, "SBM_GETPOS") \
    visit(226, "SBM_SETRANGE") \
    visit(227, "SBM_GETRANGE") \
    visit(228, "SBM_ENABLE_ARROWS") \
    visit(230, "SBM_SETRANGEREDRAW") \
    visit(233, "SBM_SETSCROLLINFO") \
    visit(234, "SBM_GETSCROLLINFO") \
    visit(235, "SBM_GETSCROLLBARINFO") \
    visit(240, "BM_GETCHECK") \
    visit(241, "BM_SETCHECK") \
    visit(242, "BM_GETSTATE") \
    visit(243, "BM_SETSTATE") \
    visit(244, "BM_SETSTYLE") \
    visit(245, "BM_CLICK") \
    visit(246, "BM_GETIMAGE") \
    visit(247, "BM_SETIMAGE") \
    visit(248, "BM_SETDONTCLICK") \
    visit(255, "WM_INPUT") \
    visit(256, "WM_KEYDOWN") \
    visit(257, "WM_KEYUP") \
    visit(258, "WM_CHAR") \
    visit(259, "WM_DEADCHAR") \
    visit(260, "WM_SYSKEYDOWN") \
    visit(261, "WM_SYSKEYUP") \
    visit(262, "WM_SYSCHAR") \
    visit(263, "WM_SYSDEADCHAR") \
    visit(265, "WM_UNICHAR, WM_WNT_CONVERTREQUESTEX") \
    visit(266, "WM_CONVERTREQUEST") \
    visit(267, "WM_CONVERTRESULT") \
    visit(268, "WM_INTERIM") \
    visit(269, "WM_IME_STARTCOMPOSITION") \
    visit(270, "WM_IME_ENDCOMPOSITION") \
    visit(271, "WM_IME_COMPOSITION") \
    visit(272, "WM_INITDIALOG") \
    visit(273, "WM_COMMAND") \
    visit(274, "WM_SYSCOMMAND") \
    visit(275, "WM_TIMER") \
    visit(276, "WM_HSCROLL") \
    visit(277, "WM_VSCROLL") \
    visit(278, "WM_INITMENU") \
    visit(279, "WM_INITMENUPOPUP") \
    visit(280, "WM_SYSTIMER") \
    visit(287, "WM_MENUSELECT") \
    visit(288, "WM_MENUCHAR") \
    visit(289, "WM_ENTERIDLE") \
    visit(290, "WM_MENURBUTTONUP") \
    visit(291, "WM_MENUDRAG") \
    visit(292, "WM_MENUGETOBJECT") \
    visit(293, "WM_UNINITMENUPOPUP") \
    visit(294, "WM_MENUCOMMAND") \
    visit(295, "WM_CHANGEUISTATE") \
    visit(296, "WM_UPDATEUISTATE") \
    visit(297, "WM_QUERYUISTATE") \
    visit(306, "WM_CTLCOLORMSGBOX") \
    visit(307, "WM_CTLCOLOREDIT") \
    visit(308, "WM_CTLCOLORLISTBOX") \
    visit(309, "WM_CTLCOLORBTN") \
    visit(310, "WM_CTLCOLORDLG") \
    visit(311, "WM_CTLCOLORSCROLLBAR") \
    visit(312, "WM_CTLCOLORSTATIC") \
    visit(512, "WM_MOUSEMOVE") \
    visit(513, "WM_LBUTTONDOWN") \
    visit(514, "WM_LBUTTONUP") \
    visit(515, "WM_LBUTTONDBLCLK") \
    visit(516, "WM_RBUTTONDOWN") \
    visit(517, "WM_RBUTTONUP") \
    visit(518, "WM_RBUTTONDBLCLK") \
    visit(519, "WM_MBUTTONDOWN") \
    visit(520, "WM_MBUTTONUP") \
    visit(521, "WM_MBUTTONDBLCLK") \
    visit(522, "WM_MOUSEWHEEL") \
    visit(523, "WM_XBUTTONDOWN") \
    visit(524, "WM_XBUTTONUP") \
    visit(525, "WM_XBUTTONDBLCLK") \
    visit(526, "WM_MOUSEHWHEEL") \
    visit(528, "WM_PARENTNOTIFY") \
    visit(529, "WM_ENTERMENULOOP") \
    visit(530, "WM_EXITMENULOOP") \
    visit(531, "WM_NEXTMENU") \
    visit(532, "WM_SIZING") \
    visit(533, "WM_CAPTURECHANGED") \
    visit(534, "WM_MOVING") \
    visit(536, "WM_POWERBROADCAST") \
    visit(537, "WM_DEVICECHANGE") \
    visit(544, "WM_MDICREATE") \
    visit(545, "WM_MDIDESTROY") \
    visit(546, "WM_MDIACTIVATE") \
    visit(547, "WM_MDIRESTORE") \
    visit(548, "WM_MDINEXT") \
    visit(549, "WM_MDIMAXIMIZE") \
    visit(550, "WM_MDITILE") \
    visit(551, "WM_MDICASCADE") \
    visit(552, "WM_MDIICONARRANGE") \
    visit(553, "WM_MDIGETACTIVE") \
    visit(560, "WM_MDISETMENU") \
    visit(561, "WM_ENTERSIZEMOVE") \
    visit(562, "WM_EXITSIZEMOVE") \
    visit(563, "WM_DROPFILES") \
    visit(564, "WM_MDIREFRESHMENU") \
    visit(640, "WM_IME_REPORT") \
    visit(641, "WM_IME_SETCONTEXT") \
    visit(642, "WM_IME_NOTIFY") \
    visit(643, "WM_IME_CONTROL") \
    visit(644, "WM_IME_COMPOSITIONFULL") \
    visit(645, "WM_IME_SELECT") \
    visit(646, "WM_IME_CHAR") \
    visit(648, "WM_IME_REQUEST") \
    visit(656, "WM_IMEKEYDOWN, WM_IME_KEYDOWN") \
    visit(657, "WM_IMEKEYUP, WM_IME_KEYUP") \
    visit(672, "WM_NCMOUSEHOVER") \
    visit(673, "WM_MOUSEHOVER") \
    visit(674, "WM_NCMOUSELEAVE") \
    visit(675, "WM_MOUSELEAVE") \
    visit(768, "WM_CUT") \
    visit(769, "WM_COPY") \
    visit(770, "WM_PASTE") \
    visit(771, "WM_CLEAR") \
    visit(772, "WM_UNDO") \
    visit(773, "WM_RENDERFORMAT") \
    visit(774, "WM_RENDERALLFORMATS") \
    visit(775, "WM_DESTROYCLIPBOARD") \
    visit(776, "WM_DRAWCLIPBOARD") \
    visit(777, "WM_PAINTCLIPBOARD") \
    visit(778, "WM_VSCROLLCLIPBOARD") \
    visit(779, "WM_SIZECLIPBOARD") \
    visit(780, "WM_ASKCBFORMATNAME") \
    visit(781, "WM_CHANGECBCHAIN") \
    visit(782, "WM_HSCROLLCLIPBOARD") \
    visit(783, "WM_QUERYNEWPALETTE") \
    visit(784, "WM_PALETTEISCHANGING") \
    visit(785, "WM_PALETTECHANGED") \
    visit(786, "WM_HOTKEY") \
    visit(791, "WM_PRINT") \
    visit(792, "WM_PRINTCLIENT") \
    visit(793, "WM_APPCOMMAND") \
    visit(856, "WM_HANDHELDFIRST") \
    visit(863, "WM_HANDHELDLAST") \
    visit(864, "WM_AFXFIRST") \
    visit(895, "WM_AFXLAST") \
    visit(896, "WM_PENWINFIRST") \
    visit(897, "WM_RCRESULT") \
    visit(898, "WM_HOOKRCRESULT") \
    visit(899, "WM_GLOBALRCCHANGE, WM_PENMISCINFO") \
    visit(900, "WM_SKB") \
    visit(901, "WM_HEDITCTL, WM_PENCTL") \
    visit(902, "WM_PENMISC") \
    visit(903, "WM_CTLINIT") \
    visit(904, "WM_PENEVENT") \
    visit(911, "WM_PENWINLAST") \
    visit(1024, "DDM_SETFMT, DM_GETDEFID, NIN_SELECT, TBM_GETPOS, WM_PSD_PAGESETUPDLG, WM_USER") \
    visit(1025, "CBEM_INSERTITEMA, DDM_DRAW, DM_SETDEFID, HKM_SETHOTKEY, PBM_SETRANGE, RB_INSERTBANDA, SB_SETTEXTA, TB_ENABLEBUTTON, TBM_GETRANGEMIN, TTM_ACTIVATE, WM_CHOOSEFONT_GETLOGFONT, WM_PSD_FULLPAGERECT") \
    visit(1026, "CBEM_SETIMAGELIST, DDM_CLOSE, DM_REPOSITION, HKM_GETHOTKEY, PBM_SETPOS, RB_DELETEBAND, SB_GETTEXTA, TB_CHECKBUTTON, TBM_GETRANGEMAX, WM_PSD_MINMARGINRECT") \
    visit(1027, "CBEM_GETIMAGELIST, DDM_BEGIN, HKM_SETRULES, PBM_DELTAPOS, RB_GETBARINFO, SB_GETTEXTLENGTHA, TBM_GETTIC, TB_PRESSBUTTON, TTM_SETDELAYTIME, WM_PSD_MARGINRECT") \
    visit(1028, "CBEM_GETITEMA, DDM_END, PBM_SETSTEP, RB_SETBARINFO, SB_SETPARTS, TB_HIDEBUTTON, TBM_SETTIC, TTM_ADDTOOLA, WM_PSD_GREEKTEXTRECT") \
    visit(1029, "CBEM_SETITEMA, PBM_STEPIT, TB_INDETERMINATE, TBM_SETPOS, TTM_DELTOOLA, WM_PSD_ENVSTAMPRECT") \
    visit(1030, "CBEM_GETCOMBOCONTROL, PBM_SETRANGE32, RB_SETBANDINFOA, SB_GETPARTS, TB_MARKBUTTON, TBM_SETRANGE, TTM_NEWTOOLRECTA, WM_PSD_YAFULLPAGERECT") \
    visit(1031, "CBEM_GETEDITCONTROL, PBM_GETRANGE, RB_SETPARENT, SB_GETBORDERS, TBM_SETRANGEMIN, TTM_RELAYEVENT") \
    visit(1032, "CBEM_SETEXSTYLE, PBM_GETPOS, RB_HITTEST, SB_SETMINHEIGHT, TBM_SETRANGEMAX, TTM_GETTOOLINFOA") \
    visit(1033, "CBEM_GETEXSTYLE, CBEM_GETEXTENDEDSTYLE, PBM_SETBARCOLOR, RB_GETRECT, SB_SIMPLE, TB_ISBUTTONENABLED, TBM_CLEARTICS, TTM_SETTOOLINFOA") \
    visit(1034, "CBEM_HASEDITCHANGED, RB_INSERTBANDW, SB_GETRECT, TB_ISBUTTONCHECKED, TBM_SETSEL, TTM_HITTESTA, WIZ_QUERYNUMPAGES") \
    visit(1035, "CBEM_INSERTITEMW, RB_SETBANDINFOW, SB_SETTEXTW, TB_ISBUTTONPRESSED, TBM_SETSELSTART, TTM_GETTEXTA, WIZ_NEXT") \
    visit(1036, "CBEM_SETITEMW, RB_GETBANDCOUNT, SB_GETTEXTLENGTHW, TB_ISBUTTONHIDDEN, TBM_SETSELEND, TTM_UPDATETIPTEXTA, WIZ_PREV") \
    visit(1037, "CBEM_GETITEMW, RB_GETROWCOUNT, SB_GETTEXTW, TB_ISBUTTONINDETERMINATE, TTM_GETTOOLCOUNT") \
    visit(1038, "CBEM_SETEXTENDEDSTYLE, RB_GETROWHEIGHT, SB_ISSIMPLE, TB_ISBUTTONHIGHLIGHTED, TBM_GETPTICS, TTM_ENUMTOOLSA") \
    visit(1039, "SB_SETICON, TBM_GETTICPOS, TTM_GETCURRENTTOOLA") \
    visit(1040, "RB_IDTOINDEX, SB_SETTIPTEXTA, TBM_GETNUMTICS, TTM_WINDOWFROMPOINT") \
    visit(1041, "RB_GETTOOLTIPS, SB_SETTIPTEXTW, TBM_GETSELSTART, TB_SETSTATE, TTM_TRACKACTIVATE") \
    visit(1042, "RB_SETTOOLTIPS, SB_GETTIPTEXTA, TB_GETSTATE, TBM_GETSELEND, TTM_TRACKPOSITION") \
    visit(1043, "RB_SETBKCOLOR, SB_GETTIPTEXTW, TB_ADDBITMAP, TBM_CLEARSEL, TTM_SETTIPBKCOLOR") \
    visit(1044, "RB_GETBKCOLOR, SB_GETICON, TB_ADDBUTTONSA, TBM_SETTICFREQ, TTM_SETTIPTEXTCOLOR") \
    visit(1045, "RB_SETTEXTCOLOR, TB_INSERTBUTTONA, TBM_SETPAGESIZE, TTM_GETDELAYTIME") \
    visit(1046, "RB_GETTEXTCOLOR, TB_DELETEBUTTON, TBM_GETPAGESIZE, TTM_GETTIPBKCOLOR") \
    visit(1047, "RB_SIZETORECT, TB_GETBUTTON, TBM_SETLINESIZE, TTM_GETTIPTEXTCOLOR") \
    visit(1048, "RB_BEGINDRAG, TB_BUTTONCOUNT, TBM_GETLINESIZE, TTM_SETMAXTIPWIDTH") \
    visit(1049, "RB_ENDDRAG, TB_COMMANDTOINDEX, TBM_GETTHUMBRECT, TTM_GETMAXTIPWIDTH") \
    visit(1050, "RB_DRAGMOVE, TBM_GETCHANNELRECT, TB_SAVERESTOREA, TTM_SETMARGIN") \
    visit(1051, "RB_GETBARHEIGHT, TB_CUSTOMIZE, TBM_SETTHUMBLENGTH, TTM_GETMARGIN") \
    visit(1052, "RB_GETBANDINFOW, TB_ADDSTRINGA, TBM_GETTHUMBLENGTH, TTM_POP") \
    visit(1053, "RB_GETBANDINFOA, TB_GETITEMRECT, TBM_SETTOOLTIPS, TTM_UPDATE") \
    visit(1054, "RB_MINIMIZEBAND, TB_BUTTONSTRUCTSIZE, TBM_GETTOOLTIPS, TTM_GETBUBBLESIZE") \
    visit(1055, "RB_MAXIMIZEBAND, TBM_SETTIPSIDE, TB_SETBUTTONSIZE, TTM_ADJUSTRECT") \
    visit(1056, "TBM_SETBUDDY, TB_SETBITMAPSIZE, TTM_SETTITLEA") \
    visit(1057, "MSG_FTS_JUMP_VA, TB_AUTOSIZE, TBM_GETBUDDY, TTM_SETTITLEW") \
    visit(1058, "RB_GETBANDBORDERS") \
    visit(1059, "MSG_FTS_JUMP_QWORD, RB_SHOWBAND, TB_GETTOOLTIPS") \
    visit(1060, "MSG_REINDEX_REQUEST, TB_SETTOOLTIPS") \
    visit(1061, "MSG_FTS_WHERE_IS_IT, RB_SETPALETTE, TB_SETPARENT") \
    visit(1062, "RB_GETPALETTE") \
    visit(1063, "RB_MOVEBAND, TB_SETROWS") \
    visit(1064, "TB_GETROWS") \
    visit(1065, "TB_GETBITMAPFLAGS") \
    visit(1066, "TB_SETCMDID") \
    visit(1067, "RB_PUSHCHEVRON, TB_CHANGEBITMAP") \
    visit(1068, "TB_GETBITMAP") \
    visit(1069, "MSG_GET_DEFFONT, TB_GETBUTTONTEXTA") \
    visit(1070, "TB_REPLACEBITMAP") \
    visit(1071, "TB_SETINDENT") \
    visit(1072, "TB_SETIMAGELIST") \
    visit(1073, "TB_GETIMAGELIST") \
    visit(1074, "TB_LOADIMAGES, EM_CANPASTE, TTM_ADDTOOLW") \
    visit(1075, "EM_DISPLAYBAND, TB_GETRECT, TTM_DELTOOLW") \
    visit(1076, "EM_EXGETSEL, TB_SETHOTIMAGELIST, TTM_NEWTOOLRECTW") \
    visit(1077, "EM_EXLIMITTEXT, TB_GETHOTIMAGELIST, TTM_GETTOOLINFOW") \
    visit(1078, "EM_EXLINEFROMCHAR, TB_SETDISABLEDIMAGELIST, TTM_SETTOOLINFOW") \
    visit(1079, "EM_EXSETSEL, TB_GETDISABLEDIMAGELIST, TTM_HITTESTW") \
    visit(1080, "EM_FINDTEXT, TB_SETSTYLE, TTM_GETTEXTW") \
    visit(1081, "EM_FORMATRANGE, TB_GETSTYLE, TTM_UPDATETIPTEXTW") \
    visit(1082, "EM_GETCHARFORMAT, TB_GETBUTTONSIZE, TTM_ENUMTOOLSW") \
    visit(1083, "EM_GETEVENTMASK, TB_SETBUTTONWIDTH, TTM_GETCURRENTTOOLW") \
    visit(1084, "EM_GETOLEINTERFACE, TB_SETMAXTEXTROWS") \
    visit(1085, "EM_GETPARAFORMAT, TB_GETTEXTROWS") \
    visit(1086, "EM_GETSELTEXT, TB_GETOBJECT") \
    visit(1087, "EM_HIDESELECTION, TB_GETBUTTONINFOW") \
    visit(1088, "EM_PASTESPECIAL, TB_SETBUTTONINFOW") \
    visit(1089, "EM_REQUESTRESIZE, TB_GETBUTTONINFOA") \
    visit(1090, "EM_SELECTIONTYPE, TB_SETBUTTONINFOA") \
    visit(1091, "EM_SETBKGNDCOLOR, TB_INSERTBUTTONW") \
    visit(1092, "EM_SETCHARFORMAT, TB_ADDBUTTONSW") \
    visit(1093, "EM_SETEVENTMASK, TB_HITTEST") \
    visit(1094, "EM_SETOLECALLBACK, TB_SETDRAWTEXTFLAGS") \
    visit(1095, "EM_SETPARAFORMAT, TB_GETHOTITEM") \
    visit(1096, "EM_SETTARGETDEVICE, TB_SETHOTITEM") \
    visit(1097, "EM_STREAMIN, TB_SETANCHORHIGHLIGHT") \
    visit(1098, "EM_STREAMOUT, TB_GETANCHORHIGHLIGHT") \
    visit(1099, "EM_GETTEXTRANGE, TB_GETBUTTONTEXTW") \
    visit(1100, "EM_FINDWORDBREAK, TB_SAVERESTOREW") \
    visit(1101, "EM_SETOPTIONS, TB_ADDSTRINGW") \
    visit(1102, "EM_GETOPTIONS, TB_MAPACCELERATORA") \
    visit(1103, "EM_FINDTEXTEX, TB_GETINSERTMARK") \
    visit(1104, "EM_GETWORDBREAKPROCEX, TB_SETINSERTMARK") \
    visit(1105, "EM_SETWORDBREAKPROCEX, TB_INSERTMARKHITTEST") \
    visit(1106, "EM_SETUNDOLIMIT, TB_MOVEBUTTON") \
    visit(1107, "TB_GETMAXSIZE") \
    visit(1108, "EM_REDO, TB_SETEXTENDEDSTYLE") \
    visit(1109, "EM_CANREDO, TB_GETEXTENDEDSTYLE") \
    visit(1110, "EM_GETUNDONAME, TB_GETPADDING") \
    visit(1111, "EM_GETREDONAME, TB_SETPADDING") \
    visit(1112, "EM_STOPGROUPTYPING, TB_SETINSERTMARKCOLOR") \
    visit(1113, "EM_SETTEXTMODE, TB_GETINSERTMARKCOLOR") \
    visit(1114, "EM_GETTEXTMODE, TB_MAPACCELERATORW") \
    visit(1115, "EM_AUTOURLDETECT, TB_GETSTRINGW") \
    visit(1116, "EM_GETAUTOURLDETECT, TB_GETSTRINGA") \
    visit(1117, "EM_SETPALETTE") \
    visit(1118, "EM_GETTEXTEX") \
    visit(1119, "EM_GETTEXTLENGTHEX") \
    visit(1120, "EM_SHOWSCROLLBAR") \
    visit(1121, "EM_SETTEXTEX") \
    visit(1123, "TAPI_REPLY") \
    visit(1124, "ACM_OPENA, BFFM_SETSTATUSTEXTA, CDM_GETSPEC, EM_SETPUNCTUATION, IPM_CLEARADDRESS, WM_CAP_UNICODE_START") \
    visit(1125, "ACM_PLAY, BFFM_ENABLEOK, CDM_GETFILEPATH, EM_GETPUNCTUATION, IPM_SETADDRESS, PSM_SETCURSEL, UDM_SETRANGE, WM_CHOOSEFONT_SETLOGFONT") \
    visit(1126, "ACM_STOP, BFFM_SETSELECTIONA, CDM_GETFOLDERPATH, EM_SETWORDWRAPMODE, IPM_GETADDRESS, PSM_REMOVEPAGE, UDM_GETRANGE, WM_CAP_SET_CALLBACK_ERRORW, WM_CHOOSEFONT_SETFLAGS") \
    visit(1127, "ACM_OPENW, BFFM_SETSELECTIONW, CDM_GETFOLDERIDLIST, EM_GETWORDWRAPMODE, IPM_SETRANGE, PSM_ADDPAGE, UDM_SETPOS, WM_CAP_SET_CALLBACK_STATUSW") \
    visit(1128, "BFFM_SETSTATUSTEXTW, CDM_SETCONTROLTEXT, EM_SETIMECOLOR, IPM_SETFOCUS, PSM_CHANGED, UDM_GETPOS") \
    visit(1129, "CDM_HIDECONTROL, EM_GETIMECOLOR, IPM_ISBLANK, PSM_RESTARTWINDOWS, UDM_SETBUDDY") \
    visit(1130, "CDM_SETDEFEXT, EM_SETIMEOPTIONS, PSM_REBOOTSYSTEM, UDM_GETBUDDY") \
    visit(1131, "EM_GETIMEOPTIONS, PSM_CANCELTOCLOSE, UDM_SETACCEL") \
    visit(1132, "EM_CONVPOSITION, EM_CONVPOSITION, PSM_QUERYSIBLINGS, UDM_GETACCEL") \
    visit(1133, "MCIWNDM_GETZOOM, PSM_UNCHANGED, UDM_SETBASE") \
    visit(1134, "PSM_APPLY, UDM_GETBASE") \
    visit(1135, "PSM_SETTITLEA, UDM_SETRANGE32") \
    visit(1136, "PSM_SETWIZBUTTONS, UDM_GETRANGE32, WM_CAP_DRIVER_GET_NAMEW") \
    visit(1137, "PSM_PRESSBUTTON, UDM_SETPOS32, WM_CAP_DRIVER_GET_VERSIONW") \
    visit(1138, "PSM_SETCURSELID, UDM_GETPOS32") \
    visit(1139, "PSM_SETFINISHTEXTA") \
    visit(1140, "PSM_GETTABCONTROL") \
    visit(1141, "PSM_ISDIALOGMESSAGE") \
    visit(1142, "MCIWNDM_REALIZE, PSM_GETCURRENTPAGEHWND") \
    visit(1143, "MCIWNDM_SETTIMEFORMATA, PSM_INSERTPAGE") \
    visit(1144, "EM_SETLANGOPTIONS, MCIWNDM_GETTIMEFORMATA, PSM_SETTITLEW, WM_CAP_FILE_SET_CAPTURE_FILEW") \
    visit(1145, "EM_GETLANGOPTIONS, MCIWNDM_VALIDATEMEDIA, PSM_SETFINISHTEXTW, WM_CAP_FILE_GET_CAPTURE_FILEW") \
    visit(1146, "EM_GETIMECOMPMODE") \
    visit(1147, "EM_FINDTEXTW, MCIWNDM_PLAYTO, WM_CAP_FILE_SAVEASW") \
    visit(1148, "EM_FINDTEXTEXW, MCIWNDM_GETFILENAMEA") \
    visit(1149, "EM_RECONVERSION, MCIWNDM_GETDEVICEA, PSM_SETHEADERTITLEA, WM_CAP_FILE_SAVEDIBW") \
    visit(1150, "EM_SETIMEMODEBIAS, MCIWNDM_GETPALETTE, PSM_SETHEADERTITLEW") \
    visit(1151, "EM_GETIMEMODEBIAS, MCIWNDM_SETPALETTE, PSM_SETHEADERSUBTITLEA") \
    visit(1152, "MCIWNDM_GETERRORA, PSM_SETHEADERSUBTITLEW") \
    visit(1153, "PSM_HWNDTOINDEX") \
    visit(1154, "PSM_INDEXTOHWND") \
    visit(1155, "MCIWNDM_SETINACTIVETIMER, PSM_PAGETOINDEX") \
    visit(1156, "PSM_INDEXTOPAGE") \
    visit(1157, "DL_BEGINDRAG, MCIWNDM_GETINACTIVETIMER, PSM_IDTOINDEX") \
    visit(1158, "DL_DRAGGING, PSM_INDEXTOID") \
    visit(1159, "DL_DROPPED, PSM_GETRESULT") \
    visit(1160, "DL_CANCELDRAG, PSM_RECALCPAGESIZES") \
    visit(1164, "MCIWNDM_GET_SOURCE") \
    visit(1165, "MCIWNDM_PUT_SOURCE") \
    visit(1166, "MCIWNDM_GET_DEST") \
    visit(1167, "MCIWNDM_PUT_DEST") \
    visit(1168, "MCIWNDM_CAN_PLAY") \
    visit(1169, "MCIWNDM_CAN_WINDOW") \
    visit(1170, "MCIWNDM_CAN_RECORD") \
    visit(1171, "MCIWNDM_CAN_SAVE") \
    visit(1172, "MCIWNDM_CAN_EJECT") \
    visit(1173, "MCIWNDM_CAN_CONFIG") \
    visit(1174, "IE_GETINK, MCIWNDM_PALETTEKICK") \
    visit(1175, "IE_SETINK") \
    visit(1176, "IE_GETPENTIP") \
    visit(1177, "IE_SETPENTIP") \
    visit(1178, "IE_GETERASERTIP") \
    visit(1179, "IE_SETERASERTIP") \
    visit(1180, "IE_GETBKGND") \
    visit(1181, "IE_SETBKGND") \
    visit(1182, "IE_GETGRIDORIGIN") \
    visit(1183, "IE_SETGRIDORIGIN") \
    visit(1184, "IE_GETGRIDPEN") \
    visit(1185, "IE_SETGRIDPEN") \
    visit(1186, "IE_GETGRIDSIZE") \
    visit(1187, "IE_SETGRIDSIZE") \
    visit(1188, "IE_GETMODE") \
    visit(1189, "IE_SETMODE") \
    visit(1190, "IE_GETINKRECT, WM_CAP_SET_MCI_DEVICEW") \
    visit(1191, "WM_CAP_GET_MCI_DEVICEW") \
    visit(1204, "WM_CAP_PAL_OPENW") \
    visit(1205, "WM_CAP_PAL_SAVEW") \
    visit(1208, "IE_GETAPPDATA") \
    visit(1209, "IE_SETAPPDATA") \
    visit(1210, "IE_GETDRAWOPTS") \
    visit(1211, "IE_SETDRAWOPTS") \
    visit(1212, "IE_GETFORMAT") \
    visit(1213, "IE_SETFORMAT") \
    visit(1214, "IE_GETINKINPUT") \
    visit(1215, "IE_SETINKINPUT") \
    visit(1216, "IE_GETNOTIFY") \
    visit(1217, "IE_SETNOTIFY") \
    visit(1218, "IE_GETRECOG") \
    visit(1219, "IE_SETRECOG") \
    visit(1220, "IE_GETSECURITY") \
    visit(1221, "IE_SETSECURITY") \
    visit(1222, "IE_GETSEL") \
    visit(1223, "IE_SETSEL") \
    visit(1224, "EM_SETBIDIOPTIONS, IE_DOCOMMAND, MCIWNDM_NOTIFYMODE") \
    visit(1225, "EM_GETBIDIOPTIONS, IE_GETCOMMAND") \
    visit(1226, "EM_SETTYPOGRAPHYOPTIONS, IE_GETCOUNT") \
    visit(1227, "EM_GETTYPOGRAPHYOPTIONS, IE_GETGESTURE, MCIWNDM_NOTIFYMEDIA") \
    visit(1228, "EM_SETEDITSTYLE, IE_GETMENU") \
    visit(1229, "EM_GETEDITSTYLE, IE_GETPAINTDC, MCIWNDM_NOTIFYERROR") \
    visit(1230, "IE_GETPDEVENT") \
    visit(1231, "IE_GETSELCOUNT") \
    visit(1232, "IE_GETSELITEMS") \
    visit(1233, "IE_GETSTYLE") \
    visit(1243, "MCIWNDM_SETTIMEFORMATW") \
    visit(1244, "EM_OUTLINE, MCIWNDM_GETTIMEFORMATW") \
    visit(1245, "EM_GETSCROLLPOS") \
    visit(1246, "EM_SETSCROLLPOS, EM_SETSCROLLPOS") \
    visit(1247, "EM_SETFONTSIZE") \
    visit(1248, "EM_GETZOOM, MCIWNDM_GETFILENAMEW") \
    visit(1249, "EM_SETZOOM, MCIWNDM_GETDEVICEW") \
    visit(1250, "EM_GETVIEWKIND") \
    visit(1251, "EM_SETVIEWKIND") \
    visit(1252, "EM_GETPAGE, MCIWNDM_GETERRORW") \
    visit(1253, "EM_SETPAGE") \
    visit(1254, "EM_GETHYPHENATEINFO") \
    visit(1255, "EM_SETHYPHENATEINFO") \
    visit(1259, "EM_GETPAGEROTATE") \
    visit(1260, "EM_SETPAGEROTATE") \
    visit(1261, "EM_GETCTFMODEBIAS") \
    visit(1262, "EM_SETCTFMODEBIAS") \
    visit(1264, "EM_GETCTFOPENSTATUS") \
    visit(1265, "EM_SETCTFOPENSTATUS") \
    visit(1266, "EM_GETIMECOMPTEXT") \
    visit(1267, "EM_ISIME") \
    visit(1268, "EM_GETIMEPROPERTY") \
    visit(1293, "EM_GETQUERYRTFOBJ") \
    visit(1294, "EM_SETQUERYRTFOBJ") \
    visit(1536, "FM_GETFOCUS") \
    visit(1537, "FM_GETDRIVEINFOA") \
    visit(1538, "FM_GETSELCOUNT") \
    visit(1539, "FM_GETSELCOUNTLFN") \
    visit(1540, "FM_GETFILESELA") \
    visit(1541, "FM_GETFILESELLFNA") \
    visit(1542, "FM_REFRESH_WINDOWS") \
    visit(1543, "FM_RELOAD_EXTENSIONS") \
    visit(1553, "FM_GETDRIVEINFOW") \
    visit(1556, "FM_GETFILESELW") \
    visit(1557, "FM_GETFILESELLFNW") \
    visit(1625, "WLX_WM_SAS") \
    visit(2024, "SM_GETSELCOUNT, UM_GETSELCOUNT, WM_CPL_LAUNCH") \
    visit(2025, "SM_GETSERVERSELA, UM_GETUSERSELA, WM_CPL_LAUNCHED") \
    visit(2026, "SM_GETSERVERSELW, UM_GETUSERSELW") \
    visit(2027, "SM_GETCURFOCUSA, UM_GETGROUPSELA") \
    visit(2028, "SM_GETCURFOCUSW, UM_GETGROUPSELW") \
    visit(2029, "SM_GETOPTIONS, UM_GETCURFOCUSA") \
    visit(2030, "UM_GETCURFOCUSW") \
    visit(2031, "UM_GETOPTIONS") \
    visit(2032, "UM_GETOPTIONS2") \
    visit(4096, "LVM_GETBKCOLOR") \
    visit(4097, "LVM_SETBKCOLOR") \
    visit(4098, "LVM_GETIMAGELIST") \
    visit(4099, "LVM_SETIMAGELIST") \
    visit(4100, "LVM_GETITEMCOUNT") \
    visit(4101, "LVM_GETITEMA") \
    visit(4102, "LVM_SETITEMA") \
    visit(4103, "LVM_INSERTITEMA") \
    visit(4104, "LVM_DELETEITEM") \
    visit(4105, "LVM_DELETEALLITEMS") \
    visit(4106, "LVM_GETCALLBACKMASK") \
    visit(4107, "LVM_SETCALLBACKMASK") \
    visit(4108, "LVM_GETNEXTITEM") \
    visit(4109, "LVM_FINDITEMA") \
    visit(4110, "LVM_GETITEMRECT") \
    visit(4111, "LVM_SETITEMPOSITION") \
    visit(4112, "LVM_GETITEMPOSITION") \
    visit(4113, "LVM_GETSTRINGWIDTHA") \
    visit(4114, "LVM_HITTEST") \
    visit(4115, "LVM_ENSUREVISIBLE") \
    visit(4116, "LVM_SCROLL") \
    visit(4117, "LVM_REDRAWITEMS") \
    visit(4118, "LVM_ARRANGE") \
    visit(4119, "LVM_EDITLABELA") \
    visit(4120, "LVM_GETEDITCONTROL") \
    visit(4121, "LVM_GETCOLUMNA") \
    visit(4122, "LVM_SETCOLUMNA") \
    visit(4123, "LVM_INSERTCOLUMNA") \
    visit(4124, "LVM_DELETECOLUMN") \
    visit(4125, "LVM_GETCOLUMNWIDTH") \
    visit(4126, "LVM_SETCOLUMNWIDTH") \
    visit(4127, "LVM_GETHEADER") \
    visit(4129, "LVM_CREATEDRAGIMAGE") \
    visit(4130, "LVM_GETVIEWRECT") \
    visit(4131, "LVM_GETTEXTCOLOR") \
    visit(4132, "LVM_SETTEXTCOLOR") \
    visit(4133, "LVM_GETTEXTBKCOLOR") \
    visit(4134, "LVM_SETTEXTBKCOLOR") \
    visit(4135, "LVM_GETTOPINDEX") \
    visit(4136, "LVM_GETCOUNTPERPAGE") \
    visit(4137, "LVM_GETORIGIN") \
    visit(4138, "LVM_UPDATE") \
    visit(4139, "LVM_SETITEMSTATE") \
    visit(4140, "LVM_GETITEMSTATE") \
    visit(4141, "LVM_GETITEMTEXTA") \
    visit(4142, "LVM_SETITEMTEXTA") \
    visit(4143, "LVM_SETITEMCOUNT") \
    visit(4144, "LVM_SORTITEMS") \
    visit(4145, "LVM_SETITEMPOSITION32") \
    visit(4146, "LVM_GETSELECTEDCOUNT") \
    visit(4147, "LVM_GETITEMSPACING") \
    visit(4148, "LVM_GETISEARCHSTRINGA") \
    visit(4149, "LVM_SETICONSPACING") \
    visit(4150, "LVM_SETEXTENDEDLISTVIEWSTYLE") \
    visit(4151, "LVM_GETEXTENDEDLISTVIEWSTYLE") \
    visit(4152, "LVM_GETSUBITEMRECT") \
    visit(4153, "LVM_SUBITEMHITTEST") \
    visit(4154, "LVM_SETCOLUMNORDERARRAY") \
    visit(4155, "LVM_GETCOLUMNORDERARRAY") \
    visit(4156, "LVM_SETHOTITEM") \
    visit(4157, "LVM_GETHOTITEM") \
    visit(4158, "LVM_SETHOTCURSOR") \
    visit(4159, "LVM_GETHOTCURSOR") \
    visit(4160, "LVM_APPROXIMATEVIEWRECT") \
    visit(4161, "LVM_SETWORKAREAS") \
    visit(4162, "LVM_GETSELECTIONMARK") \
    visit(4163, "LVM_SETSELECTIONMARK") \
    visit(4164, "LVM_SETBKIMAGEA") \
    visit(4165, "LVM_GETBKIMAGEA") \
    visit(4166, "LVM_GETWORKAREAS") \
    visit(4167, "LVM_SETHOVERTIME") \
    visit(4168, "LVM_GETHOVERTIME") \
    visit(4169, "LVM_GETNUMBEROFWORKAREAS") \
    visit(4170, "LVM_SETTOOLTIPS") \
    visit(4171, "LVM_GETITEMW") \
    visit(4172, "LVM_SETITEMW") \
    visit(4173, "LVM_INSERTITEMW") \
    visit(4174, "LVM_GETTOOLTIPS") \
    visit(4179, "LVM_FINDITEMW") \
    visit(4183, "LVM_GETSTRINGWIDTHW") \
    visit(4191, "LVM_GETCOLUMNW") \
    visit(4192, "LVM_SETCOLUMNW") \
    visit(4193, "LVM_INSERTCOLUMNW") \
    visit(4211, "LVM_GETITEMTEXTW") \
    visit(4212, "LVM_SETITEMTEXTW") \
    visit(4213, "LVM_GETISEARCHSTRINGW") \
    visit(4214, "LVM_EDITLABELW") \
    visit(4235, "LVM_GETBKIMAGEW") \
    visit(4236, "LVM_SETSELECTEDCOLUMN") \
    visit(4237, "LVM_SETTILEWIDTH") \
    visit(4238, "LVM_SETVIEW") \
    visit(4239, "LVM_GETVIEW") \
    visit(4241, "LVM_INSERTGROUP") \
    visit(4243, "LVM_SETGROUPINFO") \
    visit(4245, "LVM_GETGROUPINFO") \
    visit(4246, "LVM_REMOVEGROUP") \
    visit(4247, "LVM_MOVEGROUP") \
    visit(4250, "LVM_MOVEITEMTOGROUP") \
    visit(4251, "LVM_SETGROUPMETRICS") \
    visit(4252, "LVM_GETGROUPMETRICS") \
    visit(4253, "LVM_ENABLEGROUPVIEW") \
    visit(4254, "LVM_SORTGROUPS") \
    visit(4255, "LVM_INSERTGROUPSORTED") \
    visit(4256, "LVM_REMOVEALLGROUPS") \
    visit(4257, "LVM_HASGROUP") \
    visit(4258, "LVM_SETTILEVIEWINFO") \
    visit(4259, "LVM_GETTILEVIEWINFO") \
    visit(4260, "LVM_SETTILEINFO") \
    visit(4261, "LVM_GETTILEINFO") \
    visit(4262, "LVM_SETINSERTMARK") \
    visit(4263, "LVM_GETINSERTMARK") \
    visit(4264, "LVM_INSERTMARKHITTEST") \
    visit(4265, "LVM_GETINSERTMARKRECT") \
    visit(4266, "LVM_SETINSERTMARKCOLOR") \
    visit(4267, "LVM_GETINSERTMARKCOLOR") \
    visit(4269, "LVM_SETINFOTIP") \
    visit(4270, "LVM_GETSELECTEDCOLUMN") \
    visit(4271, "LVM_ISGROUPVIEWENABLED") \
    visit(4272, "LVM_GETOUTLINECOLOR") \
    visit(4273, "LVM_SETOUTLINECOLOR") \
    visit(4275, "LVM_CANCELEDITLABEL") \
    visit(4276, "LVM_MAPINDEXTOID") \
    visit(4277, "LVM_MAPIDTOINDEX") \
    visit(4278, "LVM_ISITEMVISIBLE") \
    visit(8192, "OCM__BASE") \
    visit(8197, "LVM_SETUNICODEFORMAT") \
    visit(8198, "LVM_GETUNICODEFORMAT") \
    visit(8217, "OCM_CTLCOLOR") \
    visit(8235, "OCM_DRAWITEM") \
    visit(8236, "OCM_MEASUREITEM") \
    visit(8237, "OCM_DELETEITEM") \
    visit(8238, "OCM_VKEYTOITEM") \
    visit(8239, "OCM_CHARTOITEM") \
    visit(8249, "OCM_COMPAREITEM") \
    visit(8270, "OCM_NOTIFY") \
    visit(8465, "OCM_COMMAND") \
    visit(8468, "OCM_HSCROLL") \
    visit(8469, "OCM_VSCROLL") \
    visit(8498, "OCM_CTLCOLORMSGBOX") \
    visit(8499, "OCM_CTLCOLOREDIT") \
    visit(8500, "OCM_CTLCOLORLISTBOX") \
    visit(8501, "OCM_CTLCOLORBTN") \
    visit(8502, "OCM_CTLCOLORDLG") \
    visit(8503, "OCM_CTLCOLORSCROLLBAR") \
    visit(8504, "OCM_CTLCOLORSTATIC") \
    visit(8720, "OCM_PARENTNOTIFY") \
    visit(32768, "WM_APP") \
    visit(52429, "WM_RASDIALEVENT")

#define VISIT_WMMSG_RETURN(num, str) \
	if (Id == num) \
	{ \
		return os << str; \
	}

	VISIT_WMMSG_CODES(VISIT_WMMSG_RETURN);

	return os << Logging::hex(Id);
}

namespace
{
	template <typename CreateStruct>
	std::ostream& logCreateStruct(std::ostream& os, const CreateStruct& cs)
	{
		return Logging::LogStruct(os)
			<< Logging::hex(cs.dwExStyle)
			<< cs.lpszClass
			<< cs.lpszName
			<< Logging::hex(cs.style)
			<< cs.x
			<< cs.y
			<< cs.cx
			<< cs.cy
			<< cs.hwndParent
			<< cs.hMenu
			<< cs.hInstance
			<< cs.lpCreateParams;
	}

	template <typename DevMode>
	std::ostream& logDevMode(std::ostream& os, const DevMode& dm)
	{
		return Logging::LogStruct(os)
			<< dm.dmPelsWidth
			<< dm.dmPelsHeight
			<< dm.dmBitsPerPel
			<< dm.dmDisplayFrequency
			<< dm.dmDisplayFlags;
	}

	template <typename MdiCreateStruct>
	std::ostream& logMdiCreateStruct(std::ostream& os, const MdiCreateStruct& mcs)
	{
		return Logging::LogStruct(os)
			<< mcs.szClass
			<< mcs.szTitle
			<< mcs.hOwner
			<< mcs.x
			<< mcs.y
			<< mcs.cx
			<< mcs.cy
			<< Logging::hex(mcs.style)
			<< Logging::hex(mcs.lParam);
	}
}

std::ostream& operator<<(std::ostream& os, const COMPAREITEMSTRUCT& cis)
{
	return Logging::LogStruct(os)
		<< cis.CtlType
		<< cis.CtlID
		<< cis.hwndItem
		<< cis.itemID1
		<< Logging::hex(cis.itemData1)
		<< cis.itemID2
		<< Logging::hex(cis.itemData2)
		<< Logging::hex(cis.dwLocaleId);
}

std::ostream& operator<<(std::ostream& os, const COPYDATASTRUCT& cds)
{
	return Logging::LogStruct(os)
		<< Logging::hex(cds.dwData)
		<< cds.cbData
		<< cds.lpData;
}

std::ostream& operator<<(std::ostream& os, const CREATESTRUCTA& cs)
{
	return logCreateStruct(os, cs);
}

std::ostream& operator<<(std::ostream& os, const CREATESTRUCTW& cs)
{
	return logCreateStruct(os, cs);
}

std::ostream& operator<<(std::ostream& os, const CWPSTRUCT& cwp)
{
	return Logging::LogStruct(os)
		<< Logging::hex(cwp.message)
		<< cwp.hwnd
		<< Logging::hex(cwp.wParam)
		<< Logging::hex(cwp.lParam);
}

std::ostream& operator<<(std::ostream& os, const CWPRETSTRUCT& cwrp)
{
	return Logging::LogStruct(os)
		<< Logging::hex(cwrp.message)
		<< cwrp.hwnd
		<< Logging::hex(cwrp.wParam)
		<< Logging::hex(cwrp.lParam)
		<< Logging::hex(cwrp.lResult);
}

std::ostream& operator<<(std::ostream& os, const DELETEITEMSTRUCT& dis)
{
	return Logging::LogStruct(os)
		<< dis.CtlType
		<< dis.CtlID
		<< dis.itemID
		<< dis.hwndItem
		<< Logging::hex(dis.itemData);
}

std::ostream& operator<<(std::ostream& os, const DEVMODEA& dm)
{
	return logDevMode(os, dm);
}

std::ostream& operator<<(std::ostream& os, const DEVMODEW& dm)
{
	return logDevMode(os, dm);
}

std::ostream& operator<<(std::ostream& os, const DRAWITEMSTRUCT& dis)
{
	return Logging::LogStruct(os)
		<< dis.CtlType
		<< dis.CtlID
		<< dis.itemID
		<< Logging::hex(dis.itemAction)
		<< Logging::hex(dis.itemState)
		<< dis.hwndItem
		<< dis.hDC
		<< dis.rcItem
		<< Logging::hex(dis.itemData);
}

std::ostream& operator<<(std::ostream& os, const GESTURENOTIFYSTRUCT& gns)
{
	return Logging::LogStruct(os)
		<< gns.cbSize
		<< Logging::hex(gns.dwFlags)
		<< gns.hwndTarget
		<< gns.ptsLocation
		<< gns.dwInstanceID;
}

std::ostream& operator<<(std::ostream& os, HDC__& dc)
{
	return os << "DC(" << static_cast<void*>(&dc) << ',' << WindowFromDC(&dc) << ')';
}

std::ostream& operator<<(std::ostream& os, const HELPINFO& hi)
{
	Logging::LogStruct log(os);
	log << hi.cbSize
		<< hi.iContextType
		<< hi.iCtrlId;

	if (HELPINFO_WINDOW == hi.iContextType)
	{
		log << static_cast<HWND>(hi.hItemHandle);
	}
	else
	{
		log << static_cast<HMENU>(hi.hItemHandle);
	}

	return log
		<< hi.dwContextId
		<< hi.MousePos;
}

std::ostream& operator<<(std::ostream& os, HFONT font)
{
	LOGFONT lf = {};
	if (font)
	{
		GetObject(font, sizeof(lf), &lf);
	}
	return Logging::LogStruct(os)
		<< static_cast<void*>(font)
		<< (font ? &lf : nullptr);
}

std::ostream& operator<<(std::ostream& os, HRGN rgn)
{
	os << "RGN";
	if (!rgn)
	{
		return os << "(null)";
	}

	DWORD size = GetRegionData(rgn, 0, nullptr);
	if (0 == size)
	{
		return os << "[]";
	}

	std::vector<unsigned char> rgnDataBuf(size);
	auto& rgnData = *reinterpret_cast<RGNDATA*>(rgnDataBuf.data());
	GetRegionData(rgn, size, &rgnData);

	return os << Logging::array(reinterpret_cast<RECT*>(rgnData.Buffer), rgnData.rdh.nCount);
}

std::ostream& operator<<(std::ostream& os, HWND__& hwnd)
{
	char name[256] = {};
	GetClassNameA(&hwnd, name, sizeof(name));
	RECT rect = {};
	GetWindowRect(&hwnd, &rect);
	return os << "WND(" << static_cast<void*>(&hwnd) << ',' << name << ',' << rect << ')';
}

std::ostream& operator<<(std::ostream& os, const LOGFONT& lf)
{
	return Logging::LogStruct(os)
		<< lf.lfHeight
		<< lf.lfWidth
		<< lf.lfEscapement
		<< lf.lfOrientation
		<< lf.lfWeight
		<< static_cast<UINT>(lf.lfItalic)
		<< static_cast<UINT>(lf.lfUnderline)
		<< static_cast<UINT>(lf.lfStrikeOut)
		<< static_cast<UINT>(lf.lfCharSet)
		<< static_cast<UINT>(lf.lfOutPrecision)
		<< static_cast<UINT>(lf.lfClipPrecision)
		<< static_cast<UINT>(lf.lfQuality)
		<< Logging::hex<UINT>(lf.lfPitchAndFamily)
		<< lf.lfFaceName;
}

std::ostream& operator<<(std::ostream& os, const MDICREATESTRUCTA& mcs)
{
	return logMdiCreateStruct(os, mcs);
}

std::ostream& operator<<(std::ostream& os, const MDICREATESTRUCTW& mcs)
{
	return logMdiCreateStruct(os, mcs);
}

std::ostream& operator<<(std::ostream& os, const MDINEXTMENU& mnm)
{
	return Logging::LogStruct(os)
		<< mnm.hmenuIn
		<< mnm.hmenuNext
		<< mnm.hwndNext;
}

std::ostream& operator<<(std::ostream& os, const MEASUREITEMSTRUCT& mis)
{
	return Logging::LogStruct(os)
		<< mis.CtlType
		<< mis.CtlID
		<< mis.itemID
		<< mis.itemWidth
		<< mis.itemHeight
		<< Logging::hex(mis.itemData);
}

std::ostream& operator<<(std::ostream& os, const MEMORYSTATUS& ms)
{
	return Logging::LogStruct(os)
		<< ms.dwLength
		<< ms.dwMemoryLoad
		<< ms.dwTotalPhys
		<< ms.dwAvailPhys
		<< ms.dwTotalPageFile
		<< ms.dwAvailPageFile
		<< ms.dwTotalVirtual
		<< ms.dwAvailVirtual;
}

std::ostream& operator<<(std::ostream& os, const MENUGETOBJECTINFO& mgoi)
{
	return Logging::LogStruct(os)
		<< Logging::hex(mgoi.dwFlags)
		<< mgoi.uPos
		<< mgoi.hmenu
		<< static_cast<GUID*>(mgoi.riid)
		<< mgoi.pvObj;
}

std::ostream& operator<<(std::ostream& os, const MINMAXINFO& mmi)
{
	return Logging::LogStruct(os)
		<< mmi.ptReserved
		<< mmi.ptMaxSize
		<< mmi.ptMaxPosition
		<< mmi.ptMinTrackSize
		<< mmi.ptMaxTrackSize;
}

std::ostream& operator<<(std::ostream& os, const MSG& msg)
{
	return Logging::LogStruct(os)
		<< msg.hwnd
		<< Logging::hex(msg.message)
		<< Logging::hex(msg.wParam)
		<< Logging::hex(msg.lParam)
		<< msg.time
		<< msg.pt;
}

std::ostream& operator<<(std::ostream& os, const NCCALCSIZE_PARAMS& nccs)
{
	return Logging::LogStruct(os)
		<< Logging::array(nccs.rgrc, sizeof(nccs.rgrc) / sizeof(nccs.rgrc[0]))
		<< nccs.lppos;
}

std::ostream& operator<<(std::ostream& os, const NMHDR& nm)
{
	return Logging::LogStruct(os)
		<< nm.hwndFrom
		<< nm.idFrom
		<< Logging::hex(nm.code);
}

std::ostream& operator<<(std::ostream& os, const POINT& p)
{
	return Logging::LogStruct(os)
		<< p.x
		<< p.y;
}

std::ostream& operator<<(std::ostream& os, const POINTS& p)
{
	return Logging::LogStruct(os)
		<< p.x
		<< p.y;
}

std::ostream& operator<<(std::ostream& os, const RECT& rect)
{
	return Logging::LogStruct(os)
		<< rect.left
		<< rect.top
		<< rect.right
		<< rect.bottom;
}

std::ostream& operator<<(std::ostream& os, const SIZE& size)
{
	return Logging::LogStruct(os)
		<< size.cx
		<< size.cy;
}

std::ostream& operator<<(std::ostream& os, const STYLESTRUCT& ss)
{
	return Logging::LogStruct(os)
		<< Logging::hex(ss.styleOld)
		<< Logging::hex(ss.styleNew);
}

std::ostream& operator<<(std::ostream& os, const TITLEBARINFOEX& tbi)
{
	return Logging::LogStruct(os)
		<< tbi.cbSize
		<< tbi.rcTitleBar
		<< Logging::array(tbi.rgstate, sizeof(tbi.rgstate) / sizeof(tbi.rgstate[0]))
		<< Logging::array(tbi.rgrect, sizeof(tbi.rgrect) / sizeof(tbi.rgrect[0]));
}

std::ostream& operator<<(std::ostream& os, const TOUCH_HIT_TESTING_INPUT& thti)
{
	return Logging::LogStruct(os)
		<< thti.pointerId
		<< thti.point
		<< thti.boundingBox
		<< thti.nonOccludedBoundingBox
		<< thti.orientation;
}

std::ostream& operator<<(std::ostream& os, const WINDOWPOS& wp)
{
	return Logging::LogStruct(os)
		<< wp.hwnd
		<< wp.hwndInsertAfter
		<< wp.x
		<< wp.y
		<< wp.cx
		<< wp.cy
		<< Logging::hex(wp.flags);
}
