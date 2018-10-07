#pragma once
#include <iostream>

// Send logging to standard out
namespace Logging
{
	using namespace std;
#define Log() cout
#define LogDebug() cout
	void LogFormat(char * fmt, ...)
	{
		// Format arg list
		va_list ap;
		va_start(ap, fmt);
		auto size = vsnprintf(nullptr, 0, fmt, ap);
		std::string output(size + 1, '\0');
		vsprintf_s(&output[0], size + 1, fmt, ap);
		va_end(ap);

		// Log formated text
		cout << output.c_str() << std::endl;
	}
}
