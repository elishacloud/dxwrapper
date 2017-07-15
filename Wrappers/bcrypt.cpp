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

#define module bcrypt

#define VISIT_PROCS(visit) \
	visit(BCryptAddContextFunction) \
	visit(BCryptAddContextFunctionProvider) \
	visit(BCryptCloseAlgorithmProvider) \
	visit(BCryptConfigureContext) \
	visit(BCryptConfigureContextFunction) \
	visit(BCryptCreateContext) \
	visit(BCryptCreateHash) \
	visit(BCryptCreateMultiHash) \
	visit(BCryptDecrypt) \
	visit(BCryptDeleteContext) \
	visit(BCryptDeriveKey) \
	visit(BCryptDeriveKeyCapi) \
	visit(BCryptDeriveKeyPBKDF2) \
	visit(BCryptDestroyHash) \
	visit(BCryptDestroyKey) \
	visit(BCryptDestroySecret) \
	visit(BCryptDuplicateHash) \
	visit(BCryptDuplicateKey) \
	visit(BCryptEncrypt) \
	visit(BCryptEnumAlgorithms) \
	visit(BCryptEnumContextFunctionProviders) \
	visit(BCryptEnumContextFunctions) \
	visit(BCryptEnumContexts) \
	visit(BCryptEnumProviders) \
	visit(BCryptEnumRegisteredProviders) \
	visit(BCryptExportKey) \
	visit(BCryptFinalizeKeyPair) \
	visit(BCryptFinishHash) \
	visit(BCryptFreeBuffer) \
	visit(BCryptGenRandom) \
	visit(BCryptGenerateKeyPair) \
	visit(BCryptGenerateSymmetricKey) \
	visit(BCryptGetFipsAlgorithmMode) \
	visit(BCryptGetProperty) \
	visit(BCryptHash) \
	visit(BCryptHashData) \
	visit(BCryptImportKey) \
	visit(BCryptImportKeyPair) \
	visit(BCryptKeyDerivation) \
	visit(BCryptOpenAlgorithmProvider) \
	visit(BCryptProcessMultiOperations) \
	visit(BCryptQueryContextConfiguration) \
	visit(BCryptQueryContextFunctionConfiguration) \
	visit(BCryptQueryContextFunctionProperty) \
	visit(BCryptQueryProviderRegistration) \
	visit(BCryptRegisterConfigChangeNotify) \
	visit(BCryptRegisterProvider) \
	visit(BCryptRemoveContextFunction) \
	visit(BCryptRemoveContextFunctionProvider) \
	visit(BCryptResolveProviders) \
	visit(BCryptSecretAgreement) \
	visit(BCryptSetAuditingInterface) \
	visit(BCryptSetContextFunctionProperty) \
	visit(BCryptSetProperty) \
	visit(BCryptSignHash) \
	visit(BCryptUnregisterConfigChangeNotify) \
	visit(BCryptUnregisterProvider) \
	visit(BCryptVerifySignature)

struct bcrypt_dll
{
	HMODULE dll = nullptr;
	VISIT_PROCS(ADD_FARPROC_MEMBER);
} bcrypt;

VISIT_PROCS(CREATE_PROC_STUB)

void LoadBcrypt()
{
	// Load real dll
	bcrypt.dll = LoadDll(dtype.bcrypt);

	// Load dll functions
	if (bcrypt.dll)
	{
		VISIT_PROCS(LOAD_ORIGINAL_PROC);
	}
}