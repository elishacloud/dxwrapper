#pragma once

#define CINTERFACE

#include <d3d.h>
#include <d3dumddi.h>

#include "Common/CompatVtable.h"
#include "D3dDdi/Visitors/AdapterCallbacksVisitor.h"

namespace D3dDdi
{
	class AdapterCallbacks : public CompatVtable<D3DDDI_ADAPTERCALLBACKS>
	{
	public:
		static void setCompatVtable(D3DDDI_ADAPTERCALLBACKS& vtable);
	};
}

SET_COMPAT_VTABLE(D3DDDI_ADAPTERCALLBACKS, D3dDdi::AdapterCallbacks);
