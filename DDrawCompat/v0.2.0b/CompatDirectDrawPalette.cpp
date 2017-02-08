#include "CompatDirectDrawPalette.h"
#include "CompatPrimarySurface.h"
#include "RealPrimarySurface.h"

void CompatDirectDrawPalette::setCompatVtable(IDirectDrawPaletteVtbl& vtable)
{
	vtable.SetEntries = &SetEntries;
}

HRESULT STDMETHODCALLTYPE CompatDirectDrawPalette::SetEntries(
	IDirectDrawPalette* This,
	DWORD dwFlags,
	DWORD dwStartingEntry,
	DWORD dwCount,
	LPPALETTEENTRY lpEntries)
{
	HRESULT result = s_origVtable.SetEntries(This, dwFlags, dwStartingEntry, dwCount, lpEntries);
	if (This == CompatPrimarySurface::palette && SUCCEEDED(result))
	{
		RealPrimarySurface::updatePalette();
	}
	return result;
}
