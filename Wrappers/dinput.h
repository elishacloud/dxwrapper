#pragma once

#define VISIT_PROCS_DINPUT(visit) \
	visit(DirectInputCreateA, jmpaddr) \
	visit(DirectInputCreateEx, jmpaddr) \
	visit(DirectInputCreateW, jmpaddr)

#ifdef PROC_CLASS
PROC_CLASS(dinput, dll, VISIT_PROCS_DINPUT)
#endif
