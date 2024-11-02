#include <ddraw.h>
#include "DDrawCompatExternal.h"

// DDrawCompat versions
#include "v0.2.0b\DllMain.h"
#include "v0.2.1\DllMain.h"
#include "v0.3.1\Dll\DllMain.h"

#define DDrawCompatDefault Compat32
#define DDrawCompatForDd7to9 Compat32

#define INITIALIZE_WRAPPED_PROC(procName) \
	volatile FARPROC procName ## _in = nullptr; \
	volatile FARPROC procName ## _out = nullptr;

#define ASSIGN_WRAPPED_PROC(procName) \
	procName ## _in = procName ## _proc;

#define PREPARE_DDRAWCOMPAT(CompatVersion) \
	if (Config.DDraw ## CompatVersion) \
	{ \
		using namespace CompatVersion; \
		VISIT_ALL_DDRAW_PROCS(ASSIGN_WRAPPED_PROC); \
		return; \
	}

#define START_DDRAWCOMPAT(CompatVersion) \
	if (Config.DDraw ## CompatVersion) \
	{ \
		return (CompatVersion::DllMain_DDrawCompat(hinstDLL, fdwReason, nullptr) == TRUE); \
	}

namespace DDrawCompat
{
	VISIT_ALL_DDRAW_PROCS(INITIALIZE_WRAPPED_PROC);

	void Prepare()
	{
		// DDrawCompat v0.3.2
#ifdef DDRAWCOMPAT_32
		PREPARE_DDRAWCOMPAT(Compat32);
#endif

		// DDrawCompat v0.2.1
#ifdef DDRAWCOMPAT_21
		PREPARE_DDRAWCOMPAT(Compat21);
#endif

		// DDrawCompat v0.2.0b
#ifdef DDRAWCOMPAT_20
		PREPARE_DDRAWCOMPAT(Compat20);
#endif

		// Default DDrawCompat version
		using namespace DDrawCompatDefault;
		VISIT_ALL_DDRAW_PROCS(ASSIGN_WRAPPED_PROC);
		return;
	}

	bool RunStart(HINSTANCE hinstDLL, DWORD fdwReason)
	{
		// Dd7to9 DDrawCompat version
		if (Config.Dd7to9)
		{
			return (DDrawCompatForDd7to9::DllMain_DDrawCompat(hinstDLL, fdwReason, nullptr) == TRUE);
		}

		// DDrawCompat v0.3.2
#ifdef DDRAWCOMPAT_32
		START_DDRAWCOMPAT(Compat32);
#endif

		// DDrawCompat v0.2.1
#ifdef DDRAWCOMPAT_21
		START_DDRAWCOMPAT(Compat21);
#endif

		// DDrawCompat v0.2.0b
#ifdef DDRAWCOMPAT_20
		START_DDRAWCOMPAT(Compat20);
#endif

		// Default DDrawCompat version
		return (DDrawCompatDefault::DllMain_DDrawCompat(hinstDLL, fdwReason, nullptr) == TRUE);
	}

	bool IsDDrawEnabled = false;

	void Start(HINSTANCE hinstDLL, DWORD fdwReason)
	{
		bool ret = RunStart(hinstDLL, fdwReason);
		IsDDrawEnabled = Config.Dd7to9 ? false : ret;
	}

	bool IsEnabled()
	{
		return IsDDrawEnabled;
	}

	// Used for hooking with dd7to9
	void InstallDd7to9Hooks(HMODULE hModule)
	{
		// Dd7to9 DDrawCompat version
		return DDrawCompatForDd7to9::InstallDd7to9Hooks(hModule);
	}
}
