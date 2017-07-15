#include  "Settings\Settings.h"
#include  "Wrappers\wrapper.h"

typedef void(WINAPI *PFN_timeBeginPeriod)(UINT uPeriod);
typedef void(WINAPI *PFN_timeEndPeriod)(UINT uPeriod);
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
	winmmModule = LoadDll(dtype.winmm);
	if (winmmModule)
	{
		timeBeginPeriodPtr = reinterpret_cast<PFN_timeBeginPeriod>(GetProcAddress(winmmModule, "timeBeginPeriod"));
		timeEndPeriodPtr = reinterpret_cast<PFN_timeEndPeriod>(GetProcAddress(winmmModule, "timeEndPeriod"));
		if (!timeBeginPeriodPtr) LOG << "Failed to get 'timeBeginPeriod' ProcAddress of winmm.dll!";
		if (!timeEndPeriodPtr) LOG << "Failed to get 'timeEndPeriod' ProcAddress of winmm.dll!";
	}
	else
	{
		LOG << "Failed to load winmm.dll!";
	}
}

void _timeBeginPeriod(UINT uPeriod)
{
	// Load module
	Loadwinmm();

	// Call function
	if (timeBeginPeriodPtr)
	{
		timeBeginPeriodPtr(uPeriod);
	}
}

void _timeEndPeriod(UINT uPeriod)
{
	// Load module
	Loadwinmm();

	// Call function
	if (timeEndPeriodPtr)
	{
		timeEndPeriodPtr(uPeriod);
	}
}
