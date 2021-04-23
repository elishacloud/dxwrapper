#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <DDrawCompat/v0.3.0/Common/Hook.h>
#include <DDrawCompat/DDrawLog.h>
#include <DDrawCompat/v0.3.0/Gdi/Font.h>

namespace
{
	BOOL g_isFontSmoothingEnabled = FALSE;

	BOOL WINAPI systemParametersInfo(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni,
		decltype(&SystemParametersInfoA) origSystemParametersInfo, [[maybe_unused]] const char* origFuncName)
	{
		LOG_FUNC(origFuncName, Compat30::hex(uiAction), uiParam, pvParam, fWinIni);
		switch (uiAction)
		{
		case SPI_GETFONTSMOOTHING:
			if (pvParam)
			{
				*static_cast<BOOL*>(pvParam) = g_isFontSmoothingEnabled;
				return TRUE;
			}
			break;
		case SPI_SETFONTSMOOTHING:
			g_isFontSmoothingEnabled = 0 != uiParam;
			return TRUE;
		}
		return LOG_RESULT(origSystemParametersInfo(uiAction, uiParam, pvParam, fWinIni));
	}

	BOOL WINAPI systemParametersInfoA(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni)
	{
		return systemParametersInfo(uiAction, uiParam, pvParam, fWinIni,
			CALL_ORIG_FUNC(SystemParametersInfoA), "SystemParametersInfoA");
	}

	BOOL WINAPI systemParametersInfoW(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni)
	{
		return systemParametersInfo(uiAction, uiParam, pvParam, fWinIni,
			CALL_ORIG_FUNC(SystemParametersInfoW), "SystemParametersInfoW");
	}
}

namespace Gdi
{
	namespace Font
	{
		Mapper::Mapper(HDC dc) : m_dc(dc), m_origFont(nullptr)
		{
			if (!dc || g_isFontSmoothingEnabled)
			{
				return;
			}

			HFONT origFont = static_cast<HFONT>(GetCurrentObject(dc, OBJ_FONT));
			if (!origFont)
			{
				return;
			}

			LOGFONT logFont = {};
			GetObject(origFont, sizeof(logFont), &logFont);
			switch (logFont.lfQuality)
			{
			case NONANTIALIASED_QUALITY:
			case ANTIALIASED_QUALITY:
			case CLEARTYPE_QUALITY:
			case CLEARTYPE_NATURAL_QUALITY:
				return;
			}

			logFont.lfQuality = NONANTIALIASED_QUALITY;
			m_origFont = static_cast<HFONT>(SelectObject(dc, CreateFontIndirect(&logFont)));
		}

		Mapper::~Mapper()
		{
			if (m_origFont)
			{
				DeleteObject(SelectObject(m_dc, m_origFont));
			}
		}

		void installHooks()
		{
			SystemParametersInfo(SPI_GETFONTSMOOTHING, 0, &g_isFontSmoothingEnabled, 0);

			char fontSmoothing[2] = {};
			DWORD fontSmoothingSize = sizeof(fontSmoothing);
			RegGetValue(HKEY_CURRENT_USER, "Control Panel\\Desktop", "FontSmoothing", RRF_RT_REG_SZ, nullptr,
				&fontSmoothing, &fontSmoothingSize);

			BOOL isFontSmoothingEnabledInRegistry = 0 == strcmp(fontSmoothing, "2");
			if (isFontSmoothingEnabledInRegistry != g_isFontSmoothingEnabled)
			{
				SystemParametersInfo(SPI_SETFONTSMOOTHING, isFontSmoothingEnabledInRegistry, nullptr,
					SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
			}

			HOOK_FUNCTION(user32, SystemParametersInfoA, systemParametersInfoA);
			HOOK_FUNCTION(user32, SystemParametersInfoW, systemParametersInfoW);
		}

		bool isFontSmoothingEnabled()
		{
			return g_isFontSmoothingEnabled;
		}
	}
}
