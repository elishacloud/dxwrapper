#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <Common/CompatVtable.h>
#include <DDraw/ScopedThreadLock.h>
#include <Direct3d/Direct3dExecuteBuffer.h>
#include <Direct3d/Visitors/Direct3dExecuteBufferVtblVisitor.h>

namespace
{
	constexpr void setCompatVtable(IDirect3DExecuteBufferVtbl& /*vtable*/)
	{
	}
}

namespace Direct3d
{
	namespace Direct3dExecuteBuffer
	{
		void hookVtable(const IDirect3DExecuteBufferVtbl& vtable)
		{
			CompatVtable<IDirect3DExecuteBufferVtbl>::hookVtable<DDraw::ScopedThreadLock>(vtable);
		}
	}
}
