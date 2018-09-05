#define WIN32_LEAN_AND_MEAN

#include <atlstr.h>
#include <Windows.h>

#include "DDrawCompat\DDrawLog.h"

#ifndef __DDRAW_INCLUDED__
DEFINE_GUID(CLSID_DirectDraw, 0xD7B70EE0, 0x4340, 0x11CF, 0xB0, 0x63, 0x00, 0x20, 0xAF, 0xC2, 0xCD, 0x35);
DEFINE_GUID(CLSID_DirectDraw7, 0x3C305196, 0x50DB, 0x11D3, 0x9C, 0xFE, 0x00, 0xC0, 0x4F, 0xD9, 0x30, 0xC5);
DEFINE_GUID(CLSID_DirectDrawClipper, 0x593817A0, 0x7DB3, 0x11CF, 0xA2, 0xDE, 0x00, 0xAA, 0x00, 0xb9, 0x33, 0x56);
DEFINE_GUID(IID_IDirectDraw, 0x6C14DB80, 0xA733, 0x11CE, 0xA5, 0x21, 0x00, 0x20, 0xAF, 0x0B, 0xE5, 0x60);
DEFINE_GUID(IID_IDirectDraw2, 0xB3A6F3E0, 0x2B43, 0x11CF, 0xA2, 0xDE, 0x00, 0xAA, 0x00, 0xB9, 0x33, 0x56);
DEFINE_GUID(IID_IDirectDraw4, 0x9c59509a, 0x39bd, 0x11d1, 0x8c, 0x4a, 0x00, 0xc0, 0x4f, 0xd9, 0x30, 0xc5);
DEFINE_GUID(IID_IDirectDraw7, 0x15e65ec0, 0x3b9c, 0x11d2, 0xb9, 0x2f, 0x00, 0x60, 0x97, 0x97, 0xea, 0x5b);
DEFINE_GUID(IID_IDirectDrawSurface, 0x6C14DB81, 0xA733, 0x11CE, 0xA5, 0x21, 0x00, 0x20, 0xAF, 0x0B, 0xE5, 0x60);
DEFINE_GUID(IID_IDirectDrawSurface2, 0x57805885, 0x6eec, 0x11cf, 0x94, 0x41, 0xa8, 0x23, 0x03, 0xc1, 0x0e, 0x27);
DEFINE_GUID(IID_IDirectDrawSurface3, 0xDA044E00, 0x69B2, 0x11D0, 0xA1, 0xD5, 0x00, 0xAA, 0x00, 0xB8, 0xDF, 0xBB);
DEFINE_GUID(IID_IDirectDrawSurface4, 0x0B2B8630, 0xAD35, 0x11D0, 0x8E, 0xA6, 0x00, 0x60, 0x97, 0x97, 0xEA, 0x5B);
DEFINE_GUID(IID_IDirectDrawSurface7, 0x06675a80, 0x3b9b, 0x11d2, 0xb9, 0x2f, 0x00, 0x60, 0x97, 0x97, 0xea, 0x5b);
DEFINE_GUID(IID_IDirectDrawPalette, 0x6C14DB84, 0xA733, 0x11CE, 0xA5, 0x21, 0x00, 0x20, 0xAF, 0x0B, 0xE5, 0x60);
DEFINE_GUID(IID_IDirectDrawClipper, 0x6C14DB85, 0xA733, 0x11CE, 0xA5, 0x21, 0x00, 0x20, 0xAF, 0x0B, 0xE5, 0x60);
DEFINE_GUID(IID_IDirectDrawColorControl, 0x4B9F0EE0, 0x0D7E, 0x11D0, 0x9B, 0x06, 0x00, 0xA0, 0xC9, 0x03, 0xA3, 0xB8);
DEFINE_GUID(IID_IDirectDrawGammaControl, 0x69C11C3E, 0xB46B, 0x11D1, 0xAD, 0x7A, 0x00, 0xC0, 0x4F, 0xC2, 0x9B, 0x4E);
#endif
#ifndef __DDRAWEXH__
DEFINE_GUID(IID_IDirectDraw3, 0x618f8ad4, 0x8b7a, 0x11d0, 0x8f, 0xcc, 0x0, 0xc0, 0x4f, 0xd9, 0x18, 0x9d);
DEFINE_GUID(CLSID_DirectDrawFactory, 0x4fd2a832, 0x86c8, 0x11d0, 0x8f, 0xca, 0x0, 0xc0, 0x4f, 0xd9, 0x18, 0x9d);
DEFINE_GUID(IID_IDirectDrawFactory, 0x4fd2a833, 0x86c8, 0x11d0, 0x8f, 0xca, 0x0, 0xc0, 0x4f, 0xd9, 0x18, 0x9d);
#endif
#ifndef _D3D_H_
DEFINE_GUID(IID_IDirect3D, 0x3BBA0080, 0x2421, 0x11CF, 0xA3, 0x1A, 0x00, 0xAA, 0x00, 0xB9, 0x33, 0x56);
DEFINE_GUID(IID_IDirect3D2, 0x6aae1ec1, 0x662a, 0x11d0, 0x88, 0x9d, 0x00, 0xaa, 0x00, 0xbb, 0xb7, 0x6a);
DEFINE_GUID(IID_IDirect3D3, 0xbb223240, 0xe72b, 0x11d0, 0xa9, 0xb4, 0x00, 0xaa, 0x00, 0xc0, 0x99, 0x3e);
DEFINE_GUID(IID_IDirect3D7, 0xf5049e77, 0x4861, 0x11d2, 0xa4, 0x7, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0xa8);
DEFINE_GUID(IID_IDirect3DRampDevice, 0xF2086B20, 0x259F, 0x11CF, 0xA3, 0x1A, 0x00, 0xAA, 0x00, 0xB9, 0x33, 0x56);
DEFINE_GUID(IID_IDirect3DRGBDevice, 0xA4665C60, 0x2673, 0x11CF, 0xA3, 0x1A, 0x00, 0xAA, 0x00, 0xB9, 0x33, 0x56);
DEFINE_GUID(IID_IDirect3DHALDevice, 0x84E63dE0, 0x46AA, 0x11CF, 0x81, 0x6F, 0x00, 0x00, 0xC0, 0x20, 0x15, 0x6E);
DEFINE_GUID(IID_IDirect3DMMXDevice, 0x881949a1, 0xd6f3, 0x11d0, 0x89, 0xab, 0x00, 0xa0, 0xc9, 0x05, 0x41, 0x29);
DEFINE_GUID(IID_IDirect3DRefDevice, 0x50936643, 0x13e9, 0x11d1, 0x89, 0xaa, 0x0, 0xa0, 0xc9, 0x5, 0x41, 0x29);
DEFINE_GUID(IID_IDirect3DNullDevice, 0x8767df22, 0xbacc, 0x11d1, 0x89, 0x69, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0xa8);
DEFINE_GUID(IID_IDirect3DTnLHalDevice, 0xf5049e78, 0x4861, 0x11d2, 0xa4, 0x7, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0xa8);
DEFINE_GUID(IID_IDirect3DDevice, 0x64108800, 0x957d, 0X11d0, 0x89, 0xab, 0x00, 0xa0, 0xc9, 0x05, 0x41, 0x29);
DEFINE_GUID(IID_IDirect3DDevice2, 0x93281501, 0x8cf8, 0x11d0, 0x89, 0xab, 0x0, 0xa0, 0xc9, 0x5, 0x41, 0x29);
DEFINE_GUID(IID_IDirect3DDevice3, 0xb0ab3b60, 0x33d7, 0x11d1, 0xa9, 0x81, 0x0, 0xc0, 0x4f, 0xd7, 0xb1, 0x74);
DEFINE_GUID(IID_IDirect3DDevice7, 0xf5049e79, 0x4861, 0x11d2, 0xa4, 0x7, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0xa8);
DEFINE_GUID(IID_IDirect3DTexture, 0x2CDCD9E0, 0x25A0, 0x11CF, 0xA3, 0x1A, 0x00, 0xAA, 0x00, 0xB9, 0x33, 0x56);
DEFINE_GUID(IID_IDirect3DTexture2, 0x93281502, 0x8cf8, 0x11d0, 0x89, 0xab, 0x0, 0xa0, 0xc9, 0x5, 0x41, 0x29);
DEFINE_GUID(IID_IDirect3DLight, 0x4417C142, 0x33AD, 0x11CF, 0x81, 0x6F, 0x00, 0x00, 0xC0, 0x20, 0x15, 0x6E);
DEFINE_GUID(IID_IDirect3DMaterial, 0x4417C144, 0x33AD, 0x11CF, 0x81, 0x6F, 0x00, 0x00, 0xC0, 0x20, 0x15, 0x6E);
DEFINE_GUID(IID_IDirect3DMaterial2, 0x93281503, 0x8cf8, 0x11d0, 0x89, 0xab, 0x0, 0xa0, 0xc9, 0x5, 0x41, 0x29);
DEFINE_GUID(IID_IDirect3DMaterial3, 0xca9c46f4, 0xd3c5, 0x11d1, 0xb7, 0x5a, 0x0, 0x60, 0x8, 0x52, 0xb3, 0x12);
DEFINE_GUID(IID_IDirect3DExecuteBuffer, 0x4417C145, 0x33AD, 0x11CF, 0x81, 0x6F, 0x00, 0x00, 0xC0, 0x20, 0x15, 0x6E);
DEFINE_GUID(IID_IDirect3DViewport, 0x4417C146, 0x33AD, 0x11CF, 0x81, 0x6F, 0x00, 0x00, 0xC0, 0x20, 0x15, 0x6E);
DEFINE_GUID(IID_IDirect3DViewport2, 0x93281500, 0x8cf8, 0x11d0, 0x89, 0xab, 0x0, 0xa0, 0xc9, 0x5, 0x41, 0x29);
DEFINE_GUID(IID_IDirect3DViewport3, 0xb0ab3b61, 0x33d7, 0x11d1, 0xa9, 0x81, 0x0, 0xc0, 0x4f, 0xd7, 0xb1, 0x74);
DEFINE_GUID(IID_IDirect3DVertexBuffer, 0x7a503555, 0x4a83, 0x11d1, 0xa5, 0xdb, 0x0, 0xa0, 0xc9, 0x3, 0x67, 0xf8);
DEFINE_GUID(IID_IDirect3DVertexBuffer7, 0xf5049e7d, 0x4861, 0x11d2, 0xa4, 0x7, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0xa8);
#endif
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
#ifndef _D3D9_H_
DEFINE_GUID(IID_IDirect3D9, 0x81bdcbca, 0x64d4, 0x426d, 0xae, 0x8d, 0xad, 0x1, 0x47, 0xf4, 0x27, 0x5c);
DEFINE_GUID(IID_IDirect3DDevice9, 0xd0223b96, 0xbf7a, 0x43fd, 0x92, 0xbd, 0xa4, 0x3b, 0xd, 0x82, 0xb9, 0xeb);
DEFINE_GUID(IID_IDirect3DResource9, 0x5eec05d, 0x8f7d, 0x4362, 0xb9, 0x99, 0xd1, 0xba, 0xf3, 0x57, 0xc7, 0x4);
DEFINE_GUID(IID_IDirect3DBaseTexture9, 0x580ca87e, 0x1d3c, 0x4d54, 0x99, 0x1d, 0xb7, 0xd3, 0xe3, 0xc2, 0x98, 0xce);
DEFINE_GUID(IID_IDirect3DTexture9, 0x85c31227, 0x3de5, 0x4f00, 0x9b, 0x3a, 0xf1, 0x1a, 0xc3, 0x8c, 0x18, 0xb5);
DEFINE_GUID(IID_IDirect3DCubeTexture9, 0xfff32f81, 0xd953, 0x473a, 0x92, 0x23, 0x93, 0xd6, 0x52, 0xab, 0xa9, 0x3f);
DEFINE_GUID(IID_IDirect3DVolumeTexture9, 0x2518526c, 0xe789, 0x4111, 0xa7, 0xb9, 0x47, 0xef, 0x32, 0x8d, 0x13, 0xe6);
DEFINE_GUID(IID_IDirect3DVertexBuffer9, 0xb64bb1b5, 0xfd70, 0x4df6, 0xbf, 0x91, 0x19, 0xd0, 0xa1, 0x24, 0x55, 0xe3);
DEFINE_GUID(IID_IDirect3DIndexBuffer9, 0x7c9dd65e, 0xd3f7, 0x4529, 0xac, 0xee, 0x78, 0x58, 0x30, 0xac, 0xde, 0x35);
DEFINE_GUID(IID_IDirect3DSurface9, 0xcfbaf3a, 0x9ff6, 0x429a, 0x99, 0xb3, 0xa2, 0x79, 0x6a, 0xf8, 0xb8, 0x9b);
DEFINE_GUID(IID_IDirect3DVolume9, 0x24f416e6, 0x1f67, 0x4aa7, 0xb8, 0x8e, 0xd3, 0x3f, 0x6f, 0x31, 0x28, 0xa1);
DEFINE_GUID(IID_IDirect3DSwapChain9, 0x794950f2, 0xadfc, 0x458a, 0x90, 0x5e, 0x10, 0xa1, 0xb, 0xb, 0x50, 0x3b);
DEFINE_GUID(IID_IDirect3DVertexDeclaration9, 0xdd13c59c, 0x36fa, 0x4098, 0xa8, 0xfb, 0xc7, 0xed, 0x39, 0xdc, 0x85, 0x46);
DEFINE_GUID(IID_IDirect3DVertexShader9, 0xefc5557e, 0x6265, 0x4613, 0x8a, 0x94, 0x43, 0x85, 0x78, 0x89, 0xeb, 0x36);
DEFINE_GUID(IID_IDirect3DPixelShader9, 0x6d3bdbdc, 0x5b02, 0x4415, 0xb8, 0x52, 0xce, 0x5e, 0x8b, 0xcc, 0xb2, 0x89);
DEFINE_GUID(IID_IDirect3DStateBlock9, 0xb07c4fe5, 0x310d, 0x4ba8, 0xa2, 0x3c, 0x4f, 0xf, 0x20, 0x6f, 0x21, 0x8b);
DEFINE_GUID(IID_IDirect3DQuery9, 0xd9771460, 0xa695, 0x4f26, 0xbb, 0xd3, 0x27, 0xb8, 0x40, 0xb5, 0x41, 0xcc);
DEFINE_GUID(IID_HelperName, 0xe4a36723, 0xfdfe, 0x4b22, 0xb1, 0x46, 0x3c, 0x4, 0xc0, 0x7f, 0x4c, 0xc8);
DEFINE_GUID(IID_IDirect3D9Ex, 0x02177241, 0x69FC, 0x400C, 0x8F, 0xF1, 0x93, 0xA4, 0x4D, 0xF6, 0x86, 0x1D);
DEFINE_GUID(IID_IDirect3DDevice9Ex, 0xb18b10ce, 0x2649, 0x405a, 0x87, 0xf, 0x95, 0xf7, 0x77, 0xd4, 0x31, 0x3a);
DEFINE_GUID(IID_IDirect3DSwapChain9Ex, 0x91886caf, 0x1c3d, 0x4d2e, 0xa0, 0xab, 0x3e, 0x4c, 0x7d, 0x8d, 0x33, 0x3);
DEFINE_GUID(IID_IDirect3D9ExOverlayExtension, 0x187aeb13, 0xaaf5, 0x4c59, 0x87, 0x6d, 0xe0, 0x59, 0x8, 0x8c, 0xd, 0xf8);
DEFINE_GUID(IID_IDirect3DDevice9Video, 0x26dc4561, 0xa1ee, 0x4ae7, 0x96, 0xda, 0x11, 0x8a, 0x36, 0xc0, 0xec, 0x95);
DEFINE_GUID(IID_IDirect3DAuthenticatedChannel9, 0xff24beee, 0xda21, 0x4beb, 0x98, 0xb5, 0xd2, 0xf8, 0x99, 0xf9, 0x8a, 0xf9);
DEFINE_GUID(IID_IDirect3DCryptoSession9, 0xfa0ab799, 0x7a9c, 0x48ca, 0x8c, 0x5b, 0x23, 0x7e, 0x71, 0xa5, 0x44, 0x34);
#endif

namespace
{
	template <typename DevMode>
	std::ostream& streamDevMode(std::ostream& os, const DevMode& dm)
	{
		return Compat::LogStruct(os)
			<< dm.dmPelsWidth
			<< dm.dmPelsHeight
			<< dm.dmBitsPerPel
			<< dm.dmDisplayFrequency
			<< dm.dmDisplayFlags;
	}
}

std::ostream& operator<<(std::ostream& os, const char* str)
{
	if (!str)
	{
		return os << "null";
	}

	if (!Compat::Log::isPointerDereferencingAllowed())
	{
		return os << static_cast<const void*>(str);
	}

	return os.write(str, strlen(str));
}

std::ostream& operator<<(std::ostream& os, const unsigned char* data)
{
	return os << static_cast<const void*>(data);
}

std::ostream& operator<<(std::ostream& os, const WCHAR* wstr)
{
	if (!wstr)
	{
		return os << "null";
	}

	if (!Compat::Log::isPointerDereferencingAllowed())
	{
		return os << static_cast<const void*>(wstr);
	}

	CStringA str(wstr);
	return os << '"' << static_cast<const char*>(str) << '"';
}

std::ostream& operator<<(std::ostream& os, const DEVMODEA& dm)
{
	return streamDevMode(os, dm);
}

std::ostream& operator<<(std::ostream& os, const DEVMODEW& dm)
{
	return streamDevMode(os, dm);
}

std::ostream& operator<<(std::ostream& os, const RECT& rect)
{
	return Compat::LogStruct(os)
		<< rect.left
		<< rect.top
		<< rect.right
		<< rect.bottom;
}

std::ostream& operator<<(std::ostream& os, HDC__& dc)
{
	return os << "DC(" << static_cast<void*>(&dc) << ',' << WindowFromDC(&dc) << ')';
}

std::ostream& operator<<(std::ostream& os, HWND__& hwnd)
{
	char name[256] = {};
	GetClassName(&hwnd, name, sizeof(name));
	RECT rect = {};
	GetWindowRect(&hwnd, &rect);
	return os << "WND(" << static_cast<void*>(&hwnd) << ',' << name << ',' << rect << ')';
}

std::ostream& operator<<(std::ostream& os, const DDCAPS& caps)
{
	return Compat::LogStruct(os)
		<< caps.dwSize
		<< Compat::hex(caps.dwCaps)
		<< Compat::hex(caps.dwCaps2)
		<< Compat::hex(caps.dwCKeyCaps)
		<< Compat::hex(caps.dwFXCaps)
		<< Compat::hex(caps.dwFXAlphaCaps)
		<< Compat::hex(caps.dwPalCaps)
		<< Compat::hex(caps.dwSVCaps)
		<< Compat::hex(caps.dwAlphaBltConstBitDepths)
		<< Compat::hex(caps.dwAlphaBltPixelBitDepths)
		<< Compat::hex(caps.dwAlphaBltSurfaceBitDepths)
		<< Compat::hex(caps.dwAlphaOverlayConstBitDepths)
		<< Compat::hex(caps.dwAlphaOverlayPixelBitDepths)
		<< Compat::hex(caps.dwAlphaOverlaySurfaceBitDepths)
		<< Compat::hex(caps.dwZBufferBitDepths)
		<< Compat::hex(caps.dwVidMemTotal)
		<< Compat::hex(caps.dwVidMemFree)
		<< Compat::hex(caps.dwMaxVisibleOverlays)
		<< Compat::hex(caps.dwCurrVisibleOverlays)
		<< Compat::hex(caps.dwNumFourCCCodes)
		<< Compat::hex(caps.dwAlignBoundarySrc)
		<< Compat::hex(caps.dwAlignSizeSrc)
		<< Compat::hex(caps.dwAlignBoundaryDest)
		<< Compat::hex(caps.dwAlignSizeDest)
		<< Compat::hex(caps.dwAlignStrideAlign)
		<< "{" << caps.dwRops[0]
		<< caps.dwRops[1]
		<< caps.dwRops[2]
		<< caps.dwRops[3]
		<< caps.dwRops[4]
		<< caps.dwRops[5]
		<< caps.dwRops[6]
		<< caps.dwRops[7] << "}"
		<< caps.ddsOldCaps
		<< caps.ddsCaps
		<< Compat::hex(caps.dwMinOverlayStretch)
		<< Compat::hex(caps.dwMaxOverlayStretch)
		<< Compat::hex(caps.dwMinLiveVideoStretch)
		<< Compat::hex(caps.dwMaxLiveVideoStretch)
		<< Compat::hex(caps.dwMinHwCodecStretch)
		<< Compat::hex(caps.dwMaxHwCodecStretch);
}

std::ostream& operator<<(std::ostream& os, const DDSCAPS& dwCaps)
{
	return Compat::LogStructNull(os)
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
	return Compat::LogStruct(os)
		<< *(LPDDSCAPS)&caps
		<< Compat::hex(caps.dwCaps2)
		<< Compat::hex(caps.dwCaps3)
		<< Compat::hex(caps.dwCaps4);
}

std::ostream& operator<<(std::ostream& os, const DDPIXELFORMAT& pf)
{
	return Compat::LogStruct(os)
		<< Compat::hex(pf.dwFlags)
		<< Compat::hex(pf.dwFourCC)
		<< pf.dwRGBBitCount
		<< Compat::hex(pf.dwRBitMask)
		<< Compat::hex(pf.dwGBitMask)
		<< Compat::hex(pf.dwBBitMask)
		<< Compat::hex(pf.dwRGBAlphaBitMask);
}

std::ostream& operator<<(std::ostream& os, const DDSURFACEDESC& sd)
{
	DDSURFACEDESC2 sd2 = {};
	memcpy(&sd2, &sd, sizeof(sd));
	return os << sd2;
}

std::ostream& operator<<(std::ostream& os, const DDSURFACEDESC2& sd)
{
	return Compat::LogStruct(os)
		<< Compat::hex(sd.dwFlags)
		<< sd.dwWidth 
		<< sd.dwHeight
		<< sd.lPitch
		<< sd.dwBackBufferCount 
		<< sd.dwMipMapCount
		<< sd.dwAlphaBitDepth 
		<< sd.dwReserved 
		<< sd.lpSurface
		<< sd.ddpfPixelFormat
		<< sd.ddsCaps
		<< sd.dwTextureStage;
}

std::ostream& operator<<(std::ostream& os, const CWPSTRUCT& cwrp)
{
	return Compat::LogStruct(os)
		<< Compat::hex(cwrp.message)
		<< cwrp.hwnd 
		<< Compat::hex(cwrp.wParam)
		<< Compat::hex(cwrp.lParam);
}

std::ostream& operator<<(std::ostream& os, const CWPRETSTRUCT& cwrp)
{
	return Compat::LogStruct(os) 
		<< Compat::hex(cwrp.message)
		<< cwrp.hwnd 
		<< Compat::hex(cwrp.wParam)
		<< Compat::hex(cwrp.lParam)
		<< Compat::hex(cwrp.lResult);
}

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

	return Compat::LogStruct(os)
		<< Compat::hex(riid.Data1)
		<< Compat::hex(riid.Data2)
		<< Compat::hex(riid.Data3)
		<< Compat::hex((UINT)riid.Data4[0])
		<< Compat::hex((UINT)riid.Data4[1])
		<< Compat::hex((UINT)riid.Data4[2])
		<< Compat::hex((UINT)riid.Data4[3])
		<< Compat::hex((UINT)riid.Data4[4])
		<< Compat::hex((UINT)riid.Data4[5])
		<< Compat::hex((UINT)riid.Data4[6])
		<< Compat::hex((UINT)riid.Data4[7]);
}

namespace Compat
{
	Log::Log()
	{
		SYSTEMTIME st = {};
		GetLocalTime(&st);

		char time[100];
		sprintf_s(time, "%02hu:%02hu:%02hu.%03hu ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

		s_logFile << GetCurrentThreadId() << " " << time;
	}

	Log::~Log()
	{
		s_logFile << std::endl;
	}

	//********** Begin Edit *************
	//std::ofstream Log::s_logFile("ddraw.log");
	std::ofstream Log::s_logFile(Log::Init());
	//********** End Edit ***************
	DWORD Log::s_outParamDepth = 0;
	bool Log::s_isLeaveLog = false;
}
