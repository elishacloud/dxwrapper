#pragma once

#define VISIT_DPLAYX_PROCS(visit) \
	visit(DirectPlayCreate) \
	visit(DirectPlayEnumerate) \
	visit(DirectPlayEnumerateA) \
	visit(DirectPlayEnumerateW) \
	visit(DirectPlayLobbyCreateA) \
	visit(DirectPlayLobbyCreateW)

class dplayx_dll
{
public:
	dplayx_dll() { };
	~dplayx_dll() { };

	void Load();

	HMODULE dll = nullptr;

private:
	VISIT_DPLAYX_PROCS(ADD_FARPROC_MEMBER);
};

extern dplayx_dll dplayx;
