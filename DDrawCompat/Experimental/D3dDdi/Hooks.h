#pragma once

#include <string>

namespace D3dDdi
{
	UINT getDdiVersion();
	void installHooks();
	void onUmdFileNameQueried(const std::wstring& umdFileName);
	void uninstallHooks();
}
