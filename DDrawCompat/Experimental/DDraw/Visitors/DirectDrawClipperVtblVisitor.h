#pragma once

#include "Common/VtableVisitor.h"

template <>
struct VtableForEach<IDirectDrawClipperVtbl>
{
	template <typename Vtable, typename Visitor>
	static void forEach(Visitor& visitor)
	{
		VtableForEach<IUnknownVtbl>::forEach<Vtable>(visitor);

		DD_VISIT(GetClipList);
		DD_VISIT(GetHWnd);
		DD_VISIT(Initialize);
		DD_VISIT(IsClipListChanged);
		DD_VISIT(SetClipList);
		DD_VISIT(SetHWnd);
	}
};
