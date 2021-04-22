#pragma once

#include <Windows.h>

namespace Gdi
{
	namespace Dc
	{
		void dllThreadDetach();
		HDC getDc(HDC origDc);
		HDC getOrigDc(HDC dc);
		void releaseDc(HDC origDc);
	}
}
