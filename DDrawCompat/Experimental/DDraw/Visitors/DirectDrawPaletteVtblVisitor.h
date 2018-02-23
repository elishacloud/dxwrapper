#pragma once

#include "Common/VtableVisitor.h"

template <>
struct VtableForEach<IDirectDrawPaletteVtbl>
{
	template <typename Vtable, typename Visitor>
	static void forEach(Visitor& visitor)
	{
		VtableForEach<IUnknownVtbl>::forEach<Vtable>(visitor);

		DD_VISIT(GetCaps);
		DD_VISIT(GetEntries);
		DD_VISIT(Initialize);
		DD_VISIT(SetEntries);
	}
};
