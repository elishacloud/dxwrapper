#include "CompatGdi.h"
#include "CompatGdiScrollBar.h"
#include "Hook.h"

namespace Compat21
{
	namespace
	{
		enum ScrollBarInfoIndex
		{
			SBII_SCROLLBAR = 0,
			SBII_TOP_RIGHT_ARROW = 1,
			SBII_PAGEUP_PAGERIGHT_REGION = 2,
			SBII_THUMB = 3,
			SBII_PAGEDOWN_PAGELEFT_REGION = 4,
			SBII_BOTTOM_LEFT_ARROW = 5
		};
	}

	namespace CompatGdi
	{
		ScrollBar::ScrollBar(HWND hwnd, HDC compatDc) :
			m_hwnd(hwnd), m_compatDc(compatDc), m_windowRect(),
			m_isLeftMouseButtonDown(false), m_cursorPos(),
			m_horizontalSbi(), m_verticalSbi()
		{
			const LONG windowStyle = GetWindowLongPtr(hwnd, GWL_STYLE);

			m_horizontalSbi.isVisible = 0 != (windowStyle & WS_HSCROLL);
			m_verticalSbi.isVisible = 0 != (windowStyle & WS_VSCROLL);

			GetWindowRect(hwnd, &m_windowRect);

			if (m_horizontalSbi.isVisible || m_verticalSbi.isVisible)
			{
				m_isLeftMouseButtonDown =
					hwnd == GetCapture() &&
					GetAsyncKeyState(GetSystemMetrics(SM_SWAPBUTTON) ? VK_RBUTTON : VK_LBUTTON) < 0 &&
					GetCursorPos(&m_cursorPos);

				if (m_isLeftMouseButtonDown)
				{
					m_cursorPos.x -= m_windowRect.left;
					m_cursorPos.y -= m_windowRect.top;
				}

				if (m_horizontalSbi.isVisible)
				{
					m_horizontalSbi = getScrollBarInfo(OBJID_HSCROLL);
				}

				if (m_verticalSbi.isVisible)
				{
					m_verticalSbi = getScrollBarInfo(OBJID_VSCROLL);
				}
			}
		}

		void ScrollBar::drawAll() const
		{
			drawHorizArrows();
			drawVertArrows();
		}

		void ScrollBar::drawArrow(const ScrollBarChildInfo& sbci, UINT dfcState) const
		{
			UINT stateFlags = 0;
			if (sbci.state & STATE_SYSTEM_UNAVAILABLE)
			{
				stateFlags |= DFCS_INACTIVE;
			}
			else if (sbci.state & STATE_SYSTEM_PRESSED)
			{
				stateFlags |= DFCS_PUSHED;
			}

			RECT rect = sbci.rect;
			CALL_ORIG_FUNC(DrawFrameControl)(m_compatDc, &rect, DFC_SCROLL, dfcState | stateFlags);
		}

		void ScrollBar::drawHorizArrows() const
		{
			if (m_horizontalSbi.isVisible)
			{
				drawArrow(m_horizontalSbi.topLeftArrow, DFCS_SCROLLLEFT);
				drawArrow(m_horizontalSbi.bottomRightArrow, DFCS_SCROLLRIGHT);
			}
		}

		void ScrollBar::drawVertArrows() const
		{
			if (m_verticalSbi.isVisible)
			{
				drawArrow(m_verticalSbi.topLeftArrow, DFCS_SCROLLUP);
				drawArrow(m_verticalSbi.bottomRightArrow, DFCS_SCROLLDOWN);
			}
		}

		void ScrollBar::excludeFromClipRegion(const RECT& rect) const
		{
			ExcludeClipRect(m_compatDc, rect.left, rect.top, rect.right, rect.bottom);
		}

		void ScrollBar::excludeFromClipRegion(const ScrollBarInfo& sbi) const
		{
			if (sbi.isVisible)
			{
				excludeFromClipRegion(sbi.topLeftArrow.rect);
				excludeFromClipRegion(sbi.bottomRightArrow.rect);
			}
		}

		void ScrollBar::excludeFromClipRegion() const
		{
			excludeFromClipRegion(m_horizontalSbi);
			excludeFromClipRegion(m_verticalSbi);
		}

		ScrollBar::ScrollBarInfo ScrollBar::getScrollBarInfo(LONG objId) const
		{
			ScrollBarInfo scrollBarInfo = {};

			SCROLLBARINFO sbi = {};
			sbi.cbSize = sizeof(sbi);
			scrollBarInfo.isVisible = GetScrollBarInfo(m_hwnd, objId, &sbi) &&
				!(sbi.rgstate[SBII_SCROLLBAR] & (STATE_SYSTEM_INVISIBLE | STATE_SYSTEM_OFFSCREEN));

			if (!scrollBarInfo.isVisible)
			{
				return scrollBarInfo;
			}

			OffsetRect(&sbi.rcScrollBar, -m_windowRect.left, -m_windowRect.top);
			scrollBarInfo.topLeftArrow.rect = sbi.rcScrollBar;
			scrollBarInfo.bottomRightArrow.rect = sbi.rcScrollBar;

			if (OBJID_HSCROLL == objId)
			{
				const int w = GetSystemMetrics(SM_CXHSCROLL);

				scrollBarInfo.topLeftArrow.rect.right = scrollBarInfo.topLeftArrow.rect.left + w;
				scrollBarInfo.topLeftArrow.state = sbi.rgstate[SBII_BOTTOM_LEFT_ARROW];

				scrollBarInfo.bottomRightArrow.rect.left = scrollBarInfo.bottomRightArrow.rect.right - w;
				scrollBarInfo.bottomRightArrow.state = sbi.rgstate[SBII_TOP_RIGHT_ARROW];
			}
			else
			{
				const int h = GetSystemMetrics(SM_CYVSCROLL);

				scrollBarInfo.topLeftArrow.rect.bottom = scrollBarInfo.topLeftArrow.rect.top + h;
				scrollBarInfo.topLeftArrow.state = sbi.rgstate[SBII_TOP_RIGHT_ARROW];

				scrollBarInfo.bottomRightArrow.rect.top = scrollBarInfo.bottomRightArrow.rect.bottom - h;
				scrollBarInfo.bottomRightArrow.state = sbi.rgstate[SBII_BOTTOM_LEFT_ARROW];
			}

			if (m_isLeftMouseButtonDown)
			{
				setPressedState(scrollBarInfo.topLeftArrow);
				setPressedState(scrollBarInfo.bottomRightArrow);
			}

			return scrollBarInfo;
		}

		void ScrollBar::setPressedState(ScrollBarChildInfo& sbci) const
		{
			if (!(sbci.state & STATE_SYSTEM_UNAVAILABLE) && PtInRect(&sbci.rect, m_cursorPos))
			{
				sbci.state |= STATE_SYSTEM_PRESSED;
			}
		}
	}
}