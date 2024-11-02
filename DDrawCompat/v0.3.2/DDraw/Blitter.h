#pragma once

#include <Windows.h>

namespace DDraw
{
	namespace Blitter
	{
		void blt(void* dst, DWORD dstPitch, DWORD dstWidth, DWORD dstHeight,
			const void* src, DWORD srcPitch, LONG srcWidth, LONG srcHeight,
			DWORD bytesPerPixel, const DWORD* dstColorKey, const DWORD* srcColorKey);
		void colorFill(void* dst, DWORD dstPitch, DWORD dstWidth, DWORD dstHeight, DWORD bytesPerPixel, DWORD color);
	}
}
