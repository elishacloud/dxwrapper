#pragma once

extern HMODULE hModule_dll;
extern CRITICAL_SECTION CriticalSection;
void CallCheckCurrentScreenRes();
void RunExitFunctions(bool = false);