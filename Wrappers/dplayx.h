#pragma once

#define VISIT_DPLAYX_PROCS(visit) \
	visit(DirectPlayCreate) \
	visit(DirectPlayEnumerate) \
	visit(DirectPlayEnumerateA) \
	visit(DirectPlayEnumerateW) \
	visit(DirectPlayLobbyCreateA) \
	visit(DirectPlayLobbyCreateW)

namespace dplayx
{
	class dplayx_dll
	{
	public:
		HMODULE dll = nullptr;
		VISIT_DPLAYX_PROCS(ADD_FARPROC_MEMBER);

		void Load()
		{
			if (Config.WrapperMode != dtype.dplayx && Config.WrapperMode != dtype.Auto)
			{
				return;
			}

			// Load real dll
			dll = LoadDll(dtype.dplayx);

			// Load dll functions
			if (dll)
			{
				VISIT_DPLAYX_PROCS(LOAD_ORIGINAL_PROC);
			}
		}

		void Unhook() {}
	};

	extern dplayx_dll module;
}
