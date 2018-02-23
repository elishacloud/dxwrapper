#pragma once

#define CINTERFACE

#include <d3d.h>

#include "Common/VtableVisitor.h"

template <>
struct VtableForEach<IDirect3DVertexBufferVtbl>
{
	template <typename Vtable, typename Visitor>
	static void forEach(Visitor& visitor)
	{
		VtableForEach<IUnknownVtbl>::forEach<Vtable>(visitor);

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
	static void forEach(Visitor& visitor)
	{
		VtableForEach<IDirect3DVertexBufferVtbl>::forEach<Vtable>(visitor);

		DD_VISIT(ProcessVerticesStrided);
	}
};
