#pragma once

#include <d3d.h>

#include <DDrawCompat/DDrawLog.h>

namespace Direct3d
{
	namespace Direct3dLight
	{
		void hookVtable(const IDirect3DLightVtbl& vtable);
	}
}
