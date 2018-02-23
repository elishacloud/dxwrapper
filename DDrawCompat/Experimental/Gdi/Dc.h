#pragma once

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

namespace Gdi
{
	namespace Dc
	{
		HDC getDc(HDC origDc);
		HDC getOrigDc(HDC dc);
		void releaseDc(HDC origDc);
	}
}
