/**
* Copyright (C) 2019 Elisha Riedlinger
*
* This software is  provided 'as-is', without any express  or implied  warranty. In no event will the
* authors be held liable for any damages arising from the use of this software.
* Permission  is granted  to anyone  to use  this software  for  any  purpose,  including  commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*   1. The origin of this software must not be misrepresented; you must not claim that you  wrote the
*      original  software. If you use this  software  in a product, an  acknowledgment in the product
*      documentation would be appreciated but is not required.
*   2. Altered source versions must  be plainly  marked as such, and  must not be  misrepresented  as
*      being the original software.
*   3. This notice may not be removed or altered from any source distribution.
*/

#include "Utils\Utils.h"
#include "Logging\Logging.h"

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
		SetThemeAppPropertiesPtr = reinterpret_cast<PFN_SetThemeAppProperties>(GetProcAddress(UxThemeModule, "SetThemeAppProperties"));
		if (!SetThemeAppPropertiesPtr)
		{
			Logging::Log() << "Failed to get 'SetThemeAppProperties' ProcAddress of uxtheme.dll!";
		}
	}
	else
	{
		Logging::Log() << "Failed to load uxtheme.dll!";
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
