#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <Windows.h>
//********** Begin Edit *************
//#include <timeapi.h>
//********** End Edit ***************

#include <DDrawCompat/v0.3.0/Common/Hook.h>
#include <DDrawCompat/v0.3.0/Common/Time.h>
#include <DDrawCompat/v0.3.0/Config/Config.h>
#include <DDrawCompat/v0.3.0/Win32/WaitFunctions.h>

namespace
{
	void mitigateBusyWaiting()
	{
		thread_local ULONG64 ctLastThreadSwitch = Time::queryThreadCycleTime();
		ULONG64 ctNow = Time::queryThreadCycleTime();
		if (ctNow - ctLastThreadSwitch >= Config30::threadSwitchCycleTime)
		{
			Sleep(0);
			ctLastThreadSwitch = ctNow;
		}
	}

	template <auto func, typename Result, typename... Params>
	Result WINAPI mitigatedBusyWaitingFunc(Params... params)
	{
		mitigateBusyWaiting();
		return Compat30::g_origFuncPtr<func>(params...);
	}
}

#define MITIGATE_BUSY_WAITING(module, func) \
		Compat30::hookFunction<&func>(#module, #func, &mitigatedBusyWaitingFunc<&func>)

namespace Win32
{
	namespace WaitFunctions
	{
		//********** Begin Edit *************
		DWORD WINAPI timeGetTime()
		{
			return 0;
		}
		//********** End Edit ***************

		void installHooks()
		{
			MITIGATE_BUSY_WAITING(user32, GetMessageA);
			MITIGATE_BUSY_WAITING(user32, GetMessageW);
			MITIGATE_BUSY_WAITING(kernel32, GetTickCount);
			MITIGATE_BUSY_WAITING(user32, MsgWaitForMultipleObjects);
			MITIGATE_BUSY_WAITING(user32, MsgWaitForMultipleObjectsEx);
			MITIGATE_BUSY_WAITING(user32, PeekMessageA);
			MITIGATE_BUSY_WAITING(user32, PeekMessageW);
			MITIGATE_BUSY_WAITING(kernel32, SignalObjectAndWait);

			//********** Begin Edit *************
			//MITIGATE_BUSY_WAITING(winmm, timeGetTime);
			typedef DWORD(WINAPI* PFN_timeGetTime)();
			HMODULE dll = LoadLibrary("winmm.dll");
			Compat30::g_origFuncPtr<timeGetTime> = (PFN_timeGetTime)GetProcAddress(dll, "timeGetTime");
			Compat30::hookFunction(dll, "timeGetTime", reinterpret_cast<void*&>(Compat30::g_origFuncPtr<timeGetTime>), &mitigatedBusyWaitingFunc<&timeGetTime, DWORD>);
			//********** End Edit ***************

			MITIGATE_BUSY_WAITING(kernel32, WaitForSingleObjectEx);
			MITIGATE_BUSY_WAITING(kernel32, WaitForMultipleObjectsEx);
		}
	}
}
