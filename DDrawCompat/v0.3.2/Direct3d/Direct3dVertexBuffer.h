#pragma once

#include <DDrawCompat/DDrawLog.h>

namespace Direct3d
{
	namespace Direct3dVertexBuffer
	{
		template <typename Vtable>
		void hookVtable(const Vtable& vtable);
	}
}
