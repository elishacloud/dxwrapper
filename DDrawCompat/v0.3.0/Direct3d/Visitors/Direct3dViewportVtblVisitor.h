#pragma once

#include <d3d.h>

#include <Common/VtableVisitor.h>

template <>
struct VtableForEach<IDirect3DViewportVtbl>
{
	template <typename Vtable, typename Visitor>
	static void forEach(Visitor& visitor, UINT version)
	{
		VtableForEach<IUnknownVtbl>::forEach<Vtable>(visitor, version);

		DD_VISIT(Initialize);
		DD_VISIT(GetViewport);
		DD_VISIT(SetViewport);
		DD_VISIT(TransformVertices);
		DD_VISIT(LightElements);
		DD_VISIT(SetBackground);
		DD_VISIT(GetBackground);
		DD_VISIT(SetBackgroundDepth);
		DD_VISIT(GetBackgroundDepth);
		DD_VISIT(Clear);
		DD_VISIT(AddLight);
		DD_VISIT(DeleteLight);
		DD_VISIT(NextLight);
	}
};

template <>
struct VtableForEach<IDirect3DViewport2Vtbl>
{
	template <typename Vtable, typename Visitor>
	static void forEach(Visitor& visitor, UINT version)
	{
		VtableForEach<IDirect3DViewportVtbl>::forEach<Vtable>(visitor, version);

		DD_VISIT(GetViewport2);
		DD_VISIT(SetViewport2);
	}
};

template <>
struct VtableForEach<IDirect3DViewport3Vtbl>
{
	template <typename Vtable, typename Visitor>
	static void forEach(Visitor& visitor, UINT version)
	{
		VtableForEach<IDirect3DViewport2Vtbl>::forEach<Vtable>(visitor, version);

		DD_VISIT(SetBackgroundDepth2);
		DD_VISIT(GetBackgroundDepth2);
		DD_VISIT(Clear2);
	}
};
