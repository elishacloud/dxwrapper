#pragma once
#include <iostream>
#include <fstream>

// Send logging to standard out
namespace Logging
{
	class Log
	{
	public:
		Log() {}
		~Log()
		{
			if (LOG.is_open())
			{
				LOG << std::endl;
			}
		}

		template <typename T>
		Log& operator<<(const T& t)
		{
			if (LOG.is_open())
			{
				LOG << t;
			}
			return *this;
		}
	private:
		static std::ofstream LOG;
	};

	static std::ostream& operator<<(std::ostream& os, const wchar_t* wchr)
	{
		std::wstring ws(wchr);
		return os << std::string(ws.begin(), ws.end()).c_str();
	}

#pragma warning(suppress: 4505)
	static void LogFormat(char * fmt, ...)
	{
		va_list ap;
		va_start(ap, fmt);
		auto size = vsnprintf(nullptr, 0, fmt, ap);
		std::string output(size + 1, '\0');
		vsprintf_s(&output[0], size + 1, fmt, ap);
		Log() << output.c_str();
		va_end(ap);
	}

#pragma warning(suppress: 4505)
	static void LogFormat(wchar_t * fmt, ...)
	{
		va_list ap;
		va_start(ap, fmt);
#pragma warning(suppress: 4996)
		auto size = _vsnwprintf(nullptr, 0, fmt, ap);
		std::wstring output(size + 1, '\0');
		vswprintf_s(&output[0], size + 1, fmt, ap);
		Log() << output.c_str();
		va_end(ap);
	}

#define LogDebug() Log()
}
