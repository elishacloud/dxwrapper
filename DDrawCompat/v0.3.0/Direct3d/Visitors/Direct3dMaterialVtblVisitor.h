#pragma once

#include <d3d.h>

#include <DDrawCompat/v0.3.0/Common/VtableVisitor.h>

template <>
struct VtableForEach<IDirect3DMaterialVtbl>
{
	template <typename Vtable, typename Visitor>
	static void forEach(Visitor& visitor, UINT version)
	{
		VtableForEach<IUnknownVtbl>::forEach<Vtable>(visitor, version);

		DD_VISIT(Initialize);
		DD_VISIT(SetMaterial);
		DD_VISIT(GetMaterial);
		DD_VISIT(GetHandle);
		DD_VISIT(Reserve);
		DD_VISIT(Unreserve);
	}
};

template <>
struct VtableForEach<IDirect3DMaterial2Vtbl>
{
	template <typename Vtable, typename Visitor>
	static void forEach(Visitor& visitor, UINT version)
	{
		VtableForEach<IUnknownVtbl>::forEach<Vtable>(visitor, version);

		DD_VISIT(SetMaterial);
		DD_VISIT(GetMaterial);
		DD_VISIT(GetHandle);
	}
};

template <>
struct VtableForEach<IDirect3DMaterial3Vtbl>
{
	template <typename Vtable, typename Visitor>
	static void forEach(Visitor& visitor, UINT version)
	{
		VtableForEach<IDirect3DMaterial2Vtbl>::forEach<Vtable>(visitor, version);
	}
};
