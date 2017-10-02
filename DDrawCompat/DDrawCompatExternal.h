#pragma once

BOOL WINAPI DllMain_DDrawCompat(HINSTANCE, DWORD, LPVOID);

extern "C" HRESULT WINAPI _DirectDrawCreate(GUID*, LPDIRECTDRAW*, IUnknown*);
extern "C" HRESULT WINAPI _DirectDrawCreateEx(GUID*, LPVOID*, REFIID, IUnknown*);
extern "C" HRESULT WINAPI _DllGetClassObject(REFCLSID, REFIID, LPVOID*);

namespace DDrawCompat
{
	constexpr FARPROC DirectDrawCreate = (FARPROC)*_DirectDrawCreate;
	constexpr FARPROC DirectDrawCreateEx = (FARPROC)*_DirectDrawCreateEx;
	constexpr FARPROC DllGetClassObject = (FARPROC)*_DllGetClassObject;
}
