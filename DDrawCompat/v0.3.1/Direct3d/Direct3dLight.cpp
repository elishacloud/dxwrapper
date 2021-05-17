#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <DDrawCompat/v0.3.1/Common/CompatVtable.h>
#include <DDrawCompat/v0.3.1/DDraw/ScopedThreadLock.h>
#include <DDrawCompat/v0.3.1/Direct3d/Direct3dLight.h>
#include <DDrawCompat/v0.3.1/Direct3d/Visitors/Direct3dLightVtblVisitor.h>

namespace
{
	constexpr void setCompatVtable(IDirect3DLightVtbl& /*vtable*/)
	{
	}
}

namespace Direct3d
{
	namespace Direct3dLight
	{
		void hookVtable(const IDirect3DLightVtbl & vtable)
		{
			CompatVtable<IDirect3DLightVtbl>::hookVtable<DDraw::ScopedThreadLock>(vtable);
		}
	}
}
