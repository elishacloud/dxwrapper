#pragma once

#include "Common/CompatVtable.h"
#include "DDraw/Visitors/DirectDrawClipperVtblVisitor.h"

namespace DDraw
{
	class DirectDrawClipper : public CompatVtable<IDirectDrawClipperVtbl>
	{
	public:
		static void setCompatVtable(IDirectDrawClipperVtbl& vtable);
	};
}

SET_COMPAT_VTABLE(IDirectDrawClipperVtbl, DDraw::DirectDrawClipper);
