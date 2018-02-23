#pragma once

#include "Common/VtableVisitor.h"

template <>
struct VtableForEach<IDirectDrawGammaControlVtbl>
{
	template <typename Vtable, typename Visitor>
	static void forEach(Visitor& visitor)
	{
		VtableForEach<IUnknownVtbl>::forEach<Vtable>(visitor);

		DD_VISIT(GetGammaRamp);
		DD_VISIT(SetGammaRamp);
	}
};
