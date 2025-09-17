#pragma once

#include <string>

#include <Windows.h>

namespace Compat32
{
	std::wstring getModulePath(HMODULE module);
	std::wstring getSystemPath();
	std::wstring getParentPath(const std::wstring& path);
	std::string toUtf8(const std::wstring& wstr);
	bool isEqual(const std::wstring& p1, const std::wstring& p2);
	std::wstring replaceFilename(const std::wstring& path, const std::wstring& filename);
}
