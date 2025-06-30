/**
* Copyright (C) 2025 Elisha Riedlinger
*
* This software is  provided 'as-is', without any express  or implied  warranty. In no event will the
* authors be held liable for any damages arising from the use of this software.
* Permission  is granted  to anyone  to use  this software  for  any  purpose,  including  commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*   1. The origin of this software must not be misrepresented; you must not claim that you  wrote the
*      original  software. If you use this  software  in a product, an  acknowledgment in the product
*      documentation would be appreciated but is not required.
*   2. Altered source versions must  be plainly  marked as such, and  must not be  misrepresented  as
*      being the original software.
*   3. This notice may not be removed or altered from any source distribution.
*/

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <functional>
#include "ReadParse.h"

// Type alias for the callback used when parsing name/value pairs
using NV = std::function<void(const char*, const char*)>;

namespace Settings
{
	// Reads entire contents of a file into a dynamically allocated buffer
	char* ReadFileContent(const char* filename)
	{
		if (!filename) return nullptr;

		HANDLE file = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (file == INVALID_HANDLE_VALUE)
		{
			return nullptr;
		}

		DWORD fileSize = GetFileSize(file, nullptr);
		if (fileSize == 0 || fileSize == INVALID_FILE_SIZE)
		{
			CloseHandle(file);
			return nullptr;
		}

		char* buffer = static_cast<char*>(malloc(fileSize + 1)); // +1 for null terminator
		if (!buffer)
		{
			CloseHandle(file);
			return nullptr;
		}

		DWORD bytesRead = 0;
		BOOL success = ReadFile(file, buffer, fileSize, &bytesRead, nullptr);
		CloseHandle(file);

		if (!success || bytesRead == 0)
		{
			free(buffer);
			return nullptr;
		}

		buffer[bytesRead] = '\0';
		return buffer;
	}

	// Returns true if a name and value pair are valid
	static bool IsValidEntry(const char* name, const char* value)
	{
		if (!name || !value) return false;
		if (!*name || !*value) return false;
		if (_stricmp(value, "AUTO") == 0) return false;
		return true;
	}

	// Strips C++ style comments from a text buffer in place
	static void StripComments(char* text)
	{
		if (!text) return;

		char* p = text;

		while (true)
		{
			p = strchr(p, '/');
			if (!p) break;

			if (p[1] == '/')
			{
				// Line comment: replace until newline
				char* end = strchr(p, '\n');
				if (!end) end = p + strlen(p);  // if no newline, go to end
				while (p < end) *p++ = ' ';
			}
			else if (p[1] == '*')
			{
				// Block comment: replace until */
				char* q = p + 2;
				while (q[0] && !(q[0] == '*' && q[1] == '/'))
				{
					*q++ = ' ';
				}
				if (q[0]) *q++ = ' ';
				if (q[0]) *q++ = ' ';
				while (p < q) *p++ = ' ';  // Overwrite the comment
			}
			else
			{
				// Not a comment, skip this '/'
				++p;
			}
		}
	}

	static void TrimWhitespace(char*& str)
	{
		if (!str || !*str) return;

		// Trim leading whitespace
		while (*str == ' ' || *str == '\t' || *str == '\r' || *str == '\n')
		{
			++str;
		}

		// Trim trailing whitespace
		size_t len = strlen(str);
		while (len > 0 && (str[len - 1] == ' ' || str[len - 1] == '\t' || str[len - 1] == '\r' || str[len - 1] == '\n'))
		{
			str[--len] = '\0';
		}
	}

	// Parses a configuration file buffer and invokes callback for each name=value or name:value entry
	void Parse(char* buffer, NV callback)
	{
		if (!buffer || !callback) return;

		StripComments(buffer);

		char* context = nullptr;
		for (char* line = strtok_s(buffer, "\n", &context); line; line = strtok_s(nullptr, "\n", &context))
		{
			// Skip blank or comment lines
			while (*line == ' ' || *line == '\t') ++line;
			if (!*line || *line == ';' || *line == '#') continue;

			// Find first delimiter
			char* eq = strchr(line, '=');
			char* colon = strchr(line, ':');
			char* delimiter = (!eq) ? colon : (!colon) ? eq : (eq < colon ? eq : colon);
			if (!delimiter) continue;

			*delimiter = '\0';
			char* name = line;
			char* value = delimiter + 1;

			TrimWhitespace(name);
			TrimWhitespace(value);

			if (*name && *value && IsValidEntry(name, value))
			{
				callback(name, value);
			}
		}
	}
}
