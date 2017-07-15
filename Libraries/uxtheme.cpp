#include  "Settings\Settings.h"

typedef void(WINAPI *PFN_SetThemeAppProperties)(DWORD dwFlags);
PFN_SetThemeAppProperties SetThemeAppPropertiesPtr = nullptr;
HMODULE UxThemeModule = nullptr;

void LoadUxtheme()
{
	static bool IsLoaded = false;
	if (IsLoaded)
	{
		return; // Only load the dll once
	}
	IsLoaded = true;
	UxThemeModule = LoadLibrary("uxtheme.dll");
	if (UxThemeModule)
	{
		LOG << "Loaded uxtheme.dll library";
		SetThemeAppPropertiesPtr = reinterpret_cast<PFN_SetThemeAppProperties>(GetProcAddress(UxThemeModule, "SetThemeAppProperties"));
		if (!SetThemeAppPropertiesPtr)
		{
			LOG << "Failed to get 'SetThemeAppProperties' ProcAddress of uxtheme.dll!";
		}
	}
	else
	{
		LOG << "Failed to load uxtheme.dll!";
	}
}

void UnLoadUxtheme()
{
	if (UxThemeModule)
	{
		FreeLibrary(UxThemeModule);
	}
}

void SetThemeAppProperties(DWORD dwFlags)
{
	// Load module
	LoadUxtheme();

	// Call function
	if (SetThemeAppPropertiesPtr)
	{
		SetThemeAppPropertiesPtr(dwFlags);
	}
}
