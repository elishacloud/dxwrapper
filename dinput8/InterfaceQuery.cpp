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

#include "dinput8.h"

REFIID IID_IDirectInputEffect8A = IID_IDirectInputEffect;
REFIID IID_IDirectInputEffect8W = IID_IDirectInputEffect;

void WINAPI Dinput8Wrapper::genericQueryInterface(REFIID riid, LPVOID * ppvObj)
{
	if (!ppvObj || !*ppvObj)
	{
		return;
	}

#define QUERYINTERFACE(x) \
	if (riid == IID_ ## x ## A) \
		{ \
			*ppvObj = ProxyAddressLookupTableDinput8.FindAddress<m_ ## x>(*ppvObj, riid); \
		} \
	if (riid == IID_ ## x ## W) \
		{ \
			*ppvObj = ProxyAddressLookupTableDinput8.FindAddress<m_ ## x>(*ppvObj, riid); \
		}

	QUERYINTERFACE(IDirectInput8);
	QUERYINTERFACE(IDirectInputDevice8);
	QUERYINTERFACE(IDirectInputEffect8);
}
