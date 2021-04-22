#pragma once

#include <Common/VtableVisitor.h>

template <>
struct VtableForEach<IDirectDrawPaletteVtbl>
{
	template <typename Vtable, typename Visitor>
	static void forEach(Visitor& visitor, UINT version)
	{
		VtableForEach<IUnknownVtbl>::forEach<Vtable>(visitor, version);

		DD_VISIT(GetCaps);
		DD_VISIT(GetEntries);
		DD_VISIT(Initialize);
		DD_VISIT(SetEntries);
	}
};
