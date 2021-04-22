#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <Common/CompatVtable.h>
#include <DDraw/ScopedThreadLock.h>
#include <Direct3d/Direct3dLight.h>
#include <Direct3d/Visitors/Direct3dLightVtblVisitor.h>

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
