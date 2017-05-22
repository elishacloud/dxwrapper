
#include "cfg.h"
#include "hotpatch.h"
#include "iatpatch.h"

void *HookAPI(HMODULE module, char *dll, void *apiproc, const char *apiname, void *hookproc)
{
#ifdef _DEBUG
	Compat::Log() << "HookAPI: module=" << module << " dll=" << dll << " apiproc=" << apiproc << " apiname=" << apiname << " hookproc=" << hookproc;
#endif

	if (!*apiname) { // check
		char *sMsg = "HookAPI: NULL api name\n";
		Compat::Log() << sMsg;
		return 0;
	}

	// Hopatch
	// Currenlty disbaling hotpatch
	// Todo: add option for this later
	if (false) {
		void *orig;
		orig = HotPatch(apiproc, apiname, hookproc);
		if (orig) return orig;
	}

	return IATPatch(module, 0, dll, apiproc, apiname, hookproc);
}
