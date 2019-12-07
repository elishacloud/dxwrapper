#pragma once
#include <iostream>

// Send logging to standard out
namespace Logging
{
	using namespace std;
#define Log() cout
#define LogDebug() cout
	void LogFormat(char * fmt, ...);
}
