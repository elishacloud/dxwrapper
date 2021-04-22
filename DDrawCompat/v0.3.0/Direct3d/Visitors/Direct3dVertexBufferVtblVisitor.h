#pragma once

#include <d3d.h>

#include <Common/VtableVisitor.h>

template <>
struct VtableForEach<IDirect3DVertexBufferVtbl>
{
	template <typename Vtable, typename Visitor>
	static void forEach(Visitor& visitor, UINT version)
	{
		VtableForEach<IUnknownVtbl>::forEach<Vtable>(visitor, version);

		DD_VISIT(Lock);
		DD_VISIT(Unlock);
		DD_VISIT(ProcessVertices);
		DD_VISIT(GetVertexBufferDesc);
		DD_VISIT(Optimize);
	}
};

template <>
struct VtableForEach<IDirect3DVertexBuffer7Vtbl>
{
	template <typename Vtable, typename Visitor>
	static void forEach(Visitor& visitor, UINT version)
	{
		VtableForEach<IDirect3DVertexBufferVtbl>::forEach<Vtable>(visitor, version);

		DD_VISIT(ProcessVerticesStrided);
	}
};
