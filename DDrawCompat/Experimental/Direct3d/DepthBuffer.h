#pragma once

#include <guiddef.h>

#include "Common/CompatPtr.h"

namespace Direct3d
{
	namespace DepthBuffer
	{
		template <typename TDirect3d, typename TD3dDeviceDesc>
		void fixSupportedZBufferBitDepths(CompatPtr<TDirect3d> d3d, TD3dDeviceDesc& desc);
	}
}
