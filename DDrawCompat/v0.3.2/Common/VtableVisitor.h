#pragma once

#include <ddraw.h>

template <typename Vtable>
struct VtableForEach;

template <typename Vtable, typename Visitor>
void forEach(Visitor& visitor, UINT version)
{
	VtableForEach<Vtable>::forEach<Vtable>(visitor, version);
}

#define DD_VISIT(member) visitor.visit<&Vtable::member>(#member)

template <>
struct VtableForEach<IUnknownVtbl>
{
	template <typename Vtable, typename Visitor>
	static void forEach(Visitor& visitor, UINT /*version*/)
	{
		DD_VISIT(QueryInterface);
		DD_VISIT(AddRef);
		DD_VISIT(Release);
	}
};
