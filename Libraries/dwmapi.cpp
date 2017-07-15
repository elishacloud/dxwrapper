#include  "Settings\Settings.h"

typedef HRESULT(WINAPI *PFN_DwmSetWindowAttribute)(HWND hwnd, DWORD dwAttribute, LPCVOID pvAttribute, DWORD cbAttribute);
PFN_DwmSetWindowAttribute DwmSetWindowAttributePtr = nullptr;
HMODULE dwmapiModule = nullptr;

void Loaddwmapi()
{
	static bool IsLoaded = false;
	if (IsLoaded)
	{
		return; // Only load the dll once
	}
	IsLoaded = true;
	dwmapiModule = LoadLibrary("dwmapi.dll");
	if (dwmapiModule)
	{
		LOG << "Loaded dwmapi.dll library";
		DwmSetWindowAttributePtr = reinterpret_cast<PFN_DwmSetWindowAttribute>(GetProcAddress(dwmapiModule, "DwmSetWindowAttribute"));
		if (!DwmSetWindowAttributePtr)
		{
			LOG << "Failed to get 'DwmSetWindowAttribute' ProcAddress of dwmapi.dll!";
		}
	}
	else
	{
		LOG << "Failed to load dwmapi.dll!";
	}
}

void UnLoaddwmapi()
{
	if (dwmapiModule)
	{
		FreeLibrary(dwmapiModule);
	}
}

HRESULT DwmSetWindowAttribute(HWND hwnd, DWORD dwAttribute, LPCVOID pvAttribute, DWORD cbAttribute)
{
	// Load module
	Loaddwmapi();

	// Call function
	if (DwmSetWindowAttributePtr)
	{
		return DwmSetWindowAttributePtr(hwnd, dwAttribute, pvAttribute, cbAttribute);
	}
	return E_FAIL;
}
