#pragma once

#define VISIT_PROCS(visit) \
	visit(DirectInputCreateA) \
	visit(DirectInputCreateEx) \
	visit(DirectInputCreateW)

PROC_CLASS(dinput, dll)

#undef VISIT_PROCS
