#pragma once

#include <d3d.h>

#include <DDrawCompat/DDrawLog.h>

namespace Direct3d
{
	namespace Direct3dExecuteBuffer
	{
		void hookVtable(const IDirect3DExecuteBufferVtbl& vtable);
	}
}
