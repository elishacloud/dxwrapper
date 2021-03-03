/**
* Copyright (C) 2021 Elisha Riedlinger
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

#include "Utils.h"

template<typename T>
bool isInString(T strCheck, T str, size_t size);

bool stristr(LPCSTR strCheck, LPCSTR str, size_t size)
{
	return isInString(strCheck, str, size);
}

bool wcsistr(LPCWSTR strCheck, LPCWSTR str, size_t size)
{
	return isInString(strCheck, str, size);
}

inline int mytolower(const char chr)
{
	return tolower((unsigned char)chr);
}

inline wint_t mytolower(const wchar_t chr)
{
	return towlower(chr);
}

template<typename T>
bool isInString(T strCheck, T str, size_t size)
{
	if (!strCheck || !str)
	{
		return false;
	}

	T p1 = strCheck;
	T p2 = str;
	T r = *p2 == 0 ? strCheck : 0;

	while (*p1 != 0 && *p2 != 0 && (size_t)(p1 - strCheck) < size)
	{
		if (mytolower(*p1) == mytolower(*p2))
		{
			if (r == 0)
			{
				r = p1;
			}

			p2++;
		}
		else
		{
			p2 = str;
			if (r != 0)
			{
				p1 = r + 1;
			}

			if (mytolower(*p1) == mytolower(*p2))
			{
				r = p1;
				p2++;
			}
			else
			{
				r = 0;
			}
		}

		p1++;
	}

	return (*p2 == 0) ? true : false;
}
