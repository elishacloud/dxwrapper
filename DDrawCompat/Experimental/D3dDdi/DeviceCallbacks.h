#pragma once

#define CINTERFACE

#include <d3d.h>
#include <d3dumddi.h>

#include "Common/CompatVtable.h"
#include "D3dDdi/Log/DeviceCallbacksLog.h"
#include "D3dDdi/Visitors/DeviceCallbacksVisitor.h"

namespace D3dDdi
{
	class DeviceCallbacks : public CompatVtable<D3DDDI_DEVICECALLBACKS>
	{
	public:
		static void setCompatVtable(D3DDDI_DEVICECALLBACKS& vtable);
	};
}

SET_COMPAT_VTABLE(D3DDDI_DEVICECALLBACKS, D3dDdi::DeviceCallbacks);
