#pragma once

#include <DDrawCompat/v0.3.0/Common/VtableVisitor.h>

template <>
struct VtableForEach<IDirectDrawClipperVtbl>
{
	template <typename Vtable, typename Visitor>
	static void forEach(Visitor& visitor, UINT version)
	{
		VtableForEach<IUnknownVtbl>::forEach<Vtable>(visitor, version);

		DD_VISIT(GetClipList);
		DD_VISIT(GetHWnd);
		DD_VISIT(Initialize);
		DD_VISIT(IsClipListChanged);
		DD_VISIT(SetClipList);
		DD_VISIT(SetHWnd);
	}
};
