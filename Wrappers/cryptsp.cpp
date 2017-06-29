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

#include "Settings\Settings.h"
#include "wrapper.h"
#include "Utils\Utils.h"

struct cryptsp_dll
{
	HMODULE dll = nullptr;
	FARPROC CheckSignatureInFile = jmpaddr;
	FARPROC CryptAcquireContextA = jmpaddr;
	FARPROC CryptAcquireContextW = jmpaddr;
	FARPROC CryptContextAddRef = jmpaddr;
	FARPROC CryptCreateHash = jmpaddr;
	FARPROC CryptDecrypt = jmpaddr;
	FARPROC CryptDeriveKey = jmpaddr;
	FARPROC CryptDestroyHash = jmpaddr;
	FARPROC CryptDestroyKey = jmpaddr;
	FARPROC CryptDuplicateHash = jmpaddr;
	FARPROC CryptDuplicateKey = jmpaddr;
	FARPROC CryptEncrypt = jmpaddr;
	FARPROC CryptEnumProviderTypesA = jmpaddr;
	FARPROC CryptEnumProviderTypesW = jmpaddr;
	FARPROC CryptEnumProvidersA = jmpaddr;
	FARPROC CryptEnumProvidersW = jmpaddr;
	FARPROC CryptExportKey = jmpaddr;
	FARPROC CryptGenKey = jmpaddr;
	FARPROC CryptGenRandom = jmpaddr;
	FARPROC CryptGetDefaultProviderA = jmpaddr;
	FARPROC CryptGetDefaultProviderW = jmpaddr;
	FARPROC CryptGetHashParam = jmpaddr;
	FARPROC CryptGetKeyParam = jmpaddr;
	FARPROC CryptGetProvParam = jmpaddr;
	FARPROC CryptGetUserKey = jmpaddr;
	FARPROC CryptHashData = jmpaddr;
	FARPROC CryptHashSessionKey = jmpaddr;
	FARPROC CryptImportKey = jmpaddr;
	FARPROC CryptReleaseContext = jmpaddr;
	FARPROC CryptSetHashParam = jmpaddr;
	FARPROC CryptSetKeyParam = jmpaddr;
	FARPROC CryptSetProvParam = jmpaddr;
	FARPROC CryptSetProviderA = jmpaddr;
	FARPROC CryptSetProviderExA = jmpaddr;
	FARPROC CryptSetProviderExW = jmpaddr;
	FARPROC CryptSetProviderW = jmpaddr;
	FARPROC CryptSignHashA = jmpaddr;
	FARPROC CryptSignHashW = jmpaddr;
	FARPROC CryptVerifySignatureA = jmpaddr;
	FARPROC CryptVerifySignatureW = jmpaddr;
	FARPROC SystemFunction006 = jmpaddr;
	FARPROC SystemFunction007 = jmpaddr;
	FARPROC SystemFunction008 = jmpaddr;
	FARPROC SystemFunction009 = jmpaddr;
	FARPROC SystemFunction010 = jmpaddr;
	FARPROC SystemFunction011 = jmpaddr;
	FARPROC SystemFunction012 = jmpaddr;
	FARPROC SystemFunction013 = jmpaddr;
	FARPROC SystemFunction014 = jmpaddr;
	FARPROC SystemFunction015 = jmpaddr;
	FARPROC SystemFunction016 = jmpaddr;
	FARPROC SystemFunction018 = jmpaddr;
	FARPROC SystemFunction020 = jmpaddr;
	FARPROC SystemFunction021 = jmpaddr;
	FARPROC SystemFunction022 = jmpaddr;
	FARPROC SystemFunction023 = jmpaddr;
	FARPROC SystemFunction024 = jmpaddr;
	FARPROC SystemFunction025 = jmpaddr;
	FARPROC SystemFunction026 = jmpaddr;
	FARPROC SystemFunction027 = jmpaddr;
	FARPROC SystemFunction030 = jmpaddr;
	FARPROC SystemFunction031 = jmpaddr;
	FARPROC SystemFunction032 = jmpaddr;
	FARPROC SystemFunction033 = jmpaddr;
	FARPROC SystemFunction035 = jmpaddr;
} cryptsp;

__declspec(naked) void FakeCheckSignatureInFile() { _asm { jmp[cryptsp.CheckSignatureInFile] } }
__declspec(naked) void FakeCryptAcquireContextA() { _asm { jmp[cryptsp.CryptAcquireContextA] } }
__declspec(naked) void FakeCryptAcquireContextW() { _asm { jmp[cryptsp.CryptAcquireContextW] } }
__declspec(naked) void FakeCryptContextAddRef() { _asm { jmp[cryptsp.CryptContextAddRef] } }
__declspec(naked) void FakeCryptCreateHash() { _asm { jmp[cryptsp.CryptCreateHash] } }
__declspec(naked) void FakeCryptDecrypt() { _asm { jmp[cryptsp.CryptDecrypt] } }
__declspec(naked) void FakeCryptDeriveKey() { _asm { jmp[cryptsp.CryptDeriveKey] } }
__declspec(naked) void FakeCryptDestroyHash() { _asm { jmp[cryptsp.CryptDestroyHash] } }
__declspec(naked) void FakeCryptDestroyKey() { _asm { jmp[cryptsp.CryptDestroyKey] } }
__declspec(naked) void FakeCryptDuplicateHash() { _asm { jmp[cryptsp.CryptDuplicateHash] } }
__declspec(naked) void FakeCryptDuplicateKey() { _asm { jmp[cryptsp.CryptDuplicateKey] } }
__declspec(naked) void FakeCryptEncrypt() { _asm { jmp[cryptsp.CryptEncrypt] } }
__declspec(naked) void FakeCryptEnumProviderTypesA() { _asm { jmp[cryptsp.CryptEnumProviderTypesA] } }
__declspec(naked) void FakeCryptEnumProviderTypesW() { _asm { jmp[cryptsp.CryptEnumProviderTypesW] } }
__declspec(naked) void FakeCryptEnumProvidersA() { _asm { jmp[cryptsp.CryptEnumProvidersA] } }
__declspec(naked) void FakeCryptEnumProvidersW() { _asm { jmp[cryptsp.CryptEnumProvidersW] } }
__declspec(naked) void FakeCryptExportKey() { _asm { jmp[cryptsp.CryptExportKey] } }
__declspec(naked) void FakeCryptGenKey() { _asm { jmp[cryptsp.CryptGenKey] } }
__declspec(naked) void FakeCryptGenRandom() { _asm { jmp[cryptsp.CryptGenRandom] } }
__declspec(naked) void FakeCryptGetDefaultProviderA() { _asm { jmp[cryptsp.CryptGetDefaultProviderA] } }
__declspec(naked) void FakeCryptGetDefaultProviderW() { _asm { jmp[cryptsp.CryptGetDefaultProviderW] } }
__declspec(naked) void FakeCryptGetHashParam() { _asm { jmp[cryptsp.CryptGetHashParam] } }
__declspec(naked) void FakeCryptGetKeyParam() { _asm { jmp[cryptsp.CryptGetKeyParam] } }
__declspec(naked) void FakeCryptGetProvParam() { _asm { jmp[cryptsp.CryptGetProvParam] } }
__declspec(naked) void FakeCryptGetUserKey() { _asm { jmp[cryptsp.CryptGetUserKey] } }
__declspec(naked) void FakeCryptHashData() { _asm { jmp[cryptsp.CryptHashData] } }
__declspec(naked) void FakeCryptHashSessionKey() { _asm { jmp[cryptsp.CryptHashSessionKey] } }
__declspec(naked) void FakeCryptImportKey() { _asm { jmp[cryptsp.CryptImportKey] } }
__declspec(naked) void FakeCryptReleaseContext() { _asm { jmp[cryptsp.CryptReleaseContext] } }
__declspec(naked) void FakeCryptSetHashParam() { _asm { jmp[cryptsp.CryptSetHashParam] } }
__declspec(naked) void FakeCryptSetKeyParam() { _asm { jmp[cryptsp.CryptSetKeyParam] } }
__declspec(naked) void FakeCryptSetProvParam() { _asm { jmp[cryptsp.CryptSetProvParam] } }
__declspec(naked) void FakeCryptSetProviderA() { _asm { jmp[cryptsp.CryptSetProviderA] } }
__declspec(naked) void FakeCryptSetProviderExA() { _asm { jmp[cryptsp.CryptSetProviderExA] } }
__declspec(naked) void FakeCryptSetProviderExW() { _asm { jmp[cryptsp.CryptSetProviderExW] } }
__declspec(naked) void FakeCryptSetProviderW() { _asm { jmp[cryptsp.CryptSetProviderW] } }
__declspec(naked) void FakeCryptSignHashA() { _asm { jmp[cryptsp.CryptSignHashA] } }
__declspec(naked) void FakeCryptSignHashW() { _asm { jmp[cryptsp.CryptSignHashW] } }
__declspec(naked) void FakeCryptVerifySignatureA() { _asm { jmp[cryptsp.CryptVerifySignatureA] } }
__declspec(naked) void FakeCryptVerifySignatureW() { _asm { jmp[cryptsp.CryptVerifySignatureW] } }
__declspec(naked) void FakeSystemFunction006() { _asm { jmp[cryptsp.SystemFunction006] } }
__declspec(naked) void FakeSystemFunction007() { _asm { jmp[cryptsp.SystemFunction007] } }
__declspec(naked) void FakeSystemFunction008() { _asm { jmp[cryptsp.SystemFunction008] } }
__declspec(naked) void FakeSystemFunction009() { _asm { jmp[cryptsp.SystemFunction009] } }
__declspec(naked) void FakeSystemFunction010() { _asm { jmp[cryptsp.SystemFunction010] } }
__declspec(naked) void FakeSystemFunction011() { _asm { jmp[cryptsp.SystemFunction011] } }
__declspec(naked) void FakeSystemFunction012() { _asm { jmp[cryptsp.SystemFunction012] } }
__declspec(naked) void FakeSystemFunction013() { _asm { jmp[cryptsp.SystemFunction013] } }
__declspec(naked) void FakeSystemFunction014() { _asm { jmp[cryptsp.SystemFunction014] } }
__declspec(naked) void FakeSystemFunction015() { _asm { jmp[cryptsp.SystemFunction015] } }
__declspec(naked) void FakeSystemFunction016() { _asm { jmp[cryptsp.SystemFunction016] } }
__declspec(naked) void FakeSystemFunction018() { _asm { jmp[cryptsp.SystemFunction018] } }
__declspec(naked) void FakeSystemFunction020() { _asm { jmp[cryptsp.SystemFunction020] } }
__declspec(naked) void FakeSystemFunction021() { _asm { jmp[cryptsp.SystemFunction021] } }
__declspec(naked) void FakeSystemFunction022() { _asm { jmp[cryptsp.SystemFunction022] } }
__declspec(naked) void FakeSystemFunction023() { _asm { jmp[cryptsp.SystemFunction023] } }
__declspec(naked) void FakeSystemFunction024() { _asm { jmp[cryptsp.SystemFunction024] } }
__declspec(naked) void FakeSystemFunction025() { _asm { jmp[cryptsp.SystemFunction025] } }
__declspec(naked) void FakeSystemFunction026() { _asm { jmp[cryptsp.SystemFunction026] } }
__declspec(naked) void FakeSystemFunction027() { _asm { jmp[cryptsp.SystemFunction027] } }
__declspec(naked) void FakeSystemFunction030() { _asm { jmp[cryptsp.SystemFunction030] } }
__declspec(naked) void FakeSystemFunction031() { _asm { jmp[cryptsp.SystemFunction031] } }
__declspec(naked) void FakeSystemFunction032() { _asm { jmp[cryptsp.SystemFunction032] } }
__declspec(naked) void FakeSystemFunction033() { _asm { jmp[cryptsp.SystemFunction033] } }
__declspec(naked) void FakeSystemFunction035() { _asm { jmp[cryptsp.SystemFunction035] } }

void LoadCryptsp()
{
	// Load real dll
	cryptsp.dll = LoadDll(dtype.cryptsp);

	// Load dll functions
	if (cryptsp.dll)
	{
		cryptsp.CheckSignatureInFile = GetFunctionAddress(cryptsp.dll, "CheckSignatureInFile", jmpaddr);
		cryptsp.CryptAcquireContextA = GetFunctionAddress(cryptsp.dll, "CryptAcquireContextA", jmpaddr);
		cryptsp.CryptAcquireContextW = GetFunctionAddress(cryptsp.dll, "CryptAcquireContextW", jmpaddr);
		cryptsp.CryptContextAddRef = GetFunctionAddress(cryptsp.dll, "CryptContextAddRef", jmpaddr);
		cryptsp.CryptCreateHash = GetFunctionAddress(cryptsp.dll, "CryptCreateHash", jmpaddr);
		cryptsp.CryptDecrypt = GetFunctionAddress(cryptsp.dll, "CryptDecrypt", jmpaddr);
		cryptsp.CryptDeriveKey = GetFunctionAddress(cryptsp.dll, "CryptDeriveKey", jmpaddr);
		cryptsp.CryptDestroyHash = GetFunctionAddress(cryptsp.dll, "CryptDestroyHash", jmpaddr);
		cryptsp.CryptDestroyKey = GetFunctionAddress(cryptsp.dll, "CryptDestroyKey", jmpaddr);
		cryptsp.CryptDuplicateHash = GetFunctionAddress(cryptsp.dll, "CryptDuplicateHash", jmpaddr);
		cryptsp.CryptDuplicateKey = GetFunctionAddress(cryptsp.dll, "CryptDuplicateKey", jmpaddr);
		cryptsp.CryptEncrypt = GetFunctionAddress(cryptsp.dll, "CryptEncrypt", jmpaddr);
		cryptsp.CryptEnumProviderTypesA = GetFunctionAddress(cryptsp.dll, "CryptEnumProviderTypesA", jmpaddr);
		cryptsp.CryptEnumProviderTypesW = GetFunctionAddress(cryptsp.dll, "CryptEnumProviderTypesW", jmpaddr);
		cryptsp.CryptEnumProvidersA = GetFunctionAddress(cryptsp.dll, "CryptEnumProvidersA", jmpaddr);
		cryptsp.CryptEnumProvidersW = GetFunctionAddress(cryptsp.dll, "CryptEnumProvidersW", jmpaddr);
		cryptsp.CryptExportKey = GetFunctionAddress(cryptsp.dll, "CryptExportKey", jmpaddr);
		cryptsp.CryptGenKey = GetFunctionAddress(cryptsp.dll, "CryptGenKey", jmpaddr);
		cryptsp.CryptGenRandom = GetFunctionAddress(cryptsp.dll, "CryptGenRandom", jmpaddr);
		cryptsp.CryptGetDefaultProviderA = GetFunctionAddress(cryptsp.dll, "CryptGetDefaultProviderA", jmpaddr);
		cryptsp.CryptGetDefaultProviderW = GetFunctionAddress(cryptsp.dll, "CryptGetDefaultProviderW", jmpaddr);
		cryptsp.CryptGetHashParam = GetFunctionAddress(cryptsp.dll, "CryptGetHashParam", jmpaddr);
		cryptsp.CryptGetKeyParam = GetFunctionAddress(cryptsp.dll, "CryptGetKeyParam", jmpaddr);
		cryptsp.CryptGetProvParam = GetFunctionAddress(cryptsp.dll, "CryptGetProvParam", jmpaddr);
		cryptsp.CryptGetUserKey = GetFunctionAddress(cryptsp.dll, "CryptGetUserKey", jmpaddr);
		cryptsp.CryptHashData = GetFunctionAddress(cryptsp.dll, "CryptHashData", jmpaddr);
		cryptsp.CryptHashSessionKey = GetFunctionAddress(cryptsp.dll, "CryptHashSessionKey", jmpaddr);
		cryptsp.CryptImportKey = GetFunctionAddress(cryptsp.dll, "CryptImportKey", jmpaddr);
		cryptsp.CryptReleaseContext = GetFunctionAddress(cryptsp.dll, "CryptReleaseContext", jmpaddr);
		cryptsp.CryptSetHashParam = GetFunctionAddress(cryptsp.dll, "CryptSetHashParam", jmpaddr);
		cryptsp.CryptSetKeyParam = GetFunctionAddress(cryptsp.dll, "CryptSetKeyParam", jmpaddr);
		cryptsp.CryptSetProvParam = GetFunctionAddress(cryptsp.dll, "CryptSetProvParam", jmpaddr);
		cryptsp.CryptSetProviderA = GetFunctionAddress(cryptsp.dll, "CryptSetProviderA", jmpaddr);
		cryptsp.CryptSetProviderExA = GetFunctionAddress(cryptsp.dll, "CryptSetProviderExA", jmpaddr);
		cryptsp.CryptSetProviderExW = GetFunctionAddress(cryptsp.dll, "CryptSetProviderExW", jmpaddr);
		cryptsp.CryptSetProviderW = GetFunctionAddress(cryptsp.dll, "CryptSetProviderW", jmpaddr);
		cryptsp.CryptSignHashA = GetFunctionAddress(cryptsp.dll, "CryptSignHashA", jmpaddr);
		cryptsp.CryptSignHashW = GetFunctionAddress(cryptsp.dll, "CryptSignHashW", jmpaddr);
		cryptsp.CryptVerifySignatureA = GetFunctionAddress(cryptsp.dll, "CryptVerifySignatureA", jmpaddr);
		cryptsp.CryptVerifySignatureW = GetFunctionAddress(cryptsp.dll, "CryptVerifySignatureW", jmpaddr);
		cryptsp.SystemFunction006 = GetFunctionAddress(cryptsp.dll, "SystemFunction006", jmpaddr);
		cryptsp.SystemFunction007 = GetFunctionAddress(cryptsp.dll, "SystemFunction007", jmpaddr);
		cryptsp.SystemFunction008 = GetFunctionAddress(cryptsp.dll, "SystemFunction008", jmpaddr);
		cryptsp.SystemFunction009 = GetFunctionAddress(cryptsp.dll, "SystemFunction009", jmpaddr);
		cryptsp.SystemFunction010 = GetFunctionAddress(cryptsp.dll, "SystemFunction010", jmpaddr);
		cryptsp.SystemFunction011 = GetFunctionAddress(cryptsp.dll, "SystemFunction011", jmpaddr);
		cryptsp.SystemFunction012 = GetFunctionAddress(cryptsp.dll, "SystemFunction012", jmpaddr);
		cryptsp.SystemFunction013 = GetFunctionAddress(cryptsp.dll, "SystemFunction013", jmpaddr);
		cryptsp.SystemFunction014 = GetFunctionAddress(cryptsp.dll, "SystemFunction014", jmpaddr);
		cryptsp.SystemFunction015 = GetFunctionAddress(cryptsp.dll, "SystemFunction015", jmpaddr);
		cryptsp.SystemFunction016 = GetFunctionAddress(cryptsp.dll, "SystemFunction016", jmpaddr);
		cryptsp.SystemFunction018 = GetFunctionAddress(cryptsp.dll, "SystemFunction018", jmpaddr);
		cryptsp.SystemFunction020 = GetFunctionAddress(cryptsp.dll, "SystemFunction020", jmpaddr);
		cryptsp.SystemFunction021 = GetFunctionAddress(cryptsp.dll, "SystemFunction021", jmpaddr);
		cryptsp.SystemFunction022 = GetFunctionAddress(cryptsp.dll, "SystemFunction022", jmpaddr);
		cryptsp.SystemFunction023 = GetFunctionAddress(cryptsp.dll, "SystemFunction023", jmpaddr);
		cryptsp.SystemFunction024 = GetFunctionAddress(cryptsp.dll, "SystemFunction024", jmpaddr);
		cryptsp.SystemFunction025 = GetFunctionAddress(cryptsp.dll, "SystemFunction025", jmpaddr);
		cryptsp.SystemFunction026 = GetFunctionAddress(cryptsp.dll, "SystemFunction026", jmpaddr);
		cryptsp.SystemFunction027 = GetFunctionAddress(cryptsp.dll, "SystemFunction027", jmpaddr);
		cryptsp.SystemFunction030 = GetFunctionAddress(cryptsp.dll, "SystemFunction030", jmpaddr);
		cryptsp.SystemFunction031 = GetFunctionAddress(cryptsp.dll, "SystemFunction031", jmpaddr);
		cryptsp.SystemFunction032 = GetFunctionAddress(cryptsp.dll, "SystemFunction032", jmpaddr);
		cryptsp.SystemFunction033 = GetFunctionAddress(cryptsp.dll, "SystemFunction033", jmpaddr);
		cryptsp.SystemFunction035 = GetFunctionAddress(cryptsp.dll, "SystemFunction035", jmpaddr);
	}
}