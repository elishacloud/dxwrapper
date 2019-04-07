#pragma once

#define VISIT_PROCS_DPLAYX(visit) \
	visit(DirectPlayCreate, jmpaddr) \
	visit(DirectPlayEnumerate, jmpaddr) \
	visit(DirectPlayEnumerateA, jmpaddr) \
	visit(DirectPlayEnumerateW, jmpaddr) \
	visit(DirectPlayLobbyCreateA, jmpaddr) \
	visit(DirectPlayLobbyCreateW, jmpaddr)

#ifdef PROC_CLASS
PROC_CLASS(dplayx, dll, VISIT_PROCS_DPLAYX, VISIT_PROCS_BLANK)
#endif
