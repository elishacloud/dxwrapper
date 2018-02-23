#pragma once

#define CINTERFACE

#include <d3d.h>
#include <d3dumddi.h>
#include <d3dnthal.h>

#include "Common/CompatVtable.h"
#include "D3dDdi/Log/AdapterFuncsLog.h"
#include "D3dDdi/Visitors/AdapterFuncsVisitor.h"

namespace D3dDdi
{
	class AdapterFuncs : public CompatVtable<D3DDDI_ADAPTERFUNCS>
	{
	public:
		static const D3DNTHAL_D3DEXTENDEDCAPS& getD3dExtendedCaps(HANDLE adapter);
		static void onOpenAdapter(HANDLE adapter);
		static void setCompatVtable(D3DDDI_ADAPTERFUNCS& vtable);
	};
}

SET_COMPAT_VTABLE(D3DDDI_ADAPTERFUNCS, D3dDdi::AdapterFuncs);
