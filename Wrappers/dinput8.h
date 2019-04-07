#pragma once

#define VISIT_PROCS_DINPUT8(visit) \
	visit(DirectInput8Create, jmpaddr) \
	visit(GetdfDIJoystick, jmpaddr)

#define VISIT_PROCS_DINPUT8_SHARED(visit) \
	visit(DllCanUnloadNow, jmpaddr) \
	visit(DllGetClassObject, jmpaddr) \
	visit(DllRegisterServer, jmpaddr) \
	visit(DllUnregisterServer, jmpaddr)

#define VISIT_SHARED_DINPUT8_PROCS(visit) \
	visit(DllCanUnloadNow, DllCanUnloadNow_dinput8, jmpaddr) \
	visit(DllGetClassObject, DllGetClassObject_dinput8, jmpaddr) \
	visit(DllRegisterServer, DllRegisterServer_dinput8, jmpaddr) \
	visit(DllUnregisterServer, DllUnregisterServer_dinput8, jmpaddr)

#ifdef PROC_CLASS
PROC_CLASS(dinput8, dll, VISIT_PROCS_DINPUT8, VISIT_SHARED_DINPUT8_PROCS)
#endif
