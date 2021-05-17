#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <DDrawCompat/v0.3.1/Common/Hook.h>
#include <DDrawCompat/v0.3.1/Gdi/Region.h>
#include <DDrawCompat/v0.3.1/Gdi/TitleBar.h>
#include <DDrawCompat/v0.3.1/Win32/DisplayMode.h>

namespace
{
	enum TitleBarInfoIndex
	{
		TBII_TITLEBAR = 0,
		TBII_MINIMIZE_BUTTON = 2,
		TBII_MAXIMIZE_BUTTON = 3,
		TBII_HELP_BUTTON = 4,
		TBII_CLOSE_BUTTON = 5
	};
}

namespace Gdi
{
	TitleBar::TitleBar(HWND hwnd)
		: m_hwnd(hwnd)
		, m_buttonWidth(0)
		, m_buttonHeight(0)
		, m_tbi{}
		, m_windowRect{}
		, m_hasIcon(false)
		, m_hasTitleBar(false)
		, m_isActive(false)
	{
		m_hasTitleBar = 0 != (CALL_ORIG_FUNC(GetWindowLongA)(hwnd, GWL_STYLE) & WS_CAPTION);
		if (!m_hasTitleBar)
		{
			return;
		}

		m_tbi.cbSize = sizeof(m_tbi);
		CALL_ORIG_FUNC(DefWindowProcA)(hwnd, WM_GETTITLEBARINFOEX, 0, reinterpret_cast<LPARAM>(&m_tbi));
		m_hasTitleBar = !IsRectEmpty(&m_tbi.rcTitleBar);
		if (!m_hasTitleBar)
		{
			return;
		}

		POINT origin = {};
		ClientToScreen(hwnd, &origin);
		m_hasIcon = m_tbi.rcTitleBar.left > origin.x;
		m_tbi.rcTitleBar.left = origin.x;
		m_tbi.rcTitleBar.bottom -= 1;

		GetWindowRect(hwnd, &m_windowRect);
		OffsetRect(&m_tbi.rcTitleBar, -m_windowRect.left, -m_windowRect.top);

		m_isActive = GetActiveWindow() == hwnd;
		m_buttonWidth = GetSystemMetrics(SM_CXSIZE) - 2;
		m_buttonHeight = GetSystemMetrics(SM_CYSIZE) - 4;

		for (std::size_t i = TBII_MINIMIZE_BUTTON; i <= TBII_CLOSE_BUTTON; ++i)
		{
			if (isVisible(i))
			{
				OffsetRect(&m_tbi.rgrect[i], -m_windowRect.left, -m_windowRect.top);
				InflateRect(&m_tbi.rgrect[i], -3, -3);
				if (TBII_MINIMIZE_BUTTON == i)
				{
					m_tbi.rgrect[i].right += 2;
				}
				else
				{
					m_tbi.rgrect[i].left -= 2;
				}
			}
		}
	}

	void TitleBar::drawAll(HDC dc) const
	{
		drawCaption(dc);
		drawButtons(dc);
	}

	void TitleBar::drawButtons(HDC dc) const
	{
		if (!m_hasTitleBar)
		{
			return;
		}

		drawButton(dc, TBII_MINIMIZE_BUTTON, DFCS_CAPTIONMIN);
		drawButton(dc, TBII_MAXIMIZE_BUTTON, IsZoomed(m_hwnd) ? DFCS_CAPTIONRESTORE : DFCS_CAPTIONMAX);
		drawButton(dc, TBII_HELP_BUTTON, DFCS_CAPTIONHELP);
		drawButton(dc, TBII_CLOSE_BUTTON, DFCS_CAPTIONCLOSE);
	}

	void TitleBar::drawCaption(HDC dc) const
	{
		if (!m_hasTitleBar)
		{
			return;
		}

		UINT flags = DC_TEXT;
		if (m_isActive)
		{
			flags |= DC_ACTIVE;
		}
		if (Win32::DisplayMode::getBpp() > 8)
		{
			flags |= DC_GRADIENT;
		}

		SelectClipRgn(dc, Region(m_tbi.rcTitleBar));
		DrawCaption(m_hwnd, dc, &m_tbi.rcTitleBar, flags);
		SelectClipRgn(dc, nullptr);

		if (m_hasIcon)
		{
			RECT r = m_tbi.rcTitleBar;
			r.right = r.left + r.bottom - r.top;
			FillRect(dc, &r, GetSysColorBrush(m_isActive ? COLOR_ACTIVECAPTION : COLOR_INACTIVECAPTION));

			HICON icon = reinterpret_cast<HICON>(SendMessage(m_hwnd, WM_GETICON, ICON_SMALL, 96));
			if (!icon)
			{
				icon = reinterpret_cast<HICON>(GetClassLong(m_hwnd, GCL_HICONSM));
			}
			int width = GetSystemMetrics(SM_CXSMICON);
			int height = GetSystemMetrics(SM_CYSMICON);
			int x = r.left + (r.right - r.left - width) / 2 + 2;
			int y = r.top + (r.bottom - r.top - height) / 2;
			DrawIconEx(dc, x, y, icon, width, height, 0, nullptr, DI_NORMAL);
		}
	}

	void TitleBar::drawButton(HDC dc, std::size_t tbiIndex, UINT dfcState) const
	{
		if (!isVisible(tbiIndex))
		{
			return;
		}

		DWORD stateFlags = 0;
		if (m_tbi.rgstate[tbiIndex] & STATE_SYSTEM_UNAVAILABLE)
		{
			stateFlags |= DFCS_INACTIVE;
		}
		else if (m_tbi.rgstate[tbiIndex] & STATE_SYSTEM_PRESSED)
		{
			stateFlags |= DFCS_PUSHED;
		}

		RECT rect = m_tbi.rgrect[tbiIndex];
		DrawFrameControl(dc, &rect, DFC_CAPTION, dfcState | stateFlags);
	}

	bool TitleBar::isVisible(std::size_t tbiIndex) const
	{
		return !(m_tbi.rgstate[tbiIndex] & (STATE_SYSTEM_INVISIBLE | STATE_SYSTEM_OFFSCREEN));
	}
}
