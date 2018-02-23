#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

#include "Win32/FontSmoothing.h"

namespace Win32
{
	namespace FontSmoothing
	{
		SystemSettings g_origSystemSettings = {};

		bool SystemSettings::operator==(const SystemSettings& rhs) const
		{
			return isEnabled == rhs.isEnabled &&
				type == rhs.type &&
				contrast == rhs.contrast &&
				orientation == rhs.orientation;
		}

		bool SystemSettings::operator!=(const SystemSettings& rhs) const
		{
			return !(*this == rhs);
		}

		SystemSettings getSystemSettings()
		{
			SystemSettings settings = {};
			SystemParametersInfo(SPI_GETFONTSMOOTHING, 0, &settings.isEnabled, 0);
			SystemParametersInfo(SPI_GETFONTSMOOTHINGTYPE, 0, &settings.type, 0);
			SystemParametersInfo(SPI_GETFONTSMOOTHINGCONTRAST, 0, &settings.contrast, 0);
			SystemParametersInfo(SPI_GETFONTSMOOTHINGORIENTATION, 0, &settings.orientation, 0);
			return settings;
		}

		void setSystemSettings(const SystemSettings& settings)
		{
			if (settings != getSystemSettings())
			{
				setSystemSettingsForced(settings);
			}
		}

		void setSystemSettingsForced(const SystemSettings& settings)
		{
			SystemParametersInfo(SPI_SETFONTSMOOTHING, settings.isEnabled, nullptr, 0);
			SystemParametersInfo(SPI_SETFONTSMOOTHINGTYPE, 0,
				reinterpret_cast<void*>(settings.type), 0);
			SystemParametersInfo(SPI_SETFONTSMOOTHINGCONTRAST, 0,
				reinterpret_cast<void*>(settings.contrast), 0);
			SystemParametersInfo(SPI_SETFONTSMOOTHINGORIENTATION, 0,
				reinterpret_cast<void*>(settings.orientation), 0);

			const char* regKey = "FontSmoothing";
			SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, SPI_SETFONTSMOOTHING,
				reinterpret_cast<LPARAM>(regKey), SMTO_BLOCK, 100, nullptr);
			RedrawWindow(nullptr, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
		}
	}
}
