
#include "cfg.h"
#include "hotpatch.h"
#include "iatpatch.h"

void *HookAPI(HMODULE module, const char *dll, void *apiproc, const char *apiname, void *hookproc)
{
#ifdef _DEBUG
	Compat::Log() << "HookAPI: module=" << module << " dll=" << dll << " apiproc=" << apiproc << " apiname=" << apiname << " hookproc=" << hookproc;
#endif

	// Check if API name is blank
	if (!*apiname)
	{
#ifdef _DEBUG
		char *sMsg = "HookAPI: NULL api name\n";
		Compat::Log() << sMsg;
#endif
		return apiproc;
	}

	// Check module and API addresses
	if (!module || !apiproc || !hookproc)
	{
		return apiproc;
	}

	// Try HotPatch first
	void *orig;
	orig = HotPatch(apiproc, apiname, hookproc);
	if ((DWORD)orig > 1) return orig;

	// Try IATPatch next
	orig = IATPatch(module, 0, dll, apiproc, apiname, hookproc);
	if ((DWORD)orig > 1) return orig;

	// Return default address
	return apiproc;
}
