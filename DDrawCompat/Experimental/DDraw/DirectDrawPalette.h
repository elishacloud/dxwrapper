#pragma once

#include "Common/CompatVtable.h"
#include "DDraw/Visitors/DirectDrawPaletteVtblVisitor.h"

namespace DDraw
{
	class DirectDrawPalette : public CompatVtable<IDirectDrawPaletteVtbl>
	{
	public:
		static void setCompatVtable(IDirectDrawPaletteVtbl& vtable);

		static HRESULT STDMETHODCALLTYPE SetEntries(
			IDirectDrawPalette* This,
			DWORD dwFlags,
			DWORD dwStartingEntry,
			DWORD dwCount,
			LPPALETTEENTRY lpEntries);

		static void waitForNextUpdate();
	};
}

SET_COMPAT_VTABLE(IDirectDrawPaletteVtbl, DDraw::DirectDrawPalette);
