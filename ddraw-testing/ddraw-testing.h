#pragma once

#define INITGUID
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <fstream>

#include <ddraw.h>
#include <ddrawex.h>
#include <d3d.h>
#include <d3dtypes.h>
#include <d3dhal.h>

#include "Logging.h"

// ddraw proc typedefs
typedef HRESULT(WINAPI* AcquireDDThreadLockProc)();
typedef DWORD(WINAPI* CompleteCreateSysmemSurfaceProc)(DWORD);
typedef HRESULT(WINAPI* D3DParseUnknownCommandProc)(LPVOID lpCmd, LPVOID* lpRetCmd);
typedef HRESULT(WINAPI* DDGetAttachedSurfaceLclProc)(DWORD, DWORD, DWORD);
typedef DWORD(WINAPI* DDInternalLockProc)(DWORD, DWORD);
typedef DWORD(WINAPI* DDInternalUnlockProc)(DWORD);
typedef HRESULT(WINAPI* DSoundHelpProc)(DWORD, DWORD, DWORD);
typedef HRESULT(WINAPI* DirectDrawCreateProc)(GUID FAR* lpGUID, LPDIRECTDRAW FAR* lplpDD, IUnknown FAR* pUnkOuter);
typedef HRESULT(WINAPI* DirectDrawCreateClipperProc)(DWORD dwFlags, LPDIRECTDRAWCLIPPER* lplpDDClipper, LPUNKNOWN pUnkOuter);
typedef HRESULT(WINAPI* DirectDrawEnumerateAProc)(LPDDENUMCALLBACKA lpCallback, LPVOID lpContext);
typedef HRESULT(WINAPI* DirectDrawEnumerateExAProc)(LPDDENUMCALLBACKEXA lpCallback, LPVOID lpContext, DWORD dwFlags);
typedef HRESULT(WINAPI* DirectDrawEnumerateExWProc)(LPDDENUMCALLBACKEXW lpCallback, LPVOID lpContext, DWORD dwFlags);
typedef HRESULT(WINAPI* DirectDrawEnumerateWProc)(LPDDENUMCALLBACKW lpCallback, LPVOID lpContext);
typedef HRESULT(WINAPI* DirectDrawCreateExProc)(GUID FAR* lpGUID, LPVOID* lplpDD, REFIID riid, IUnknown FAR* pUnkOuter);
typedef HRESULT(WINAPI* DllCanUnloadNowProc)();
typedef HRESULT(WINAPI* DllGetClassObjectProc)(REFCLSID rclsid, REFIID riid, LPVOID* ppv);
typedef HRESULT(WINAPI* GetDDSurfaceLocalProc)(DWORD, DWORD, DWORD);
typedef DWORD(WINAPI* GetOLEThunkDataProc)(DWORD index);
typedef HRESULT(WINAPI* GetSurfaceFromDCProc)(HDC hdc, LPDIRECTDRAWSURFACE7* lpDDS, DWORD);
typedef HRESULT(WINAPI* RegisterSpecialCaseProc)(DWORD, DWORD, DWORD, DWORD);
typedef HRESULT(WINAPI* ReleaseDDThreadLockProc)();
typedef HRESULT(WINAPI* SetAppCompatDataProc)(DWORD Type, DWORD Value);

class IDirectDraw7Ex : public IDirectDraw7 {};

extern HWND DDhWnd;

extern DirectDrawCreateProc pDirectDrawCreate;
extern DirectDrawCreateExProc pDirectDrawCreateEx;
extern DirectDrawEnumerateAProc pDirectDrawEnumerateA;
extern DirectDrawEnumerateExAProc pDirectDrawEnumerateExA;
extern DirectDrawEnumerateExWProc pDirectDrawEnumerateExW;
extern DirectDrawEnumerateWProc pDirectDrawEnumerateW;

template <typename T>
static UINT GetRefCount(T IUnknownAddr)
{
    UINT ref = IUnknownAddr->AddRef() - 1;
    IUnknownAddr->Release(); // Undo AddRef
    return ref;
}

void TestEnumDisplaySettings();

template <typename DDType>
void TestQueryInterfaces(DDType* pDDraw, IUnknown* pInterface, const char* ParentName, DWORD TestIDBase);

template <typename DDType>
void TestDirectDrawCreate();

template <typename DDType>
void TestCreateSurface(DDType* pDDraw);

template <typename DDType>
void TestCreateDirect3D(DDType* pDDraw);

template <typename DDType, typename D3DType>
void TestCreate3DDevice(DDType* pDDraw, D3DType* pDirect3D);
