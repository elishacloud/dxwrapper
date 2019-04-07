#pragma once

#define VISIT_PROCS_DINPUT(visit) \
	visit(DirectInputCreateA, jmpaddr) \
	visit(DirectInputCreateEx, jmpaddr) \
	visit(DirectInputCreateW, jmpaddr)

#define VISIT_PROCS_DINPUT_SHARED(visit) \
	visit(DllCanUnloadNow, jmpaddr) \
	visit(DllGetClassObject, jmpaddr) \
	visit(DllRegisterServer, jmpaddr) \
	visit(DllUnregisterServer, jmpaddr)

#define VISIT_SHARED_DINPUT_PROCS(visit) \
	visit(DllCanUnloadNow, DllCanUnloadNow_dinput, jmpaddr) \
	visit(DllGetClassObject, DllGetClassObject_dinput, jmpaddr) \
	visit(DllRegisterServer, DllRegisterServer_dinput, jmpaddr) \
	visit(DllUnregisterServer, DllUnregisterServer_dinput, jmpaddr)

#ifdef PROC_CLASS
PROC_CLASS(dinput, dll, VISIT_PROCS_DINPUT, VISIT_SHARED_DINPUT_PROCS)
#endif
