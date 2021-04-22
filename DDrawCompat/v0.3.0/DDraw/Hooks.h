#pragma once

#include <ddraw.h>

#include <Common/CompatPtr.h>

namespace DDraw
{
	void installHooks(CompatPtr<IDirectDraw7> dd7);
}
