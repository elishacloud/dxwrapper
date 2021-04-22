#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <Common/CompatVtable.h>
#include <DDraw/ScopedThreadLock.h>
#include <Direct3d/Direct3dMaterial.h>
#include <Direct3d/Visitors/Direct3dMaterialVtblVisitor.h>

namespace
{
	template <typename Vtable>
	constexpr void setCompatVtable(Vtable& /*vtable*/)
	{
	}
}

namespace Direct3d
{
	namespace Direct3dMaterial
	{
		template <typename Vtable>
		void hookVtable(const Vtable& vtable)
		{
			CompatVtable<Vtable>::hookVtable<DDraw::ScopedThreadLock>(vtable);
		}

		template void hookVtable(const IDirect3DMaterialVtbl&);
		template void hookVtable(const IDirect3DMaterial2Vtbl&);
		template void hookVtable(const IDirect3DMaterial3Vtbl&);
	}
}
