#pragma once

#include <d3d.h>
#include <d3dumddi.h>

#include <DDrawCompat/v0.3.0/D3dDdi/Log/AdapterFuncsLog.h>

namespace D3dDdi
{
	namespace AdapterFuncs
	{
		void hookVtable(const D3DDDI_ADAPTERFUNCS& vtable, UINT version);
	}
}
