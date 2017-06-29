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

struct bcrypt_dll
{
	HMODULE dll = nullptr;
	FARPROC BCryptAddContextFunction = jmpaddr;
	FARPROC BCryptAddContextFunctionProvider = jmpaddr;
	FARPROC BCryptCloseAlgorithmProvider = jmpaddr;
	FARPROC BCryptConfigureContext = jmpaddr;
	FARPROC BCryptConfigureContextFunction = jmpaddr;
	FARPROC BCryptCreateContext = jmpaddr;
	FARPROC BCryptCreateHash = jmpaddr;
	FARPROC BCryptCreateMultiHash = jmpaddr;
	FARPROC BCryptDecrypt = jmpaddr;
	FARPROC BCryptDeleteContext = jmpaddr;
	FARPROC BCryptDeriveKey = jmpaddr;
	FARPROC BCryptDeriveKeyCapi = jmpaddr;
	FARPROC BCryptDeriveKeyPBKDF2 = jmpaddr;
	FARPROC BCryptDestroyHash = jmpaddr;
	FARPROC BCryptDestroyKey = jmpaddr;
	FARPROC BCryptDestroySecret = jmpaddr;
	FARPROC BCryptDuplicateHash = jmpaddr;
	FARPROC BCryptDuplicateKey = jmpaddr;
	FARPROC BCryptEncrypt = jmpaddr;
	FARPROC BCryptEnumAlgorithms = jmpaddr;
	FARPROC BCryptEnumContextFunctionProviders = jmpaddr;
	FARPROC BCryptEnumContextFunctions = jmpaddr;
	FARPROC BCryptEnumContexts = jmpaddr;
	FARPROC BCryptEnumProviders = jmpaddr;
	FARPROC BCryptEnumRegisteredProviders = jmpaddr;
	FARPROC BCryptExportKey = jmpaddr;
	FARPROC BCryptFinalizeKeyPair = jmpaddr;
	FARPROC BCryptFinishHash = jmpaddr;
	FARPROC BCryptFreeBuffer = jmpaddr;
	FARPROC BCryptGenRandom = jmpaddr;
	FARPROC BCryptGenerateKeyPair = jmpaddr;
	FARPROC BCryptGenerateSymmetricKey = jmpaddr;
	FARPROC BCryptGetFipsAlgorithmMode = jmpaddr;
	FARPROC BCryptGetProperty = jmpaddr;
	FARPROC BCryptHash = jmpaddr;
	FARPROC BCryptHashData = jmpaddr;
	FARPROC BCryptImportKey = jmpaddr;
	FARPROC BCryptImportKeyPair = jmpaddr;
	FARPROC BCryptKeyDerivation = jmpaddr;
	FARPROC BCryptOpenAlgorithmProvider = jmpaddr;
	FARPROC BCryptProcessMultiOperations = jmpaddr;
	FARPROC BCryptQueryContextConfiguration = jmpaddr;
	FARPROC BCryptQueryContextFunctionConfiguration = jmpaddr;
	FARPROC BCryptQueryContextFunctionProperty = jmpaddr;
	FARPROC BCryptQueryProviderRegistration = jmpaddr;
	FARPROC BCryptRegisterConfigChangeNotify = jmpaddr;
	FARPROC BCryptRegisterProvider = jmpaddr;
	FARPROC BCryptRemoveContextFunction = jmpaddr;
	FARPROC BCryptRemoveContextFunctionProvider = jmpaddr;
	FARPROC BCryptResolveProviders = jmpaddr;
	FARPROC BCryptSecretAgreement = jmpaddr;
	FARPROC BCryptSetAuditingInterface = jmpaddr;
	FARPROC BCryptSetContextFunctionProperty = jmpaddr;
	FARPROC BCryptSetProperty = jmpaddr;
	FARPROC BCryptSignHash = jmpaddr;
	FARPROC BCryptUnregisterConfigChangeNotify = jmpaddr;
	FARPROC BCryptUnregisterProvider = jmpaddr;
	FARPROC BCryptVerifySignature = jmpaddr;
} bcrypt;

__declspec(naked) void FakeBCryptAddContextFunction() { _asm { jmp[bcrypt.BCryptAddContextFunction] } }
__declspec(naked) void FakeBCryptAddContextFunctionProvider() { _asm { jmp[bcrypt.BCryptAddContextFunctionProvider] } }
__declspec(naked) void FakeBCryptCloseAlgorithmProvider() { _asm { jmp[bcrypt.BCryptCloseAlgorithmProvider] } }
__declspec(naked) void FakeBCryptConfigureContext() { _asm { jmp[bcrypt.BCryptConfigureContext] } }
__declspec(naked) void FakeBCryptConfigureContextFunction() { _asm { jmp[bcrypt.BCryptConfigureContextFunction] } }
__declspec(naked) void FakeBCryptCreateContext() { _asm { jmp[bcrypt.BCryptCreateContext] } }
__declspec(naked) void FakeBCryptCreateHash() { _asm { jmp[bcrypt.BCryptCreateHash] } }
__declspec(naked) void FakeBCryptCreateMultiHash() { _asm { jmp[bcrypt.BCryptCreateMultiHash] } }
__declspec(naked) void FakeBCryptDecrypt() { _asm { jmp[bcrypt.BCryptDecrypt] } }
__declspec(naked) void FakeBCryptDeleteContext() { _asm { jmp[bcrypt.BCryptDeleteContext] } }
__declspec(naked) void FakeBCryptDeriveKey() { _asm { jmp[bcrypt.BCryptDeriveKey] } }
__declspec(naked) void FakeBCryptDeriveKeyCapi() { _asm { jmp[bcrypt.BCryptDeriveKeyCapi] } }
__declspec(naked) void FakeBCryptDeriveKeyPBKDF2() { _asm { jmp[bcrypt.BCryptDeriveKeyPBKDF2] } }
__declspec(naked) void FakeBCryptDestroyHash() { _asm { jmp[bcrypt.BCryptDestroyHash] } }
__declspec(naked) void FakeBCryptDestroyKey() { _asm { jmp[bcrypt.BCryptDestroyKey] } }
__declspec(naked) void FakeBCryptDestroySecret() { _asm { jmp[bcrypt.BCryptDestroySecret] } }
__declspec(naked) void FakeBCryptDuplicateHash() { _asm { jmp[bcrypt.BCryptDuplicateHash] } }
__declspec(naked) void FakeBCryptDuplicateKey() { _asm { jmp[bcrypt.BCryptDuplicateKey] } }
__declspec(naked) void FakeBCryptEncrypt() { _asm { jmp[bcrypt.BCryptEncrypt] } }
__declspec(naked) void FakeBCryptEnumAlgorithms() { _asm { jmp[bcrypt.BCryptEnumAlgorithms] } }
__declspec(naked) void FakeBCryptEnumContextFunctionProviders() { _asm { jmp[bcrypt.BCryptEnumContextFunctionProviders] } }
__declspec(naked) void FakeBCryptEnumContextFunctions() { _asm { jmp[bcrypt.BCryptEnumContextFunctions] } }
__declspec(naked) void FakeBCryptEnumContexts() { _asm { jmp[bcrypt.BCryptEnumContexts] } }
__declspec(naked) void FakeBCryptEnumProviders() { _asm { jmp[bcrypt.BCryptEnumProviders] } }
__declspec(naked) void FakeBCryptEnumRegisteredProviders() { _asm { jmp[bcrypt.BCryptEnumRegisteredProviders] } }
__declspec(naked) void FakeBCryptExportKey() { _asm { jmp[bcrypt.BCryptExportKey] } }
__declspec(naked) void FakeBCryptFinalizeKeyPair() { _asm { jmp[bcrypt.BCryptFinalizeKeyPair] } }
__declspec(naked) void FakeBCryptFinishHash() { _asm { jmp[bcrypt.BCryptFinishHash] } }
__declspec(naked) void FakeBCryptFreeBuffer() { _asm { jmp[bcrypt.BCryptFreeBuffer] } }
__declspec(naked) void FakeBCryptGenRandom() { _asm { jmp[bcrypt.BCryptGenRandom] } }
__declspec(naked) void FakeBCryptGenerateKeyPair() { _asm { jmp[bcrypt.BCryptGenerateKeyPair] } }
__declspec(naked) void FakeBCryptGenerateSymmetricKey() { _asm { jmp[bcrypt.BCryptGenerateSymmetricKey] } }
__declspec(naked) void FakeBCryptGetFipsAlgorithmMode() { _asm { jmp[bcrypt.BCryptGetFipsAlgorithmMode] } }
__declspec(naked) void FakeBCryptGetProperty() { _asm { jmp[bcrypt.BCryptGetProperty] } }
__declspec(naked) void FakeBCryptHash() { _asm { jmp[bcrypt.BCryptHash] } }
__declspec(naked) void FakeBCryptHashData() { _asm { jmp[bcrypt.BCryptHashData] } }
__declspec(naked) void FakeBCryptImportKey() { _asm { jmp[bcrypt.BCryptImportKey] } }
__declspec(naked) void FakeBCryptImportKeyPair() { _asm { jmp[bcrypt.BCryptImportKeyPair] } }
__declspec(naked) void FakeBCryptKeyDerivation() { _asm { jmp[bcrypt.BCryptKeyDerivation] } }
__declspec(naked) void FakeBCryptOpenAlgorithmProvider() { _asm { jmp[bcrypt.BCryptOpenAlgorithmProvider] } }
__declspec(naked) void FakeBCryptProcessMultiOperations() { _asm { jmp[bcrypt.BCryptProcessMultiOperations] } }
__declspec(naked) void FakeBCryptQueryContextConfiguration() { _asm { jmp[bcrypt.BCryptQueryContextConfiguration] } }
__declspec(naked) void FakeBCryptQueryContextFunctionConfiguration() { _asm { jmp[bcrypt.BCryptQueryContextFunctionConfiguration] } }
__declspec(naked) void FakeBCryptQueryContextFunctionProperty() { _asm { jmp[bcrypt.BCryptQueryContextFunctionProperty] } }
__declspec(naked) void FakeBCryptQueryProviderRegistration() { _asm { jmp[bcrypt.BCryptQueryProviderRegistration] } }
__declspec(naked) void FakeBCryptRegisterConfigChangeNotify() { _asm { jmp[bcrypt.BCryptRegisterConfigChangeNotify] } }
__declspec(naked) void FakeBCryptRegisterProvider() { _asm { jmp[bcrypt.BCryptRegisterProvider] } }
__declspec(naked) void FakeBCryptRemoveContextFunction() { _asm { jmp[bcrypt.BCryptRemoveContextFunction] } }
__declspec(naked) void FakeBCryptRemoveContextFunctionProvider() { _asm { jmp[bcrypt.BCryptRemoveContextFunctionProvider] } }
__declspec(naked) void FakeBCryptResolveProviders() { _asm { jmp[bcrypt.BCryptResolveProviders] } }
__declspec(naked) void FakeBCryptSecretAgreement() { _asm { jmp[bcrypt.BCryptSecretAgreement] } }
__declspec(naked) void FakeBCryptSetAuditingInterface() { _asm { jmp[bcrypt.BCryptSetAuditingInterface] } }
__declspec(naked) void FakeBCryptSetContextFunctionProperty() { _asm { jmp[bcrypt.BCryptSetContextFunctionProperty] } }
__declspec(naked) void FakeBCryptSetProperty() { _asm { jmp[bcrypt.BCryptSetProperty] } }
__declspec(naked) void FakeBCryptSignHash() { _asm { jmp[bcrypt.BCryptSignHash] } }
__declspec(naked) void FakeBCryptUnregisterConfigChangeNotify() { _asm { jmp[bcrypt.BCryptUnregisterConfigChangeNotify] } }
__declspec(naked) void FakeBCryptUnregisterProvider() { _asm { jmp[bcrypt.BCryptUnregisterProvider] } }
__declspec(naked) void FakeBCryptVerifySignature() { _asm { jmp[bcrypt.BCryptVerifySignature] } }

void LoadBcrypt()
{
	// Load real dll
	bcrypt.dll = LoadDll(dtype.bcrypt);

	// Load dll functions
	if (bcrypt.dll)
	{
		bcrypt.BCryptAddContextFunction = GetFunctionAddress(bcrypt.dll, "BCryptAddContextFunction", jmpaddr);
		bcrypt.BCryptAddContextFunctionProvider = GetFunctionAddress(bcrypt.dll, "BCryptAddContextFunctionProvider", jmpaddr);
		bcrypt.BCryptCloseAlgorithmProvider = GetFunctionAddress(bcrypt.dll, "BCryptCloseAlgorithmProvider", jmpaddr);
		bcrypt.BCryptConfigureContext = GetFunctionAddress(bcrypt.dll, "BCryptConfigureContext", jmpaddr);
		bcrypt.BCryptConfigureContextFunction = GetFunctionAddress(bcrypt.dll, "BCryptConfigureContextFunction", jmpaddr);
		bcrypt.BCryptCreateContext = GetFunctionAddress(bcrypt.dll, "BCryptCreateContext", jmpaddr);
		bcrypt.BCryptCreateHash = GetFunctionAddress(bcrypt.dll, "BCryptCreateHash", jmpaddr);
		bcrypt.BCryptCreateMultiHash = GetFunctionAddress(bcrypt.dll, "BCryptCreateMultiHash", jmpaddr);
		bcrypt.BCryptDecrypt = GetFunctionAddress(bcrypt.dll, "BCryptDecrypt", jmpaddr);
		bcrypt.BCryptDeleteContext = GetFunctionAddress(bcrypt.dll, "BCryptDeleteContext", jmpaddr);
		bcrypt.BCryptDeriveKey = GetFunctionAddress(bcrypt.dll, "BCryptDeriveKey", jmpaddr);
		bcrypt.BCryptDeriveKeyCapi = GetFunctionAddress(bcrypt.dll, "BCryptDeriveKeyCapi", jmpaddr);
		bcrypt.BCryptDeriveKeyPBKDF2 = GetFunctionAddress(bcrypt.dll, "BCryptDeriveKeyPBKDF2", jmpaddr);
		bcrypt.BCryptDestroyHash = GetFunctionAddress(bcrypt.dll, "BCryptDestroyHash", jmpaddr);
		bcrypt.BCryptDestroyKey = GetFunctionAddress(bcrypt.dll, "BCryptDestroyKey", jmpaddr);
		bcrypt.BCryptDestroySecret = GetFunctionAddress(bcrypt.dll, "BCryptDestroySecret", jmpaddr);
		bcrypt.BCryptDuplicateHash = GetFunctionAddress(bcrypt.dll, "BCryptDuplicateHash", jmpaddr);
		bcrypt.BCryptDuplicateKey = GetFunctionAddress(bcrypt.dll, "BCryptDuplicateKey", jmpaddr);
		bcrypt.BCryptEncrypt = GetFunctionAddress(bcrypt.dll, "BCryptEncrypt", jmpaddr);
		bcrypt.BCryptEnumAlgorithms = GetFunctionAddress(bcrypt.dll, "BCryptEnumAlgorithms", jmpaddr);
		bcrypt.BCryptEnumContextFunctionProviders = GetFunctionAddress(bcrypt.dll, "BCryptEnumContextFunctionProviders", jmpaddr);
		bcrypt.BCryptEnumContextFunctions = GetFunctionAddress(bcrypt.dll, "BCryptEnumContextFunctions", jmpaddr);
		bcrypt.BCryptEnumContexts = GetFunctionAddress(bcrypt.dll, "BCryptEnumContexts", jmpaddr);
		bcrypt.BCryptEnumProviders = GetFunctionAddress(bcrypt.dll, "BCryptEnumProviders", jmpaddr);
		bcrypt.BCryptEnumRegisteredProviders = GetFunctionAddress(bcrypt.dll, "BCryptEnumRegisteredProviders", jmpaddr);
		bcrypt.BCryptExportKey = GetFunctionAddress(bcrypt.dll, "BCryptExportKey", jmpaddr);
		bcrypt.BCryptFinalizeKeyPair = GetFunctionAddress(bcrypt.dll, "BCryptFinalizeKeyPair", jmpaddr);
		bcrypt.BCryptFinishHash = GetFunctionAddress(bcrypt.dll, "BCryptFinishHash", jmpaddr);
		bcrypt.BCryptFreeBuffer = GetFunctionAddress(bcrypt.dll, "BCryptFreeBuffer", jmpaddr);
		bcrypt.BCryptGenRandom = GetFunctionAddress(bcrypt.dll, "BCryptGenRandom", jmpaddr);
		bcrypt.BCryptGenerateKeyPair = GetFunctionAddress(bcrypt.dll, "BCryptGenerateKeyPair", jmpaddr);
		bcrypt.BCryptGenerateSymmetricKey = GetFunctionAddress(bcrypt.dll, "BCryptGenerateSymmetricKey", jmpaddr);
		bcrypt.BCryptGetFipsAlgorithmMode = GetFunctionAddress(bcrypt.dll, "BCryptGetFipsAlgorithmMode", jmpaddr);
		bcrypt.BCryptGetProperty = GetFunctionAddress(bcrypt.dll, "BCryptGetProperty", jmpaddr);
		bcrypt.BCryptHash = GetFunctionAddress(bcrypt.dll, "BCryptHash", jmpaddr);
		bcrypt.BCryptHashData = GetFunctionAddress(bcrypt.dll, "BCryptHashData", jmpaddr);
		bcrypt.BCryptImportKey = GetFunctionAddress(bcrypt.dll, "BCryptImportKey", jmpaddr);
		bcrypt.BCryptImportKeyPair = GetFunctionAddress(bcrypt.dll, "BCryptImportKeyPair", jmpaddr);
		bcrypt.BCryptKeyDerivation = GetFunctionAddress(bcrypt.dll, "BCryptKeyDerivation", jmpaddr);
		bcrypt.BCryptOpenAlgorithmProvider = GetFunctionAddress(bcrypt.dll, "BCryptOpenAlgorithmProvider", jmpaddr);
		bcrypt.BCryptProcessMultiOperations = GetFunctionAddress(bcrypt.dll, "BCryptProcessMultiOperations", jmpaddr);
		bcrypt.BCryptQueryContextConfiguration = GetFunctionAddress(bcrypt.dll, "BCryptQueryContextConfiguration", jmpaddr);
		bcrypt.BCryptQueryContextFunctionConfiguration = GetFunctionAddress(bcrypt.dll, "BCryptQueryContextFunctionConfiguration", jmpaddr);
		bcrypt.BCryptQueryContextFunctionProperty = GetFunctionAddress(bcrypt.dll, "BCryptQueryContextFunctionProperty", jmpaddr);
		bcrypt.BCryptQueryProviderRegistration = GetFunctionAddress(bcrypt.dll, "BCryptQueryProviderRegistration", jmpaddr);
		bcrypt.BCryptRegisterConfigChangeNotify = GetFunctionAddress(bcrypt.dll, "BCryptRegisterConfigChangeNotify", jmpaddr);
		bcrypt.BCryptRegisterProvider = GetFunctionAddress(bcrypt.dll, "BCryptRegisterProvider", jmpaddr);
		bcrypt.BCryptRemoveContextFunction = GetFunctionAddress(bcrypt.dll, "BCryptRemoveContextFunction", jmpaddr);
		bcrypt.BCryptRemoveContextFunctionProvider = GetFunctionAddress(bcrypt.dll, "BCryptRemoveContextFunctionProvider", jmpaddr);
		bcrypt.BCryptResolveProviders = GetFunctionAddress(bcrypt.dll, "BCryptResolveProviders", jmpaddr);
		bcrypt.BCryptSecretAgreement = GetFunctionAddress(bcrypt.dll, "BCryptSecretAgreement", jmpaddr);
		bcrypt.BCryptSetAuditingInterface = GetFunctionAddress(bcrypt.dll, "BCryptSetAuditingInterface", jmpaddr);
		bcrypt.BCryptSetContextFunctionProperty = GetFunctionAddress(bcrypt.dll, "BCryptSetContextFunctionProperty", jmpaddr);
		bcrypt.BCryptSetProperty = GetFunctionAddress(bcrypt.dll, "BCryptSetProperty", jmpaddr);
		bcrypt.BCryptSignHash = GetFunctionAddress(bcrypt.dll, "BCryptSignHash", jmpaddr);
		bcrypt.BCryptUnregisterConfigChangeNotify = GetFunctionAddress(bcrypt.dll, "BCryptUnregisterConfigChangeNotify", jmpaddr);
		bcrypt.BCryptUnregisterProvider = GetFunctionAddress(bcrypt.dll, "BCryptUnregisterProvider", jmpaddr);
		bcrypt.BCryptVerifySignature = GetFunctionAddress(bcrypt.dll, "BCryptVerifySignature", jmpaddr);
	}
}