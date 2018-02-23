#pragma once

#define CINTERFACE

#include <d3d.h>

#include "Common/VtableVisitor.h"

template <>
struct VtableForEach<IDirect3DTextureVtbl>
{
	template <typename Vtable, typename Visitor>
	static void forEach(Visitor& visitor)
	{
		VtableForEach<IUnknownVtbl>::forEach<Vtable>(visitor);

		DD_VISIT(Initialize);
		DD_VISIT(GetHandle);
		DD_VISIT(PaletteChanged);
		DD_VISIT(Load);
		DD_VISIT(Unload);
	}
};

template <>
struct VtableForEach<IDirect3DTexture2Vtbl>
{
	template <typename Vtable, typename Visitor>
	static void forEach(Visitor& visitor)
	{
		VtableForEach<IUnknownVtbl>::forEach<Vtable>(visitor);

		DD_VISIT(GetHandle);
		DD_VISIT(PaletteChanged);
		DD_VISIT(Load);
	}
};
