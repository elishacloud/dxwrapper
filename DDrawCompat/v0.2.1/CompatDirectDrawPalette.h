#pragma once

#include "CompatVtable.h"
#include "DirectDrawPaletteVtblVisitor.h"

namespace Compat21
{
	class CompatDirectDrawPalette : public CompatVtable<CompatDirectDrawPalette, IDirectDrawPalette>
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
