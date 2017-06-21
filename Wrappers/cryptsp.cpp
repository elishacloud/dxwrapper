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

struct cryptsp_dll
{
	HMODULE dll = nullptr;
	FARPROC CheckSignatureInFile;
	FARPROC CryptAcquireContextA;
	FARPROC CryptAcquireContextW;
	FARPROC CryptContextAddRef;
	FARPROC CryptCreateHash;
	FARPROC CryptDecrypt;
	FARPROC CryptDeriveKey;
	FARPROC CryptDestroyHash;
	FARPROC CryptDestroyKey;
	FARPROC CryptDuplicateHash;
	FARPROC CryptDuplicateKey;
	FARPROC CryptEncrypt;
	FARPROC CryptEnumProviderTypesA;
	FARPROC CryptEnumProviderTypesW;
	FARPROC CryptEnumProvidersA;
	FARPROC CryptEnumProvidersW;
	FARPROC CryptExportKey;
	FARPROC CryptGenKey;
	FARPROC CryptGenRandom;
	FARPROC CryptGetDefaultProviderA;
	FARPROC CryptGetDefaultProviderW;
	FARPROC CryptGetHashParam;
	FARPROC CryptGetKeyParam;
	FARPROC CryptGetProvParam;
	FARPROC CryptGetUserKey;
	FARPROC CryptHashData;
	FARPROC CryptHashSessionKey;
	FARPROC CryptImportKey;
	FARPROC CryptReleaseContext;
	FARPROC CryptSetHashParam;
	FARPROC CryptSetKeyParam;
	FARPROC CryptSetProvParam;
	FARPROC CryptSetProviderA;
	FARPROC CryptSetProviderExA;
	FARPROC CryptSetProviderExW;
	FARPROC CryptSetProviderW;
	FARPROC CryptSignHashA;
	FARPROC CryptSignHashW;
	FARPROC CryptVerifySignatureA;
	FARPROC CryptVerifySignatureW;
	FARPROC SystemFunction006;
	FARPROC SystemFunction007;
	FARPROC SystemFunction008;
	FARPROC SystemFunction009;
	FARPROC SystemFunction010;
	FARPROC SystemFunction011;
	FARPROC SystemFunction012;
	FARPROC SystemFunction013;
	FARPROC SystemFunction014;
	FARPROC SystemFunction015;
	FARPROC SystemFunction016;
	FARPROC SystemFunction018;
	FARPROC SystemFunction020;
	FARPROC SystemFunction021;
	FARPROC SystemFunction022;
	FARPROC SystemFunction023;
	FARPROC SystemFunction024;
	FARPROC SystemFunction025;
	FARPROC SystemFunction026;
	FARPROC SystemFunction027;
	FARPROC SystemFunction030;
	FARPROC SystemFunction031;
	FARPROC SystemFunction032;
	FARPROC SystemFunction033;
	FARPROC SystemFunction035;
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
		cryptsp.CheckSignatureInFile = GetProcAddress(cryptsp.dll, "CheckSignatureInFile");
		cryptsp.CryptAcquireContextA = GetProcAddress(cryptsp.dll, "CryptAcquireContextA");
		cryptsp.CryptAcquireContextW = GetProcAddress(cryptsp.dll, "CryptAcquireContextW");
		cryptsp.CryptContextAddRef = GetProcAddress(cryptsp.dll, "CryptContextAddRef");
		cryptsp.CryptCreateHash = GetProcAddress(cryptsp.dll, "CryptCreateHash");
		cryptsp.CryptDecrypt = GetProcAddress(cryptsp.dll, "CryptDecrypt");
		cryptsp.CryptDeriveKey = GetProcAddress(cryptsp.dll, "CryptDeriveKey");
		cryptsp.CryptDestroyHash = GetProcAddress(cryptsp.dll, "CryptDestroyHash");
		cryptsp.CryptDestroyKey = GetProcAddress(cryptsp.dll, "CryptDestroyKey");
		cryptsp.CryptDuplicateHash = GetProcAddress(cryptsp.dll, "CryptDuplicateHash");
		cryptsp.CryptDuplicateKey = GetProcAddress(cryptsp.dll, "CryptDuplicateKey");
		cryptsp.CryptEncrypt = GetProcAddress(cryptsp.dll, "CryptEncrypt");
		cryptsp.CryptEnumProviderTypesA = GetProcAddress(cryptsp.dll, "CryptEnumProviderTypesA");
		cryptsp.CryptEnumProviderTypesW = GetProcAddress(cryptsp.dll, "CryptEnumProviderTypesW");
		cryptsp.CryptEnumProvidersA = GetProcAddress(cryptsp.dll, "CryptEnumProvidersA");
		cryptsp.CryptEnumProvidersW = GetProcAddress(cryptsp.dll, "CryptEnumProvidersW");
		cryptsp.CryptExportKey = GetProcAddress(cryptsp.dll, "CryptExportKey");
		cryptsp.CryptGenKey = GetProcAddress(cryptsp.dll, "CryptGenKey");
		cryptsp.CryptGenRandom = GetProcAddress(cryptsp.dll, "CryptGenRandom");
		cryptsp.CryptGetDefaultProviderA = GetProcAddress(cryptsp.dll, "CryptGetDefaultProviderA");
		cryptsp.CryptGetDefaultProviderW = GetProcAddress(cryptsp.dll, "CryptGetDefaultProviderW");
		cryptsp.CryptGetHashParam = GetProcAddress(cryptsp.dll, "CryptGetHashParam");
		cryptsp.CryptGetKeyParam = GetProcAddress(cryptsp.dll, "CryptGetKeyParam");
		cryptsp.CryptGetProvParam = GetProcAddress(cryptsp.dll, "CryptGetProvParam");
		cryptsp.CryptGetUserKey = GetProcAddress(cryptsp.dll, "CryptGetUserKey");
		cryptsp.CryptHashData = GetProcAddress(cryptsp.dll, "CryptHashData");
		cryptsp.CryptHashSessionKey = GetProcAddress(cryptsp.dll, "CryptHashSessionKey");
		cryptsp.CryptImportKey = GetProcAddress(cryptsp.dll, "CryptImportKey");
		cryptsp.CryptReleaseContext = GetProcAddress(cryptsp.dll, "CryptReleaseContext");
		cryptsp.CryptSetHashParam = GetProcAddress(cryptsp.dll, "CryptSetHashParam");
		cryptsp.CryptSetKeyParam = GetProcAddress(cryptsp.dll, "CryptSetKeyParam");
		cryptsp.CryptSetProvParam = GetProcAddress(cryptsp.dll, "CryptSetProvParam");
		cryptsp.CryptSetProviderA = GetProcAddress(cryptsp.dll, "CryptSetProviderA");
		cryptsp.CryptSetProviderExA = GetProcAddress(cryptsp.dll, "CryptSetProviderExA");
		cryptsp.CryptSetProviderExW = GetProcAddress(cryptsp.dll, "CryptSetProviderExW");
		cryptsp.CryptSetProviderW = GetProcAddress(cryptsp.dll, "CryptSetProviderW");
		cryptsp.CryptSignHashA = GetProcAddress(cryptsp.dll, "CryptSignHashA");
		cryptsp.CryptSignHashW = GetProcAddress(cryptsp.dll, "CryptSignHashW");
		cryptsp.CryptVerifySignatureA = GetProcAddress(cryptsp.dll, "CryptVerifySignatureA");
		cryptsp.CryptVerifySignatureW = GetProcAddress(cryptsp.dll, "CryptVerifySignatureW");
		cryptsp.SystemFunction006 = GetProcAddress(cryptsp.dll, "SystemFunction006");
		cryptsp.SystemFunction007 = GetProcAddress(cryptsp.dll, "SystemFunction007");
		cryptsp.SystemFunction008 = GetProcAddress(cryptsp.dll, "SystemFunction008");
		cryptsp.SystemFunction009 = GetProcAddress(cryptsp.dll, "SystemFunction009");
		cryptsp.SystemFunction010 = GetProcAddress(cryptsp.dll, "SystemFunction010");
		cryptsp.SystemFunction011 = GetProcAddress(cryptsp.dll, "SystemFunction011");
		cryptsp.SystemFunction012 = GetProcAddress(cryptsp.dll, "SystemFunction012");
		cryptsp.SystemFunction013 = GetProcAddress(cryptsp.dll, "SystemFunction013");
		cryptsp.SystemFunction014 = GetProcAddress(cryptsp.dll, "SystemFunction014");
		cryptsp.SystemFunction015 = GetProcAddress(cryptsp.dll, "SystemFunction015");
		cryptsp.SystemFunction016 = GetProcAddress(cryptsp.dll, "SystemFunction016");
		cryptsp.SystemFunction018 = GetProcAddress(cryptsp.dll, "SystemFunction018");
		cryptsp.SystemFunction020 = GetProcAddress(cryptsp.dll, "SystemFunction020");
		cryptsp.SystemFunction021 = GetProcAddress(cryptsp.dll, "SystemFunction021");
		cryptsp.SystemFunction022 = GetProcAddress(cryptsp.dll, "SystemFunction022");
		cryptsp.SystemFunction023 = GetProcAddress(cryptsp.dll, "SystemFunction023");
		cryptsp.SystemFunction024 = GetProcAddress(cryptsp.dll, "SystemFunction024");
		cryptsp.SystemFunction025 = GetProcAddress(cryptsp.dll, "SystemFunction025");
		cryptsp.SystemFunction026 = GetProcAddress(cryptsp.dll, "SystemFunction026");
		cryptsp.SystemFunction027 = GetProcAddress(cryptsp.dll, "SystemFunction027");
		cryptsp.SystemFunction030 = GetProcAddress(cryptsp.dll, "SystemFunction030");
		cryptsp.SystemFunction031 = GetProcAddress(cryptsp.dll, "SystemFunction031");
		cryptsp.SystemFunction032 = GetProcAddress(cryptsp.dll, "SystemFunction032");
		cryptsp.SystemFunction033 = GetProcAddress(cryptsp.dll, "SystemFunction033");
		cryptsp.SystemFunction035 = GetProcAddress(cryptsp.dll, "SystemFunction035");
	}
}