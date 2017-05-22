/**
* Copyright (C) 2017 Elisha Riedlinger
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

#include "cfg.h"
#include "wrapper.h"

struct dplayx_dll
{
	HMODULE dll = nullptr;
	FARPROC DirectPlayCreate;
	FARPROC DirectPlayEnumerate;
	FARPROC DirectPlayEnumerateA;
	FARPROC DirectPlayEnumerateW;
	FARPROC DirectPlayLobbyCreateA;
	FARPROC DirectPlayLobbyCreateW;
} dplayx;

__declspec(naked) void FakeDirectPlayCreate()				{ _asm { jmp [dplayx.DirectPlayCreate] } }
__declspec(naked) void FakeDirectPlayEnumerate()			{ _asm { jmp [dplayx.DirectPlayEnumerate] } }
__declspec(naked) void FakeDirectPlayEnumerateA()			{ _asm { jmp [dplayx.DirectPlayEnumerateA] } }
__declspec(naked) void FakeDirectPlayEnumerateW()			{ _asm { jmp [dplayx.DirectPlayEnumerateW] } }
__declspec(naked) void FakeDirectPlayLobbyCreateA()			{ _asm { jmp [dplayx.DirectPlayLobbyCreateA] } }
__declspec(naked) void FakeDirectPlayLobbyCreateW()			{ _asm { jmp [dplayx.DirectPlayLobbyCreateW] } }

void LoadDplayx()
{
	// Load real dll
	dplayx.dll = LoadDll(dtype.dplayx);
	// Load dll functions
	if (dplayx.dll)
	{
		dplayx.DirectPlayCreate				= GetProcAddress(dplayx.dll, "DirectPlayCreate");
		dplayx.DirectPlayEnumerate			= GetProcAddress(dplayx.dll, "DirectPlayEnumerate");
		dplayx.DirectPlayEnumerateA			= GetProcAddress(dplayx.dll, "DirectPlayEnumerateA");
		dplayx.DirectPlayEnumerateW			= GetProcAddress(dplayx.dll, "DirectPlayEnumerateW");
		dplayx.DirectPlayLobbyCreateA		= GetProcAddress(dplayx.dll, "DirectPlayLobbyCreateA");
		dplayx.DirectPlayLobbyCreateW		= GetProcAddress(dplayx.dll, "DirectPlayLobbyCreateW");
	}
}

void FreeDplayxLibrary()
{
	if (dplayx.dll) FreeLibrary(dplayx.dll);
}