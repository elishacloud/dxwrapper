#pragma once

#include "DDrawVtableVisitor.h"

namespace Compat20
{
	template <>
	struct DDrawVtableForEach<IDirectDrawPaletteVtbl>
	{
		template <typename Vtable, typename Visitor>
		static void forEach(Visitor& visitor)
		{
			DDrawVtableForEach<IUnknownVtbl>::forEach<Vtable>(visitor);

			DD_VISIT(GetCaps);
			DD_VISIT(GetEntries);
			DD_VISIT(Initialize);
			DD_VISIT(SetEntries);
		}
	};
}
