#pragma once

#define VISIT_BCRYPT_PROCS(visit) \
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

namespace bcrypt
{
	class bcrypt_dll
	{
	public:
		HMODULE dll = nullptr;
		VISIT_BCRYPT_PROCS(ADD_FARPROC_MEMBER);

		void Load()
		{
			if (Config.WrapperMode != dtype.bcrypt && Config.WrapperMode != dtype.Auto)
			{
				return;
			}
			
			// Load real dll
			dll = LoadDll(dtype.bcrypt);

			// Load dll functions
			if (dll)
			{
				VISIT_BCRYPT_PROCS(LOAD_ORIGINAL_PROC);
			}
		}

		void Unhook() {}
	};

	extern bcrypt_dll module;
};
