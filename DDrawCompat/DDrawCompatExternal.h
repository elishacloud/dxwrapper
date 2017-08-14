#pragma once

#include "Wrappers\Wrapper.h"

BOOL WINAPI DllMain_DDrawCompat(HINSTANCE, DWORD, LPVOID);

extern "C" HRESULT WINAPI DirectDrawCreate(GUID*, LPDIRECTDRAW*, IUnknown*);
extern "C" HRESULT WINAPI DirectDrawCreateEx(GUID*, LPVOID*, REFIID, IUnknown*);
extern "C" HRESULT WINAPI DllGetClassObject(REFCLSID, REFIID, LPVOID*);

namespace Wrapper
{
	namespace DDrawCompat
	{
		FARPROC _DirectDrawCreate = (FARPROC)*DirectDrawCreate;
		FARPROC _DirectDrawCreateEx = (FARPROC)*DirectDrawCreateEx;
		FARPROC _DllGetClassObject = (FARPROC)*DllGetClassObject;
	}

}
