#pragma once

#include <ddraw.h>

#include <Common/CompatPtr.h>

namespace Direct3d
{
	void installHooks(CompatPtr<IDirectDraw> dd, CompatPtr<IDirectDraw7> dd7);
}
