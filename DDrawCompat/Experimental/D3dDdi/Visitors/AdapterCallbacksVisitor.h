#pragma once

#define CINTERFACE

#include <d3d.h>
#include <d3dumddi.h>

#include "Common/VtableVisitor.h"

struct AdapterCallbacksIntf
{
	D3DDDI_ADAPTERCALLBACKS* lpVtbl;
};

template <>
struct VtableForEach<D3DDDI_ADAPTERCALLBACKS>
{
	template <typename Vtable, typename Visitor>
	static void forEach(Visitor& visitor)
	{
		DD_VISIT(pfnQueryAdapterInfoCb);
		// DD_VISIT(pfnGetMultisampleMethodListCb);   -- not set by ddraw, potentially garbage
	}
};
