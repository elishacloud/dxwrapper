#pragma once

#define VISIT_PROCS_CRYPTSP(visit) \
	visit(CheckSignatureInFile, jmpaddr) \
	visit(CryptAcquireContextA, jmpaddr) \
	visit(CryptAcquireContextW, jmpaddr) \
	visit(CryptContextAddRef, jmpaddr) \
	visit(CryptCreateHash, jmpaddr) \
	visit(CryptDecrypt, jmpaddr) \
	visit(CryptDeriveKey, jmpaddr) \
	visit(CryptDestroyHash, jmpaddr) \
	visit(CryptDestroyKey, jmpaddr) \
	visit(CryptDuplicateHash, jmpaddr) \
	visit(CryptDuplicateKey, jmpaddr) \
	visit(CryptEncrypt, jmpaddr) \
	visit(CryptEnumProviderTypesA, jmpaddr) \
	visit(CryptEnumProviderTypesW, jmpaddr) \
	visit(CryptEnumProvidersA, jmpaddr) \
	visit(CryptEnumProvidersW, jmpaddr) \
	visit(CryptExportKey, jmpaddr) \
	visit(CryptGenKey, jmpaddr) \
	visit(CryptGenRandom, jmpaddr) \
	visit(CryptGetDefaultProviderA, jmpaddr) \
	visit(CryptGetDefaultProviderW, jmpaddr) \
	visit(CryptGetHashParam, jmpaddr) \
	visit(CryptGetKeyParam, jmpaddr) \
	visit(CryptGetProvParam, jmpaddr) \
	visit(CryptGetUserKey, jmpaddr) \
	visit(CryptHashData, jmpaddr) \
	visit(CryptHashSessionKey, jmpaddr) \
	visit(CryptImportKey, jmpaddr) \
	visit(CryptReleaseContext, jmpaddr) \
	visit(CryptSetHashParam, jmpaddr) \
	visit(CryptSetKeyParam, jmpaddr) \
	visit(CryptSetProvParam, jmpaddr) \
	visit(CryptSetProviderA, jmpaddr) \
	visit(CryptSetProviderExA, jmpaddr) \
	visit(CryptSetProviderExW, jmpaddr) \
	visit(CryptSetProviderW, jmpaddr) \
	visit(CryptSignHashA, jmpaddr) \
	visit(CryptSignHashW, jmpaddr) \
	visit(CryptVerifySignatureA, jmpaddr) \
	visit(CryptVerifySignatureW, jmpaddr) \
	visit(SystemFunction006, jmpaddr) \
	visit(SystemFunction007, jmpaddr) \
	visit(SystemFunction008, jmpaddr) \
	visit(SystemFunction009, jmpaddr) \
	visit(SystemFunction010, jmpaddr) \
	visit(SystemFunction011, jmpaddr) \
	visit(SystemFunction012, jmpaddr) \
	visit(SystemFunction013, jmpaddr) \
	visit(SystemFunction014, jmpaddr) \
	visit(SystemFunction015, jmpaddr) \
	visit(SystemFunction016, jmpaddr) \
	visit(SystemFunction018, jmpaddr) \
	visit(SystemFunction020, jmpaddr) \
	visit(SystemFunction021, jmpaddr) \
	visit(SystemFunction022, jmpaddr) \
	visit(SystemFunction023, jmpaddr) \
	visit(SystemFunction024, jmpaddr) \
	visit(SystemFunction025, jmpaddr) \
	visit(SystemFunction026, jmpaddr) \
	visit(SystemFunction027, jmpaddr) \
	visit(SystemFunction030, jmpaddr) \
	visit(SystemFunction031, jmpaddr) \
	visit(SystemFunction032, jmpaddr) \
	visit(SystemFunction033, jmpaddr) \
	visit(SystemFunction035, jmpaddr)

#ifdef PROC_CLASS
PROC_CLASS(cryptsp, dll, VISIT_PROCS_CRYPTSP, VISIT_PROCS_BLANK)
#endif
