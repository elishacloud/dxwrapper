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

struct bcrypt_dll
{
	HMODULE dll = nullptr;
	FARPROC BCryptAddContextFunction;
	FARPROC BCryptAddContextFunctionProvider;
	FARPROC BCryptCloseAlgorithmProvider;
	FARPROC BCryptConfigureContext;
	FARPROC BCryptConfigureContextFunction;
	FARPROC BCryptCreateContext;
	FARPROC BCryptCreateHash;
	FARPROC BCryptCreateMultiHash;
	FARPROC BCryptDecrypt;
	FARPROC BCryptDeleteContext;
	FARPROC BCryptDeriveKey;
	FARPROC BCryptDeriveKeyCapi;
	FARPROC BCryptDeriveKeyPBKDF2;
	FARPROC BCryptDestroyHash;
	FARPROC BCryptDestroyKey;
	FARPROC BCryptDestroySecret;
	FARPROC BCryptDuplicateHash;
	FARPROC BCryptDuplicateKey;
	FARPROC BCryptEncrypt;
	FARPROC BCryptEnumAlgorithms;
	FARPROC BCryptEnumContextFunctionProviders;
	FARPROC BCryptEnumContextFunctions;
	FARPROC BCryptEnumContexts;
	FARPROC BCryptEnumProviders;
	FARPROC BCryptEnumRegisteredProviders;
	FARPROC BCryptExportKey;
	FARPROC BCryptFinalizeKeyPair;
	FARPROC BCryptFinishHash;
	FARPROC BCryptFreeBuffer;
	FARPROC BCryptGenRandom;
	FARPROC BCryptGenerateKeyPair;
	FARPROC BCryptGenerateSymmetricKey;
	FARPROC BCryptGetFipsAlgorithmMode;
	FARPROC BCryptGetProperty;
	FARPROC BCryptHash;
	FARPROC BCryptHashData;
	FARPROC BCryptImportKey;
	FARPROC BCryptImportKeyPair;
	FARPROC BCryptKeyDerivation;
	FARPROC BCryptOpenAlgorithmProvider;
	FARPROC BCryptProcessMultiOperations;
	FARPROC BCryptQueryContextConfiguration;
	FARPROC BCryptQueryContextFunctionConfiguration;
	FARPROC BCryptQueryContextFunctionProperty;
	FARPROC BCryptQueryProviderRegistration;
	FARPROC BCryptRegisterConfigChangeNotify;
	FARPROC BCryptRegisterProvider;
	FARPROC BCryptRemoveContextFunction;
	FARPROC BCryptRemoveContextFunctionProvider;
	FARPROC BCryptResolveProviders;
	FARPROC BCryptSecretAgreement;
	FARPROC BCryptSetAuditingInterface;
	FARPROC BCryptSetContextFunctionProperty;
	FARPROC BCryptSetProperty;
	FARPROC BCryptSignHash;
	FARPROC BCryptUnregisterConfigChangeNotify;
	FARPROC BCryptUnregisterProvider;
	FARPROC BCryptVerifySignature;
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
		bcrypt.BCryptAddContextFunction = GetProcAddress(bcrypt.dll, "BCryptAddContextFunction");
		bcrypt.BCryptAddContextFunctionProvider = GetProcAddress(bcrypt.dll, "BCryptAddContextFunctionProvider");
		bcrypt.BCryptCloseAlgorithmProvider = GetProcAddress(bcrypt.dll, "BCryptCloseAlgorithmProvider");
		bcrypt.BCryptConfigureContext = GetProcAddress(bcrypt.dll, "BCryptConfigureContext");
		bcrypt.BCryptConfigureContextFunction = GetProcAddress(bcrypt.dll, "BCryptConfigureContextFunction");
		bcrypt.BCryptCreateContext = GetProcAddress(bcrypt.dll, "BCryptCreateContext");
		bcrypt.BCryptCreateHash = GetProcAddress(bcrypt.dll, "BCryptCreateHash");
		bcrypt.BCryptCreateMultiHash = GetProcAddress(bcrypt.dll, "BCryptCreateMultiHash");
		bcrypt.BCryptDecrypt = GetProcAddress(bcrypt.dll, "BCryptDecrypt");
		bcrypt.BCryptDeleteContext = GetProcAddress(bcrypt.dll, "BCryptDeleteContext");
		bcrypt.BCryptDeriveKey = GetProcAddress(bcrypt.dll, "BCryptDeriveKey");
		bcrypt.BCryptDeriveKeyCapi = GetProcAddress(bcrypt.dll, "BCryptDeriveKeyCapi");
		bcrypt.BCryptDeriveKeyPBKDF2 = GetProcAddress(bcrypt.dll, "BCryptDeriveKeyPBKDF2");
		bcrypt.BCryptDestroyHash = GetProcAddress(bcrypt.dll, "BCryptDestroyHash");
		bcrypt.BCryptDestroyKey = GetProcAddress(bcrypt.dll, "BCryptDestroyKey");
		bcrypt.BCryptDestroySecret = GetProcAddress(bcrypt.dll, "BCryptDestroySecret");
		bcrypt.BCryptDuplicateHash = GetProcAddress(bcrypt.dll, "BCryptDuplicateHash");
		bcrypt.BCryptDuplicateKey = GetProcAddress(bcrypt.dll, "BCryptDuplicateKey");
		bcrypt.BCryptEncrypt = GetProcAddress(bcrypt.dll, "BCryptEncrypt");
		bcrypt.BCryptEnumAlgorithms = GetProcAddress(bcrypt.dll, "BCryptEnumAlgorithms");
		bcrypt.BCryptEnumContextFunctionProviders = GetProcAddress(bcrypt.dll, "BCryptEnumContextFunctionProviders");
		bcrypt.BCryptEnumContextFunctions = GetProcAddress(bcrypt.dll, "BCryptEnumContextFunctions");
		bcrypt.BCryptEnumContexts = GetProcAddress(bcrypt.dll, "BCryptEnumContexts");
		bcrypt.BCryptEnumProviders = GetProcAddress(bcrypt.dll, "BCryptEnumProviders");
		bcrypt.BCryptEnumRegisteredProviders = GetProcAddress(bcrypt.dll, "BCryptEnumRegisteredProviders");
		bcrypt.BCryptExportKey = GetProcAddress(bcrypt.dll, "BCryptExportKey");
		bcrypt.BCryptFinalizeKeyPair = GetProcAddress(bcrypt.dll, "BCryptFinalizeKeyPair");
		bcrypt.BCryptFinishHash = GetProcAddress(bcrypt.dll, "BCryptFinishHash");
		bcrypt.BCryptFreeBuffer = GetProcAddress(bcrypt.dll, "BCryptFreeBuffer");
		bcrypt.BCryptGenRandom = GetProcAddress(bcrypt.dll, "BCryptGenRandom");
		bcrypt.BCryptGenerateKeyPair = GetProcAddress(bcrypt.dll, "BCryptGenerateKeyPair");
		bcrypt.BCryptGenerateSymmetricKey = GetProcAddress(bcrypt.dll, "BCryptGenerateSymmetricKey");
		bcrypt.BCryptGetFipsAlgorithmMode = GetProcAddress(bcrypt.dll, "BCryptGetFipsAlgorithmMode");
		bcrypt.BCryptGetProperty = GetProcAddress(bcrypt.dll, "BCryptGetProperty");
		bcrypt.BCryptHash = GetProcAddress(bcrypt.dll, "BCryptHash");
		bcrypt.BCryptHashData = GetProcAddress(bcrypt.dll, "BCryptHashData");
		bcrypt.BCryptImportKey = GetProcAddress(bcrypt.dll, "BCryptImportKey");
		bcrypt.BCryptImportKeyPair = GetProcAddress(bcrypt.dll, "BCryptImportKeyPair");
		bcrypt.BCryptKeyDerivation = GetProcAddress(bcrypt.dll, "BCryptKeyDerivation");
		bcrypt.BCryptOpenAlgorithmProvider = GetProcAddress(bcrypt.dll, "BCryptOpenAlgorithmProvider");
		bcrypt.BCryptProcessMultiOperations = GetProcAddress(bcrypt.dll, "BCryptProcessMultiOperations");
		bcrypt.BCryptQueryContextConfiguration = GetProcAddress(bcrypt.dll, "BCryptQueryContextConfiguration");
		bcrypt.BCryptQueryContextFunctionConfiguration = GetProcAddress(bcrypt.dll, "BCryptQueryContextFunctionConfiguration");
		bcrypt.BCryptQueryContextFunctionProperty = GetProcAddress(bcrypt.dll, "BCryptQueryContextFunctionProperty");
		bcrypt.BCryptQueryProviderRegistration = GetProcAddress(bcrypt.dll, "BCryptQueryProviderRegistration");
		bcrypt.BCryptRegisterConfigChangeNotify = GetProcAddress(bcrypt.dll, "BCryptRegisterConfigChangeNotify");
		bcrypt.BCryptRegisterProvider = GetProcAddress(bcrypt.dll, "BCryptRegisterProvider");
		bcrypt.BCryptRemoveContextFunction = GetProcAddress(bcrypt.dll, "BCryptRemoveContextFunction");
		bcrypt.BCryptRemoveContextFunctionProvider = GetProcAddress(bcrypt.dll, "BCryptRemoveContextFunctionProvider");
		bcrypt.BCryptResolveProviders = GetProcAddress(bcrypt.dll, "BCryptResolveProviders");
		bcrypt.BCryptSecretAgreement = GetProcAddress(bcrypt.dll, "BCryptSecretAgreement");
		bcrypt.BCryptSetAuditingInterface = GetProcAddress(bcrypt.dll, "BCryptSetAuditingInterface");
		bcrypt.BCryptSetContextFunctionProperty = GetProcAddress(bcrypt.dll, "BCryptSetContextFunctionProperty");
		bcrypt.BCryptSetProperty = GetProcAddress(bcrypt.dll, "BCryptSetProperty");
		bcrypt.BCryptSignHash = GetProcAddress(bcrypt.dll, "BCryptSignHash");
		bcrypt.BCryptUnregisterConfigChangeNotify = GetProcAddress(bcrypt.dll, "BCryptUnregisterConfigChangeNotify");
		bcrypt.BCryptUnregisterProvider = GetProcAddress(bcrypt.dll, "BCryptUnregisterProvider");
		bcrypt.BCryptVerifySignature = GetProcAddress(bcrypt.dll, "BCryptVerifySignature");
	}
}