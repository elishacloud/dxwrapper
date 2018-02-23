#pragma once

#define WIN32_LEAN_AND_MEAN

#include <cstddef>

#include <Windows.h>

namespace Gdi
{
	class TitleBar
	{
	public:
		TitleBar(HWND hwnd, HDC compatDc);

		void drawAll() const;
		void drawButtons() const;
		void drawCaption() const;
		void excludeFromClipRegion() const;

	private:
		void adjustButtonSize(RECT& rect) const;
		void drawButton(std::size_t tbiIndex, UINT dfcState) const;
		bool isVisible(std::size_t tbiIndex) const;

		HWND m_hwnd;
		HDC m_compatDc; 
		int m_buttonWidth;
		int m_buttonHeight;
		TITLEBARINFOEX m_tbi;
		RECT m_windowRect;
		bool m_hasIcon;
		bool m_hasTitleBar;
	};
}
