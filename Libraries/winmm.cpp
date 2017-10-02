/**
* Copyright (C) 2017 Elisha Riedlinger
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

#include "winmm.h"
#include "Settings\Settings.h"
#include "Utils\Utils.h"
#include "Logging\Logging.h"

typedef MMRESULT(WINAPI *PFN_timeBeginPeriod)(UINT uPeriod);
typedef MMRESULT(WINAPI *PFN_timeEndPeriod)(UINT uPeriod);
PFN_timeBeginPeriod timeBeginPeriodPtr = nullptr;
PFN_timeEndPeriod timeEndPeriodPtr = nullptr;
HMODULE winmmModule = nullptr;

void Loadwinmm()
{
	static bool IsLoaded = false;
	if (IsLoaded)
	{
		return; // Only load the dll once
	}
	IsLoaded = true;
	winmmModule = Utils::LoadLibrary("winmm.dll");
	if (winmmModule)
	{
		timeBeginPeriodPtr = reinterpret_cast<PFN_timeBeginPeriod>(GetProcAddress(winmmModule, "timeBeginPeriod"));
		timeEndPeriodPtr = reinterpret_cast<PFN_timeEndPeriod>(GetProcAddress(winmmModule, "timeEndPeriod"));
		if (!timeBeginPeriodPtr) Logging::Log() << "Failed to get 'timeBeginPeriod' ProcAddress of winmm.dll!";
		if (!timeEndPeriodPtr) Logging::Log() << "Failed to get 'timeEndPeriod' ProcAddress of winmm.dll!";
	}
	else
	{
		Logging::Log() << "Failed to load winmm.dll!";
	}
}

MMRESULT timeBeginPeriod(UINT uPeriod)
{
	// Load module
	Loadwinmm();

	// Call function
	if (timeBeginPeriodPtr)
	{
		return timeBeginPeriodPtr(uPeriod);
	}
	return S_FALSE;
}

MMRESULT timeEndPeriod(UINT uPeriod)
{
	// Load module
	Loadwinmm();

	// Call function
	if (timeEndPeriodPtr)
	{
		return timeEndPeriodPtr(uPeriod);
	}
	return S_FALSE;
}
