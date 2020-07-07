/**
* Copyright (C) 2020 Elisha Riedlinger
*
* This software is  provided 'as-is', without any express  or implied  warranty. In no event will the
* authors be held liable for any damages arising from the use of this software.
* Permission  is granted  to anyone  to use  this software  for  any  purpose,  including  commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*   1. The origin of this software must not be misrepresented; you must not claim that you  wrote the
*      original  software. If you use this  software  in a product, an  acknowledgment in the product
*      documentation would be appreciated but is not required.
*   2. Altered source versions must  be plainly  marked as such, and  must not be  misrepresented  as
*      being the original software.
*   3. This notice may not be removed or altered from any source distribution.
*/

#include "..\ddraw.h"

HRESULT m_IDirectDraw7::QueryInterface(REFIID riid, LPVOID FAR * ppvObj)
{
	return ProxyInterface->QueryInterface(ReplaceIIDUnknown(riid, WrapperID), ppvObj);
}

ULONG m_IDirectDraw7::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG m_IDirectDraw7::Release()
{
	return ProxyInterface->Release();
}

HRESULT m_IDirectDraw7::Compact()
{
	return ProxyInterface->Compact();
}

HRESULT m_IDirectDraw7::CreateClipper(DWORD a, LPDIRECTDRAWCLIPPER FAR * b, IUnknown FAR * c)
{
	return ProxyInterface->CreateClipper(a, b, c);
}

HRESULT m_IDirectDraw7::CreatePalette(DWORD a, LPPALETTEENTRY b, LPDIRECTDRAWPALETTE FAR * c, IUnknown FAR * d)
{
	return ProxyInterface->CreatePalette(a, b, c, d);
}

HRESULT m_IDirectDraw7::CreateSurface(LPDDSURFACEDESC2 a, LPDIRECTDRAWSURFACE7 FAR * b, IUnknown FAR * c)
{
	return ProxyInterface->CreateSurface2(a, b, c, DirectXVersion);
}

HRESULT m_IDirectDraw7::DuplicateSurface(LPDIRECTDRAWSURFACE7 a, LPDIRECTDRAWSURFACE7 FAR * b)
{
	return ProxyInterface->DuplicateSurface(a, b, DirectXVersion);
}

HRESULT m_IDirectDraw7::EnumDisplayModes(DWORD a, LPDDSURFACEDESC2 b, LPVOID c, LPDDENUMMODESCALLBACK2 d)
{
	return ProxyInterface->EnumDisplayModes2(a, b, c, d);
}

HRESULT m_IDirectDraw7::EnumSurfaces(DWORD a, LPDDSURFACEDESC2 b, LPVOID c, LPDDENUMSURFACESCALLBACK7 d)
{
	return ProxyInterface->EnumSurfaces2(a, b, c, d, DirectXVersion);
}

HRESULT m_IDirectDraw7::FlipToGDISurface()
{
	return ProxyInterface->FlipToGDISurface();
}

HRESULT m_IDirectDraw7::GetCaps(LPDDCAPS lpDDDriverCaps, LPDDCAPS lpDDHELCaps)
{
	return ProxyInterface->GetCaps(lpDDDriverCaps, lpDDHELCaps);
}

HRESULT m_IDirectDraw7::GetDisplayMode(LPDDSURFACEDESC2 a)
{
	return ProxyInterface->GetDisplayMode2(a);
}

HRESULT m_IDirectDraw7::GetFourCCCodes(LPDWORD a, LPDWORD b)
{
	return ProxyInterface->GetFourCCCodes(a, b);
}

HRESULT m_IDirectDraw7::GetGDISurface(LPDIRECTDRAWSURFACE7 FAR * a)
{
	return ProxyInterface->GetGDISurface(a, DirectXVersion);
}

HRESULT m_IDirectDraw7::GetMonitorFrequency(LPDWORD a)
{
	return ProxyInterface->GetMonitorFrequency(a);
}

HRESULT m_IDirectDraw7::GetScanLine(LPDWORD a)
{
	return ProxyInterface->GetScanLine(a);
}

HRESULT m_IDirectDraw7::GetVerticalBlankStatus(LPBOOL a)
{
	return ProxyInterface->GetVerticalBlankStatus(a);
}

HRESULT m_IDirectDraw7::Initialize(GUID FAR * a)
{
	return ProxyInterface->Initialize(a);
}

HRESULT m_IDirectDraw7::RestoreDisplayMode()
{
	return ProxyInterface->RestoreDisplayMode();
}

HRESULT m_IDirectDraw7::SetCooperativeLevel(HWND a, DWORD b)
{
	return ProxyInterface->SetCooperativeLevel(a, b);
}

HRESULT m_IDirectDraw7::SetDisplayMode(DWORD a, DWORD b, DWORD c, DWORD d, DWORD e)
{
	return ProxyInterface->SetDisplayMode(a, b, c, d, e);
}

HRESULT m_IDirectDraw7::WaitForVerticalBlank(DWORD a, HANDLE b)
{
	return ProxyInterface->WaitForVerticalBlank(a, b);
}

HRESULT m_IDirectDraw7::GetAvailableVidMem(LPDDSCAPS2 a, LPDWORD b, LPDWORD c)
{
	return ProxyInterface->GetAvailableVidMem2(a, b, c);
}

HRESULT m_IDirectDraw7::GetSurfaceFromDC(HDC a, LPDIRECTDRAWSURFACE7 * b)
{
	return ProxyInterface->GetSurfaceFromDC(a, b, DirectXVersion);
}

HRESULT m_IDirectDraw7::RestoreAllSurfaces()
{
	return ProxyInterface->RestoreAllSurfaces();
}

HRESULT m_IDirectDraw7::TestCooperativeLevel()
{
	return ProxyInterface->TestCooperativeLevel();
}

HRESULT m_IDirectDraw7::GetDeviceIdentifier(LPDDDEVICEIDENTIFIER2 a, DWORD b)
{
	return ProxyInterface->GetDeviceIdentifier2(a, b);
}

HRESULT m_IDirectDraw7::StartModeTest(LPSIZE a, DWORD b, DWORD c)
{
	return ProxyInterface->StartModeTest(a, b, c);
}

HRESULT m_IDirectDraw7::EvaluateMode(DWORD a, DWORD * b)
{
	return ProxyInterface->EvaluateMode(a, b);
}
