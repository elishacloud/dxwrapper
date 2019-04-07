#pragma once

#define VISIT_PROCS_BCRYPT(visit) \
	visit(BCryptAddContextFunction, jmpaddr) \
	visit(BCryptAddContextFunctionProvider, jmpaddr) \
	visit(BCryptCloseAlgorithmProvider, jmpaddr) \
	visit(BCryptConfigureContext, jmpaddr) \
	visit(BCryptConfigureContextFunction, jmpaddr) \
	visit(BCryptCreateContext, jmpaddr) \
	visit(BCryptCreateHash, jmpaddr) \
	visit(BCryptCreateMultiHash, jmpaddr) \
	visit(BCryptDecrypt, jmpaddr) \
	visit(BCryptDeleteContext, jmpaddr) \
	visit(BCryptDeriveKey, jmpaddr) \
	visit(BCryptDeriveKeyCapi, jmpaddr) \
	visit(BCryptDeriveKeyPBKDF2, jmpaddr) \
	visit(BCryptDestroyHash, jmpaddr) \
	visit(BCryptDestroyKey, jmpaddr) \
	visit(BCryptDestroySecret, jmpaddr) \
	visit(BCryptDuplicateHash, jmpaddr) \
	visit(BCryptDuplicateKey, jmpaddr) \
	visit(BCryptEncrypt, jmpaddr) \
	visit(BCryptEnumAlgorithms, jmpaddr) \
	visit(BCryptEnumContextFunctionProviders, jmpaddr) \
	visit(BCryptEnumContextFunctions, jmpaddr) \
	visit(BCryptEnumContexts, jmpaddr) \
	visit(BCryptEnumProviders, jmpaddr) \
	visit(BCryptEnumRegisteredProviders, jmpaddr) \
	visit(BCryptExportKey, jmpaddr) \
	visit(BCryptFinalizeKeyPair, jmpaddr) \
	visit(BCryptFinishHash, jmpaddr) \
	visit(BCryptFreeBuffer, jmpaddr) \
	visit(BCryptGenRandom, jmpaddr) \
	visit(BCryptGenerateKeyPair, jmpaddr) \
	visit(BCryptGenerateSymmetricKey, jmpaddr) \
	visit(BCryptGetFipsAlgorithmMode, jmpaddr) \
	visit(BCryptGetProperty, jmpaddr) \
	visit(BCryptHash, jmpaddr) \
	visit(BCryptHashData, jmpaddr) \
	visit(BCryptImportKey, jmpaddr) \
	visit(BCryptImportKeyPair, jmpaddr) \
	visit(BCryptKeyDerivation, jmpaddr) \
	visit(BCryptOpenAlgorithmProvider, jmpaddr) \
	visit(BCryptProcessMultiOperations, jmpaddr) \
	visit(BCryptQueryContextConfiguration, jmpaddr) \
	visit(BCryptQueryContextFunctionConfiguration, jmpaddr) \
	visit(BCryptQueryContextFunctionProperty, jmpaddr) \
	visit(BCryptQueryProviderRegistration, jmpaddr) \
	visit(BCryptRegisterConfigChangeNotify, jmpaddr) \
	visit(BCryptRegisterProvider, jmpaddr) \
	visit(BCryptRemoveContextFunction, jmpaddr) \
	visit(BCryptRemoveContextFunctionProvider, jmpaddr) \
	visit(BCryptResolveProviders, jmpaddr) \
	visit(BCryptSecretAgreement, jmpaddr) \
	visit(BCryptSetAuditingInterface, jmpaddr) \
	visit(BCryptSetContextFunctionProperty, jmpaddr) \
	visit(BCryptSetProperty, jmpaddr) \
	visit(BCryptSignHash, jmpaddr) \
	visit(BCryptUnregisterConfigChangeNotify, jmpaddr) \
	visit(BCryptUnregisterProvider, jmpaddr) \
	visit(BCryptVerifySignature, jmpaddr)

#ifdef PROC_CLASS
PROC_CLASS(bcrypt, dll, VISIT_PROCS_BCRYPT, VISIT_PROCS_BLANK)
#endif
