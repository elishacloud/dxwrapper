#pragma once

#define VISIT_CRYTPSP_PROCS(visit) \
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

namespace cryptsp
{
	class cryptsp_dll
	{
	public:
		HMODULE dll = nullptr;
		VISIT_CRYTPSP_PROCS(ADD_FARPROC_MEMBER);

		void Load()
		{
			if (Config.WrapperMode != dtype.cryptsp && Config.WrapperMode != dtype.Auto)
			{
				return;
			}

			// Load real dll
			dll = Wrapper::LoadDll(dtype.cryptsp);

			// Load dll functions
			if (dll)
			{
				VISIT_CRYTPSP_PROCS(LOAD_ORIGINAL_PROC);
			}
		}

		void Unhook() {}
	};

	extern cryptsp_dll module;
};
