#pragma once

#include <Windows.h>

namespace Gdi
{
	class CompatDc
	{
	public:
		CompatDc(HDC dc, bool isReadOnly = false);
		CompatDc(const CompatDc&) = delete;
		CompatDc(CompatDc&& other) = delete;
		CompatDc& operator=(const CompatDc&) = delete;
		CompatDc& operator=(CompatDc&&) = delete;
		~CompatDc();

		operator HDC() const
		{
			return m_compatDc;
		}

	private:
		HDC m_origDc;
		HDC m_compatDc;
		bool m_isReadOnly;
	};
}
