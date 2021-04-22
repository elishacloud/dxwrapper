#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <Common/Hook.h>
#include <Gdi/ScrollBar.h>

namespace
{
	enum ScrollBarInfoIndex
	{
		SBII_SELF = 0,
		SBII_TOP_ARROW = 1,
		SBII_PAGEUP_REGION = 2,
		SBII_THUMB = 3,
		SBII_PAGEDOWN_REGION = 4,
		SBII_BOTTOM_ARROW = 5
	};

	bool isVertical(HWND hwnd, int bar)
	{
		if (SB_CTL == bar)
		{
			return CALL_ORIG_FUNC(GetWindowLongA)(hwnd, GWL_STYLE) & SBS_VERT;
		}
		return SB_VERT == bar;
	}

	thread_local Gdi::ScrollBar* g_trackedScrollBar = nullptr;
}

namespace Gdi
{
	ScrollBar::ScrollBar(HWND hwnd, int bar)
		: m_hwnd(hwnd)
		, m_bar(bar)
		, m_windowRect{}
		, m_sbi{}
		, m_isVertical(isVertical(hwnd, bar))
		, m_arrowSize(CALL_ORIG_FUNC(GetSystemMetrics)(m_isVertical ? SM_CYVSCROLL : SM_CXHSCROLL))
		, m_left(m_isVertical ? &RECT::left : &RECT::top)
		, m_top(m_isVertical ? &RECT::top : &RECT::left)
		, m_right(m_isVertical ? &RECT::right : &RECT::bottom)
		, m_bottom(m_isVertical ? &RECT::bottom : &RECT::right)
		, m_trackedChildId(-1)
		, m_trackedThumbOffset(0)
	{
		LONG objectId = OBJID_CLIENT;
		if (SB_HORZ == bar)
		{
			objectId = OBJID_HSCROLL;
		}
		else if (SB_VERT == bar)
		{
			objectId = OBJID_VSCROLL;
		}
		m_sbi.cbSize = sizeof(m_sbi);
		GetScrollBarInfo(hwnd, objectId, &m_sbi);

		GetWindowRect(hwnd, &m_windowRect);
		OffsetRect(&m_sbi.rcScrollBar, -m_windowRect.left, -m_windowRect.top);

		if (g_trackedScrollBar &&
			hwnd == g_trackedScrollBar->m_hwnd &&
			bar == g_trackedScrollBar->m_bar &&
			GetKeyState(GetSystemMetrics(SM_SWAPBUTTON) ? VK_RBUTTON : VK_LBUTTON) < 0)
		{
			DWORD pos = GetMessagePos();
			POINTS pt = *reinterpret_cast<POINTS*>(&pos);
			POINT p = { pt.x - m_windowRect.left, pt.y - m_windowRect.top };

			if (SBII_THUMB == g_trackedScrollBar->m_trackedChildId)
			{
				RECT trackRect = m_sbi.rcScrollBar;
				if (m_isVertical)
				{
					InflateRect(&trackRect, 8 * (trackRect.right - trackRect.left), 2 * m_arrowSize);
				}
				else
				{
					InflateRect(&trackRect, 2 * m_arrowSize, 8 * (trackRect.bottom - trackRect.top));
				}
				if (PtInRect(&trackRect, p))
				{
					const LONG thumbSize = m_sbi.xyThumbBottom - m_sbi.xyThumbTop;
					const LONG minThumbPos = m_arrowSize;
					const LONG maxThumbPos = m_sbi.rcScrollBar.*m_bottom - m_sbi.rcScrollBar.*m_top - thumbSize - m_arrowSize;

					LONG thumbPos = (m_isVertical ? p.y : p.x) - m_sbi.rcScrollBar.*m_top -
						g_trackedScrollBar->m_trackedThumbOffset;
					if (thumbPos < minThumbPos)
					{
						thumbPos = minThumbPos;
					}
					if (thumbPos > maxThumbPos)
					{
						thumbPos = maxThumbPos;
					}

					m_sbi.xyThumbTop = thumbPos;
					m_sbi.xyThumbBottom = thumbPos + thumbSize;
				}
			}
			else
			{
				RECT rect = getChildRect(g_trackedScrollBar->m_trackedChildId);
				if (PtInRect(&rect, p))
				{
					m_sbi.rgstate[g_trackedScrollBar->m_trackedChildId] |= STATE_SYSTEM_PRESSED;
				}
			}
		}
	}

	ScrollBar::~ScrollBar()
	{
		if (this == g_trackedScrollBar)
		{
			g_trackedScrollBar = nullptr;
		}
	}

	void ScrollBar::drawAll(HDC dc, HBRUSH brush)
	{
		if (isVisible())
		{
			drawArrow(dc, SBII_TOP_ARROW);
			drawPageRegion(dc, brush, SBII_PAGEUP_REGION);
			drawThumb(dc, brush);
			drawPageRegion(dc, brush, SBII_PAGEDOWN_REGION);
			drawArrow(dc, SBII_BOTTOM_ARROW);
		}
	}

	void ScrollBar::drawArrow(HDC dc, LONG childId)
	{
		UINT state = 0;
		if (SBII_TOP_ARROW == childId)
		{
			state = m_isVertical ? DFCS_SCROLLUP : DFCS_SCROLLLEFT;
		}
		else
		{
			state = m_isVertical ? DFCS_SCROLLDOWN : DFCS_SCROLLRIGHT;
		}

		if (m_sbi.rgstate[childId] & STATE_SYSTEM_UNAVAILABLE)
		{
			state |= DFCS_INACTIVE;
		}
		else if (m_sbi.rgstate[childId] & STATE_SYSTEM_PRESSED)
		{
			state |= DFCS_PUSHED;
		}

		RECT rect = getChildRect(childId);
		DrawFrameControl(dc, &rect, DFC_SCROLL, state);
	}

	void ScrollBar::drawPageRegion(HDC dc, HBRUSH brush, LONG childId)
	{
		RECT rect = getChildRect(childId);
		if (IsRectEmpty(&rect))
		{
			return;
		}

		FillRect(dc, &rect, brush);

		if (SB_CTL == m_bar)
		{
			const UINT edges = m_isVertical ? (BF_LEFT | BF_RIGHT) : (BF_TOP | BF_BOTTOM);
			DrawEdge(dc, &rect, BDR_SUNKEN, edges | BF_FLAT);
		}

		if (m_sbi.rgstate[childId] & STATE_SYSTEM_PRESSED)
		{
			InvertRect(dc, &rect);
		}
	}

	void ScrollBar::drawThumb(HDC dc, HBRUSH brush)
	{
		if (m_sbi.rgstate[SBII_SELF] & STATE_SYSTEM_UNAVAILABLE)
		{
			drawPageRegion(dc, brush, SBII_THUMB);
		}
		else
		{
			RECT rect = m_sbi.rcScrollBar;
			rect.*m_top += m_sbi.xyThumbTop;
			rect.*m_bottom = m_sbi.rcScrollBar.*m_top + m_sbi.xyThumbBottom;
			DrawFrameControl(dc, &rect, DFC_BUTTON, DFCS_BUTTONPUSH);
		}
	}

	RECT ScrollBar::getChildRect(LONG childId)
	{
		RECT r = m_sbi.rcScrollBar;
		switch (childId)
		{
		case SBII_TOP_ARROW:
			r.*m_bottom = r.*m_top + m_arrowSize;
			break;

		case SBII_PAGEUP_REGION:
			r.*m_bottom = r.*m_top + m_sbi.xyThumbTop;
			r.*m_top += m_arrowSize;
			break;

		case SBII_THUMB:
			r.*m_bottom = r.*m_top + m_sbi.xyThumbBottom;
			r.*m_top += m_sbi.xyThumbTop;
			break;

		case SBII_PAGEDOWN_REGION:
			r.*m_top += m_sbi.xyThumbBottom;
			r.*m_bottom -= m_arrowSize;
			break;

		case SBII_BOTTOM_ARROW:
			r.*m_top = r.*m_bottom - m_arrowSize;
			break;
		}
		return r;
	}

	LONG ScrollBar::hitTest(POINT p)
	{
		if (m_sbi.rgstate[SBII_SELF] & STATE_SYSTEM_UNAVAILABLE)
		{
			return -1;
		}

		for (UINT i = SBII_TOP_ARROW; i <= SBII_BOTTOM_ARROW; ++i)
		{
			RECT r = getChildRect(i);
			if (PtInRect(&r, p))
			{
				return (m_sbi.rgstate[i] & STATE_SYSTEM_UNAVAILABLE) ? -1 : i;
			}
		}
		return -1;
	}

	bool ScrollBar::isVisible()
	{
		return !(m_sbi.rgstate[SBII_SELF] & (STATE_SYSTEM_INVISIBLE | STATE_SYSTEM_OFFSCREEN));
	}

	void ScrollBar::onCtlColorScrollBar(HWND hwnd, WPARAM wParam, LPARAM lParam, LRESULT result)
	{
		HWND hwndSb = reinterpret_cast<HWND>(lParam);
		HBRUSH brush = reinterpret_cast<HBRUSH>(result);

		HDC dc = reinterpret_cast<HDC>(wParam);
		if (hwnd == hwndSb)
		{
			ScrollBar(hwnd, SB_HORZ).drawAll(dc, brush);
			ScrollBar(hwnd, SB_VERT).drawAll(dc, brush);
		}
		else
		{
			ScrollBar(hwndSb, SB_CTL).drawAll(dc, brush);
		}
	}

	void ScrollBar::onLButtonDown(LPARAM lParam)
	{
		POINTS pt = *reinterpret_cast<POINTS*>(&lParam);
		POINT p = { pt.x, pt.y };
		if (SB_CTL != m_bar)
		{
			p.x -= m_windowRect.left;
			p.y -= m_windowRect.top;
		}

		m_trackedChildId = hitTest(p);
		if (-1 != m_trackedChildId)
		{
			g_trackedScrollBar = this;
			if (SBII_THUMB == m_trackedChildId)
			{
				RECT rect = getChildRect(SBII_THUMB);
				m_trackedThumbOffset = m_isVertical ? (p.y - rect.top) : (p.x - rect.left);
			}
		}
	}
}
