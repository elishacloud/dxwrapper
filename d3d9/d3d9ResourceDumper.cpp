/**
* Copyright (C) 2026 Elisha Riedlinger
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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>
#include "d3d9.h"
#include "d3dx9.h"
#include <ddraw.h>
#include <d3dtypes.h>

namespace {
	typedef struct {
		DWORD dwSize;
		DWORD dwFlags;
		DWORD dwFourCC;
		DWORD dwRGBBitCount;
		DWORD dwRBitMask;
		DWORD dwGBitMask;
		DWORD dwBBitMask;
		DWORD dwABitMask;
	} DDS_PIXELFORMAT;

	typedef struct {
		DWORD           dwSize;
		DWORD           dwFlags;
		DWORD           dwHeight;
		DWORD           dwWidth;
		DWORD           dwPitchOrLinearSize;
		DWORD           dwDepth;
		DWORD           dwMipMapCount;
		DWORD           dwReserved1[11];
		DDS_PIXELFORMAT ddspf;
		DWORD           dwCaps;
		DWORD           dwCaps2;
		DWORD           dwCaps3;
		DWORD           dwCaps4;
		DWORD           dwReserved2;
	} DDS_HEADER;

	typedef struct {
		DWORD               dwMagic;
		DDS_HEADER          header;
#pragma warning (suppress : 4200)
		BYTE bdata[];
	} DDS_BUFFER;

	constexpr DWORD DDS_MAGIC = 0x20534444; // "DDS "
	constexpr DWORD DDS_HEADER_SIZE = sizeof(DWORD) + sizeof(DDS_HEADER);
	constexpr DWORD DDS_HEADER_FLAGS_TEXTURE = 0x00001007; // DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT 
	constexpr DWORD DDS_HEADER_FLAGS_PITCH = 0x00000008;
}

HRESULT DumpDXTDataToDDS(const void* data, size_t dataSize, int dxtVersion, DWORD Width, DWORD Height, const char* filename)
{
	int blockSize = 0;
	DWORD fourCC = 0;

	switch (dxtVersion)
	{
	case 1:
		blockSize = 8;
		fourCC = '1TXD';
		break;

	case 3:
		blockSize = 16;
		fourCC = '3TXD';
		break;

	case 5:
		blockSize = 16;
		fourCC = '5TXD';
		break;

	default:
		Logging::Log() << __FUNCTION__ << " Error: unsupported DXT version!";
		return D3DERR_INVALIDCALL;
	}

	std::ofstream outFile(filename, std::ios::binary | std::ios::out);
	if (outFile.is_open())
	{
		DDS_HEADER header = {};
		header.dwSize = sizeof(DDS_HEADER);
		header.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_LINEARSIZE;
		header.dwHeight = Height;
		header.dwWidth = Height;
		header.dwPitchOrLinearSize = max(1, (Width + 3) / 4) * blockSize;  // 8 for DXT1, 16 for others
		header.dwDepth = 0;
		header.dwMipMapCount = 0;
		header.ddspf.dwSize = sizeof(DDS_PIXELFORMAT);
		header.ddspf.dwFlags = DDPF_FOURCC;
		header.ddspf.dwFourCC = fourCC;
		header.dwCaps = DDSCAPS_TEXTURE;// | DDSCAPS_COMPLEX | DDSCAPS_MIPMAP;
		header.dwCaps2 = 0x00000000;
		header.dwCaps3 = 0x00000000;
		header.dwCaps4 = 0x00000000;
		header.dwReserved2 = 0;

		outFile.write("DDS ", 4);
		outFile.write((char*)&header, sizeof(DDS_HEADER));
		outFile.write((char*)data, dataSize);
		outFile.close();

		return D3D_OK;
	}

	return DDERR_GENERIC;
}

HRESULT DumpSurfaceToFile(IDirect3DSurface9* pSurface, D3DXIMAGE_FILEFORMAT format, const char* filename)
{
	if (!pSurface)
	{
		return D3DERR_INVALIDCALL;
	}

	ComPtr<ID3DXBuffer> pDestBuf;
	HRESULT hr = D3DXSaveSurfaceToFileInMemory(pDestBuf.GetAddressOf(), format, pSurface, nullptr, nullptr);

	if (SUCCEEDED(hr))
	{
		// Save the buffer to a file
		std::ofstream outFile(filename, std::ios::binary | std::ios::out);
		if (outFile.is_open())
		{
			outFile.write((const char*)pDestBuf->GetBufferPointer(), pDestBuf->GetBufferSize());
			outFile.close();
		}
	}

	return hr;
}

bool DumpHDCToBMP(const std::string& filename, HDC hdc)
{
	if (!hdc) return false;

	// Get the size of the source HDC (assumes it's a window or screen DC)
	RECT rect;
	if (!GetClipBox(hdc, &rect))
		return false;

	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	HDC memDC = CreateCompatibleDC(hdc);
	if (!memDC) return false;

	HBITMAP hBitmap = CreateCompatibleBitmap(hdc, width, height);
	if (!hBitmap)
	{
		DeleteDC(memDC);
		return false;
	}

	HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, hBitmap);

	// Copy pixels from source HDC
	BitBlt(memDC, 0, 0, width, height, hdc, rect.left, rect.top, SRCCOPY);

	BITMAP bmp;
	GetObject(hBitmap, sizeof(BITMAP), &bmp);

	BITMAPINFOHEADER bi = {};
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = bmp.bmWidth;
	bi.biHeight = bmp.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = 24;
	bi.biCompression = BI_RGB;

	int rowSize = ((bmp.bmWidth * 24 + 31) / 32) * 4;
	int imageSize = rowSize * bmp.bmHeight;

	BITMAPFILEHEADER bmf = {};
	bmf.bfType = 0x4D42; // "BM"
	bmf.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	bmf.bfSize = bmf.bfOffBits + imageSize;

	BYTE* bits = new BYTE[imageSize];

	BITMAPINFO biInfo = {};
	biInfo.bmiHeader = bi;

	HDC tempDC = CreateCompatibleDC(hdc);
	GetDIBits(tempDC, hBitmap, 0, bmp.bmHeight, bits, &biInfo, DIB_RGB_COLORS);
	DeleteDC(tempDC);

	FILE* file = nullptr;
	fopen_s(&file, ("dxwrapper - " + filename).c_str(), "wb");
	if (!file)
	{
		delete[] bits;
		return false;
	}

	fwrite(&bmf, sizeof(bmf), 1, file);
	fwrite(&bi, sizeof(bi), 1, file);
	fwrite(bits, imageSize, 1, file);
	fclose(file);

	delete[] bits;

	SelectObject(memDC, oldBitmap);
	DeleteObject(hBitmap);
	DeleteDC(memDC);

	return true;
}
