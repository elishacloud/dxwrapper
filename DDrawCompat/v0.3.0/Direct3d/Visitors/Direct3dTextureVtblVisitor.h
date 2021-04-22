#pragma once

#include <d3d.h>

#include <Common/VtableVisitor.h>

template <>
struct VtableForEach<IDirect3DTextureVtbl>
{
	template <typename Vtable, typename Visitor>
	static void forEach(Visitor& visitor, UINT version)
	{
		VtableForEach<IUnknownVtbl>::forEach<Vtable>(visitor, version);

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
	static void forEach(Visitor& visitor, UINT version)
	{
		VtableForEach<IUnknownVtbl>::forEach<Vtable>(visitor, version);

		DD_VISIT(GetHandle);
		DD_VISIT(PaletteChanged);
		DD_VISIT(Load);
	}
};
