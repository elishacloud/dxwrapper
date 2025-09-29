#include <DDrawCompat/v0.3.2/Common/Path.h>
#include <cwchar>

namespace Compat32
{
	std::wstring getModulePath(HMODULE module)
	{
		wchar_t path[MAX_PATH] = {};
		GetModuleFileNameW(module, path, MAX_PATH);
		return std::wstring(path);
	}

	std::wstring getSystemPath()
	{
		wchar_t path[MAX_PATH] = {};
		GetSystemDirectoryW(path, MAX_PATH);
		return std::wstring(path);
	}

	std::wstring getParentPath(const std::wstring& path)
	{
		size_t pos = path.find_last_of(L"\\/");
		if (pos == std::wstring::npos)
		{
			return L""; // no parent, return empty
		}
		return path.substr(0, pos);
	}

	std::string toUtf8(const std::wstring& wstr)
	{
		if (wstr.empty())
			return {};

		int sizeNeeded = WideCharToMultiByte(
			CP_UTF8, 0, wstr.c_str(), (int)wstr.size(),
			nullptr, 0, nullptr, nullptr);

		std::string str(sizeNeeded, 0);
		WideCharToMultiByte(
			CP_UTF8, 0, wstr.c_str(), (int)wstr.size(),
			&str[0], sizeNeeded, nullptr, nullptr);

		return str;
	}

	bool isEqual(const std::wstring& p1, const std::wstring& p2)
	{
		return 0 == _wcsicmp(p1.c_str(), p2.c_str());
	}

	std::wstring replaceFilename(const std::wstring& path, const std::wstring& filename)
	{
		// Find the last slash or backslash
		size_t pos = path.find_last_of(L"\\/");

		if (pos == std::wstring::npos)
		{
			// No directory component, just return filename
			return filename;
		}

		return path.substr(0, pos + 1) + filename;
	}
}
