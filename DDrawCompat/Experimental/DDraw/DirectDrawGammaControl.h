#pragma once

#include "Common/CompatVtable.h"
#include "DDraw/Visitors/DirectDrawGammaControlVtblVisitor.h"

namespace DDraw
{
	class DirectDrawGammaControl : public CompatVtable<IDirectDrawGammaControlVtbl>
	{
	public:
		static void setCompatVtable(IDirectDrawGammaControlVtbl& vtable);
	};
}

SET_COMPAT_VTABLE(IDirectDrawGammaControlVtbl, DDraw::DirectDrawGammaControl);
