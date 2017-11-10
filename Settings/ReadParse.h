#pragma once

namespace Settings
{
	typedef void(__stdcall* NV)(char* name, char* value);
	char* Read(char* szFileName);
	void Parse(char* str, NV NameValueCallback);
}
