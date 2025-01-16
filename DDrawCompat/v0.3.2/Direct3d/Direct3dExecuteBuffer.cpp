#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <DDrawCompat/v0.3.2/Common/CompatVtable.h>
#include <DDrawCompat/v0.3.2/DDraw/ScopedThreadLock.h>
#include <DDrawCompat/v0.3.2/Direct3d/Direct3dExecuteBuffer.h>
#include <DDrawCompat/v0.3.2/Direct3d/Visitors/Direct3dExecuteBufferVtblVisitor.h>

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
