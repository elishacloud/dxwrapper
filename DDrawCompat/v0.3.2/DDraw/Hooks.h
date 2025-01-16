#pragma once

#include <ddraw.h>

#include <DDrawCompat/v0.3.2/Common/CompatPtr.h>

namespace DDraw
{
	void installHooks(CompatPtr<IDirectDraw7> dd7);
}
