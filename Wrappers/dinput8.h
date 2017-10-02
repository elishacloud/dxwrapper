#pragma once

#define VISIT_PROCS(visit) \
	visit(DirectInput8Create) \
	visit(GetdfDIJoystick)

PROC_CLASS(dinput8, dll)

#undef VISIT_PROCS
