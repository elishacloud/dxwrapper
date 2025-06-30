#pragma once

namespace Settings
{
	typedef void(__stdcall* NV)(char* name, char* value);
	char* ReadFileContent(const char* filename);
	void Parse(char* buffer, NV callback);
}
