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

#define module cryptsp

#define VISIT_PROCS(visit) \
	visit(CheckSignatureInFile) \
	visit(CryptAcquireContextA) \
	visit(CryptAcquireContextW) \
	visit(CryptContextAddRef) \
	visit(CryptCreateHash) \
	visit(CryptDecrypt) \
	visit(CryptDeriveKey) \
	visit(CryptDestroyHash) \
	visit(CryptDestroyKey) \
	visit(CryptDuplicateHash) \
	visit(CryptDuplicateKey) \
	visit(CryptEncrypt) \
	visit(CryptEnumProviderTypesA) \
	visit(CryptEnumProviderTypesW) \
	visit(CryptEnumProvidersA) \
	visit(CryptEnumProvidersW) \
	visit(CryptExportKey) \
	visit(CryptGenKey) \
	visit(CryptGenRandom) \
	visit(CryptGetDefaultProviderA) \
	visit(CryptGetDefaultProviderW) \
	visit(CryptGetHashParam) \
	visit(CryptGetKeyParam) \
	visit(CryptGetProvParam) \
	visit(CryptGetUserKey) \
	visit(CryptHashData) \
	visit(CryptHashSessionKey) \
	visit(CryptImportKey) \
	visit(CryptReleaseContext) \
	visit(CryptSetHashParam) \
	visit(CryptSetKeyParam) \
	visit(CryptSetProvParam) \
	visit(CryptSetProviderA) \
	visit(CryptSetProviderExA) \
	visit(CryptSetProviderExW) \
	visit(CryptSetProviderW) \
	visit(CryptSignHashA) \
	visit(CryptSignHashW) \
	visit(CryptVerifySignatureA) \
	visit(CryptVerifySignatureW) \
	visit(SystemFunction006) \
	visit(SystemFunction007) \
	visit(SystemFunction008) \
	visit(SystemFunction009) \
	visit(SystemFunction010) \
	visit(SystemFunction011) \
	visit(SystemFunction012) \
	visit(SystemFunction013) \
	visit(SystemFunction014) \
	visit(SystemFunction015) \
	visit(SystemFunction016) \
	visit(SystemFunction018) \
	visit(SystemFunction020) \
	visit(SystemFunction021) \
	visit(SystemFunction022) \
	visit(SystemFunction023) \
	visit(SystemFunction024) \
	visit(SystemFunction025) \
	visit(SystemFunction026) \
	visit(SystemFunction027) \
	visit(SystemFunction030) \
	visit(SystemFunction031) \
	visit(SystemFunction032) \
	visit(SystemFunction033) \
	visit(SystemFunction035)

struct cryptsp_dll
{
	HMODULE dll = nullptr;
	VISIT_PROCS(ADD_FARPROC_MEMBER);
} cryptsp;

VISIT_PROCS(CREATE_PROC_STUB)

void LoadCryptsp()
{
	// Load real dll
	cryptsp.dll = LoadDll(dtype.cryptsp);

	// Load dll functions
	if (cryptsp.dll)
	{
		VISIT_PROCS(LOAD_ORIGINAL_PROC);
	}
}