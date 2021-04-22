#pragma once

#include <ddraw.h>

namespace DDraw
{
	namespace DirectDrawGammaControl
	{
		void hookVtable(const IDirectDrawGammaControlVtbl& vtable);
	}
}
