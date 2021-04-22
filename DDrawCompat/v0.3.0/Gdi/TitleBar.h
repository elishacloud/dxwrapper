#pragma once

#include <cstddef>

#include <Windows.h>

namespace Gdi
{
	class TitleBar
	{
	public:
		TitleBar(HWND hwnd);

		void drawAll(HDC dc) const;
		void drawButtons(HDC dc) const;
		void drawCaption(HDC dc) const;

	private:
		void drawButton(HDC dc, std::size_t tbiIndex, UINT dfcState) const;
		bool isVisible(std::size_t tbiIndex) const;

		HWND m_hwnd;
		int m_buttonWidth;
		int m_buttonHeight;
		TITLEBARINFOEX m_tbi;
		RECT m_windowRect;
		bool m_hasIcon;
		bool m_hasTitleBar;
		bool m_isActive;
	};
}
