#pragma once

#include <Windows.h>

namespace Gdi
{
	namespace Font
	{
		class Mapper
		{
		public:
			Mapper(HDC dc);
			~Mapper();

		private:
			HDC m_dc;
			HFONT m_origFont;
		};

		void installHooks();
	}
}
