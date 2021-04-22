#pragma once

#include <Windows.h>

namespace Gdi
{
	class ScrollBar
	{
	public:
		ScrollBar(HWND hwnd, int bar);
		~ScrollBar();

		void onLButtonDown(LPARAM lParam);

		static void onCtlColorScrollBar(HWND hwnd, WPARAM wParam, LPARAM lParam, LRESULT result);

	private:
		void drawAll(HDC dc, HBRUSH brush);
		void drawArrow(HDC dc, LONG childId);
		void drawPageRegion(HDC dc, HBRUSH brush, LONG childId);
		void drawThumb(HDC dc, HBRUSH brush);
		RECT getChildRect(LONG childId);
		LONG hitTest(POINT p);
		bool isVisible();

		HWND m_hwnd;
		int m_bar;
		RECT m_windowRect;
		SCROLLBARINFO m_sbi;
		bool m_isVertical;
		int m_arrowSize;
		LONG RECT::* m_left;
		LONG RECT::* m_top;
		LONG RECT::* m_right;
		LONG RECT::* m_bottom;
		LONG m_trackedChildId;
		LONG m_trackedThumbOffset;
	};
}
