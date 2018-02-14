#pragma once

HRESULT WINAPI dd_DirectDrawCreate(GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter);
HRESULT WINAPI dd_DirectDrawCreateEx(GUID FAR *lpGUID, LPVOID *lplpDD, REFIID riid, IUnknown FAR *pUnkOuter);

namespace DdrawWrapper
{
	constexpr FARPROC in_DirectDrawCreate = (FARPROC)*dd_DirectDrawCreate;
	constexpr FARPROC in_DirectDrawCreateEx = (FARPROC)*dd_DirectDrawCreateEx;
	extern FARPROC out_DirectDrawCreate;
	extern FARPROC out_DirectDrawCreateEx;
}
