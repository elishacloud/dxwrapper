/**
* Created from source code found in DxWnd v2.03.99
* https://sourceforge.net/projects/dxwnd/
*
* Updated 2017 by Elisha Riedlinger
*/

#include "Settings\Settings.h"
#include "hotpatch.h"
#include "iatpatch.h"

void *HookAPI(HMODULE module, const char *dll, void *apiproc, const char *apiname, void *hookproc)
{
#ifdef _DEBUG
	Compat::Log() << "HookAPI: module=" << module << " dll=" << dll << " apiproc=" << apiproc << " apiname=" << apiname << " hookproc=" << hookproc;
#endif

	// Check if API name is blank
	if (!apiname)
	{
		Compat::Log() << "HookAPI: NULL api name";
		return apiproc;
	}

	// Check API address
	if (!apiproc)
	{
		Compat::Log() << "Failed to find '" << apiname << "' api";
		return apiproc;
	}

	// Check hook address
	if (!hookproc)
	{
		Compat::Log() << "Invalid hook address for '" << apiname << "'";
		return apiproc;
	}

	// Try HotPatch first
	void *orig;
	orig = HotPatch(apiproc, apiname, hookproc);
	if ((DWORD)orig > 1)
	{
		return orig;
	}

	// Check if dll name is blank
	if (!apiname)
	{
		Compat::Log() << "HookAPI: NULL dll name";
		return apiproc;
	}

	// Check module addresses
	if (!module)
	{
		Compat::Log() << "HookAPI: NULL api module address for '" << dll << "'";
		return apiproc;
	}

	// Try IATPatch next
	orig = IATPatch(module, 0, dll, apiproc, apiname, hookproc);
	if ((DWORD)orig > 1)
	{
		return orig;
	}

	// Return default address
	return apiproc;
}