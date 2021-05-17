#pragma once

#include <d3d.h>

#include <DDrawCompat/v0.3.1/Common/VtableVisitor.h>

template <>
struct VtableForEach<IDirect3DLightVtbl>
{
	template <typename Vtable, typename Visitor>
	static void forEach(Visitor& visitor, UINT version)
	{
		VtableForEach<IUnknownVtbl>::forEach<Vtable>(visitor, version);

		DD_VISIT(Initialize);
		DD_VISIT(SetLight);
		DD_VISIT(GetLight);
	}
};
