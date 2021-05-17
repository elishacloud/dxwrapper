#pragma once

#include <d3d.h>
#include <d3dumddi.h>

#include <DDrawCompat/v0.3.1/D3dDdi/Log/DeviceFuncsLog.h>

namespace D3dDdi
{
	namespace DeviceFuncs
	{
		void hookVtable(const D3DDDI_DEVICEFUNCS& vtable, UINT version);
	}
}
