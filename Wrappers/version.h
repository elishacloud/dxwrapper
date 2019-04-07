#pragma once

#define VISIT_PROCS_VERSION(visit) \
	visit(GetFileVersionInfoA, jmpaddr) \
	visit(GetFileVersionInfoByHandle, jmpaddr) \
	visit(GetFileVersionInfoExA, jmpaddr) \
	visit(GetFileVersionInfoExW, jmpaddr) \
	visit(GetFileVersionInfoSizeA, jmpaddr) \
	visit(GetFileVersionInfoSizeExA, jmpaddr) \
	visit(GetFileVersionInfoSizeExW, jmpaddr) \
	visit(GetFileVersionInfoSizeW, jmpaddr) \
	visit(GetFileVersionInfoW, jmpaddr) \
	visit(VerFindFileA, jmpaddr) \
	visit(VerFindFileW, jmpaddr) \
	visit(VerInstallFileA, jmpaddr) \
	visit(VerInstallFileW, jmpaddr) \
	visit(VerLanguageNameA, jmpaddr) \
	visit(VerLanguageNameW, jmpaddr) \
	visit(VerQueryValueA, jmpaddr) \
	visit(VerQueryValueW, jmpaddr)

#ifdef PROC_CLASS
PROC_CLASS(version, dll, VISIT_PROCS_VERSION, VISIT_PROCS_BLANK)
#endif
