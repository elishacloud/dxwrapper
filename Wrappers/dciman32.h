#pragma once

#define VISIT_DCIMAN32_PROCS(visit) \
	visit(DCIBeginAccess) \
	visit(DCICloseProvider) \
	visit(DCICreateOffscreen) \
	visit(DCICreateOverlay) \
	visit(DCICreatePrimary) \
	visit(DCIDestroy) \
	visit(DCIDraw) \
	visit(DCIEndAccess) \
	visit(DCIEnum) \
	visit(DCIOpenProvider) \
	visit(DCISetClipList) \
	visit(DCISetDestination) \
	visit(DCISetSrcDestClip) \
	visit(GetDCRegionData) \
	visit(GetWindowRegionData) \
	visit(WinWatchClose) \
	visit(WinWatchDidStatusChange) \
	visit(WinWatchGetClipList) \
	visit(WinWatchNotify) \
	visit(WinWatchOpen)

namespace dciman32
{
	class dciman32_dll
	{
	public:
		HMODULE dll = nullptr;
		VISIT_DCIMAN32_PROCS(ADD_FARPROC_MEMBER);

		void Load()
		{
			if (Config.WrapperMode != dtype.dciman32 && Config.WrapperMode != dtype.Auto)
			{
				return;
			}

			// Load real dll
			dll = LoadDll(dtype.dciman32);

			// Load dll functions
			if (dll)
			{
				VISIT_DCIMAN32_PROCS(LOAD_ORIGINAL_PROC);
			}
		}

		void Unhook() {}
	};

	extern dciman32_dll module;
};
