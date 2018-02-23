#pragma once

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

namespace Win32
{
	namespace FontSmoothing
	{
		struct SystemSettings
		{
			BOOL isEnabled;
			UINT type;
			UINT contrast;
			UINT orientation;

			bool operator==(const SystemSettings& rhs) const;
			bool operator!=(const SystemSettings& rhs) const;
		};

		extern SystemSettings g_origSystemSettings;

		SystemSettings getSystemSettings();
		void setSystemSettings(const SystemSettings& settings);
		void setSystemSettingsForced(const SystemSettings& settings);
	}
}
