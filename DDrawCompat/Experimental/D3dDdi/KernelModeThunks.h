#pragma once

#include "D3dDdi/Log/KernelModeThunksLog.h"

static const auto D3DDDI_FLIPINTERVAL_NOOVERRIDE = static_cast<D3DDDI_FLIPINTERVAL_TYPE>(5);

namespace D3dDdi
{
	namespace KernelModeThunks
	{
		void installHooks();
		bool isPresentReady();
		void overrideFlipInterval(D3DDDI_FLIPINTERVAL_TYPE flipInterval);
		void releaseVidPnSources();
	}
}
