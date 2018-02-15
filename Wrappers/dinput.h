#pragma once

#define VISIT_PROCS(visit) \
	visit(DirectInputCreateA, jmpaddr) \
	visit(DirectInputCreateEx, jmpaddr) \
	visit(DirectInputCreateW, jmpaddr)

PROC_CLASS(dinput, dll)

#undef VISIT_PROCS
