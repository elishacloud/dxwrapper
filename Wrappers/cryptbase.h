#pragma once

#define VISIT_PROCS_CRYPTBASE(visit) \
	visit(SystemFunction001, jmpaddr) \
	visit(SystemFunction002, jmpaddr) \
	visit(SystemFunction003, jmpaddr) \
	visit(SystemFunction004, jmpaddr) \
	visit(SystemFunction005, jmpaddr) \
	visit(SystemFunction028, jmpaddr) \
	visit(SystemFunction029, jmpaddr) \
	visit(SystemFunction034, jmpaddr) \
	visit(SystemFunction036, jmpaddr) \
	visit(SystemFunction040, jmpaddr) \
	visit(SystemFunction041, jmpaddr)

#ifdef PROC_CLASS
PROC_CLASS(cryptbase, dll, VISIT_PROCS_CRYPTBASE, VISIT_PROCS_BLANK, VISIT_PROCS_BLANK)
#endif
