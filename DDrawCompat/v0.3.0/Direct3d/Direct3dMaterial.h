#pragma once

#include <../DDrawLog.h>

namespace Direct3d
{
	namespace Direct3dMaterial
	{
		template <typename Vtable>
		void hookVtable(const Vtable& vtable);
	}
}
