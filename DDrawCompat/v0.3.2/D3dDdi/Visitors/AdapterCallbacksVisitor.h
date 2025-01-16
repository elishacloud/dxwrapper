#pragma once

#include <d3d.h>
#include <d3dumddi.h>

#include <DDrawCompat/v0.3.2/Common/VtableVisitor.h>

template <>
struct VtableForEach<D3DDDI_ADAPTERCALLBACKS>
{
	template <typename Vtable, typename Visitor>
	static void forEach(Visitor& visitor, UINT version)
	{
		DD_VISIT(pfnQueryAdapterInfoCb);
		DD_VISIT(pfnGetMultisampleMethodListCb);

		if (version >= D3D_UMD_INTERFACE_VERSION_WDDM2_4)
		{
			DD_VISIT(pfnQueryAdapterInfoCb2);
		}
	}
};
