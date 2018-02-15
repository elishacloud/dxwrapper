#pragma once

#define VISIT_PROCS(visit) \
	visit(DirectInput8Create, jmpaddr) \
	visit(GetdfDIJoystick, jmpaddr)

PROC_CLASS(dinput8, dll)

#undef VISIT_PROCS
