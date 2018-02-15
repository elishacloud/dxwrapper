#pragma once

#define VISIT_PROCS(visit) \
	visit(DirectPlayCreate, jmpaddr) \
	visit(DirectPlayEnumerate, jmpaddr) \
	visit(DirectPlayEnumerateA, jmpaddr) \
	visit(DirectPlayEnumerateW, jmpaddr) \
	visit(DirectPlayLobbyCreateA, jmpaddr) \
	visit(DirectPlayLobbyCreateW, jmpaddr)

PROC_CLASS(dplayx, dll)

#undef VISIT_PROCS
