#pragma once

#include <vector>

#include <Windows.h>

#include <DDrawCompat/v0.3.2/Common/CompatPtr.h>
#include <DDrawCompat/v0.3.2/Common/CompatRef.h>

namespace DDraw
{
	namespace DirectDrawSurface
	{
		std::vector<CompatPtr<IDirectDrawSurface7>> getAllAttachedSurfaces(CompatRef<IDirectDrawSurface7> surface);

		template <typename TSurface>
		HANDLE getRuntimeResourceHandle(TSurface& surface)
		{
			return reinterpret_cast<HANDLE**>(&surface)[1][2];
		}

		template <typename TSurface>
		HANDLE getDriverResourceHandle(TSurface& surface)
		{
			return *reinterpret_cast<HANDLE*>(getRuntimeResourceHandle(surface));
		}

		template <typename Vtable>
		void hookVtable(const Vtable& vtable);
	}
}
