#pragma once

#include <filesystem>

#include <Windows.h>

namespace Compat30
{
	std::filesystem::path getModulePath(HMODULE module);
	std::filesystem::path getSystemPath();
	bool isEqual(const std::filesystem::path& p1, const std::filesystem::path& p2);
	std::filesystem::path replaceFilename(const std::filesystem::path& path, const std::filesystem::path& filename);
}
