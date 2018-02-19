#pragma once

#define VISIT_PROCS_DINPUT8(visit) \
	visit(DirectInput8Create, jmpaddr) \
	visit(GetdfDIJoystick, jmpaddr)

#ifdef PROC_CLASS
PROC_CLASS(dinput8, dll, VISIT_PROCS_DINPUT8)
#endif
