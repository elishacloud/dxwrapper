#pragma once

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

namespace Compat20
{
	namespace CompatGdiDc
	{
		HDC getDc(HDC origDc);
		void releaseDc(HDC origDc);
	}
}
