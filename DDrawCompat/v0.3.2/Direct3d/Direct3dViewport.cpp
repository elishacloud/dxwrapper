#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <DDrawCompat/v0.3.2/Common/CompatVtable.h>
#include <DDrawCompat/v0.3.2/DDraw/ScopedThreadLock.h>
#include <DDrawCompat/v0.3.2/Direct3d/Direct3dViewport.h>
#include <DDrawCompat/v0.3.2/Direct3d/Visitors/Direct3dViewportVtblVisitor.h>

namespace
{
	template <typename Vtable>
	constexpr void setCompatVtable(Vtable& /*vtable*/)
	{
	}
}

namespace Direct3d
{
	namespace Direct3dViewport
	{
		template <typename Vtable>
		void hookVtable(const Vtable& vtable)
		{
			CompatVtable<Vtable>::hookVtable<DDraw::ScopedThreadLock>(vtable);
		}

		template void hookVtable(const IDirect3DViewportVtbl&);
		template void hookVtable(const IDirect3DViewport2Vtbl&);
		template void hookVtable(const IDirect3DViewport3Vtbl&);
	}
}
