#pragma once

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

namespace Win32
{
	namespace DisplayMode
	{
		HBITMAP WINAPI createCompatibleBitmap(HDC hdc, int cx, int cy);
		HBITMAP WINAPI createDIBitmap(HDC hdc, const BITMAPINFOHEADER* lpbmih, DWORD fdwInit,
			const void* lpbInit, const BITMAPINFO* lpbmi, UINT fuUsage);
		HBITMAP WINAPI createDiscardableBitmap(HDC hdc, int nWidth, int nHeight);

		void setDDrawBpp(DWORD bpp);

		void disableDwm8And16BitMitigation();
		void installHooks(HMODULE origDDrawModule);
	}
}
