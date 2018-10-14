#include <ddraw.h>
#include "DDrawCompatExternal.h"

// DDrawCompat versions
#include "Experimental\Dll\DllMain.h"
#include "v0.2.0b\DllMain.h"
#include "v0.2.1\DllMain.h"

#define INITIALIZE_WRAPPED_PROC(procName) \
	FARPROC procName ## _in = nullptr; \
	FARPROC procName ## _out = nullptr;

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
		// DDrawCompat v0.2.1
#ifdef DDRAWCOMPAT_21
		PREPARE_DDRAWCOMPAT(Compat21);
#endif

		// DDrawCompat v0.2.0b
#ifdef DDRAWCOMPAT_20
		PREPARE_DDRAWCOMPAT(Compat20);
#endif

		// DDrawCompat Eperimental
#ifdef DDRAWCOMPAT_EXP
		PREPARE_DDRAWCOMPAT(CompatExperimental);
#endif

		// Default DDrawCompat version
		using namespace DDrawCompatDefault;
		VISIT_ALL_DDRAW_PROCS(ASSIGN_WRAPPED_PROC);
		return;
	}

	bool Start(HINSTANCE hinstDLL, DWORD fdwReason)
	{
		// DDrawCompat v0.2.1
#ifdef DDRAWCOMPAT_21
		START_DDRAWCOMPAT(Compat21);
#endif

		// DDrawCompat v0.2.0b
#ifdef DDRAWCOMPAT_20
		START_DDRAWCOMPAT(Compat20);
#endif

		// DDrawCompat Eperimental
#ifdef DDRAWCOMPAT_EXP
		START_DDRAWCOMPAT(CompatExperimental);
#endif

		// Default DDrawCompat version
		return (DDrawCompatDefault::DllMain_DDrawCompat(hinstDLL, fdwReason, nullptr) == TRUE);
	}
}
