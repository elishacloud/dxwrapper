#include <DDrawCompat/v0.3.1/Common/Path.h>

namespace Compat32
{
	std::filesystem::path getModulePath(HMODULE module)
	{
		wchar_t path[MAX_PATH];
		GetModuleFileNameW(module, path, MAX_PATH);
		return path;
	}

	std::filesystem::path getSystemPath()
	{
		wchar_t path[MAX_PATH] = {};
		GetSystemDirectoryW(path, MAX_PATH);
		return path;
	}

	bool isEqual(const std::filesystem::path& p1, const std::filesystem::path& p2)
	{
		return 0 == _wcsicmp(p1.c_str(), p2.c_str());
	}

	std::filesystem::path replaceFilename(const std::filesystem::path& path, const std::filesystem::path& filename)
	{
		std::filesystem::path result(path);
		result.replace_filename(filename);
		return result;
	}
}
