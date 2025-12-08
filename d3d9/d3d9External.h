#pragma once

#define INITGUID

#include <d3d9.h>
#include "d3d9Shared.h"
#include "Wrappers\wrapper.h"

// Initial screen resolution
extern volatile LONG InitWidth;
extern volatile LONG InitHeight;

typedef LPDIRECT3D9(WINAPI* Direct3DCreate9Proc)(UINT SDKVersion);
typedef HRESULT(WINAPI* Direct3DCreate9ExProc)(UINT, IDirect3D9Ex**);

int WINAPI d9_D3DPERF_BeginEvent(D3DCOLOR col, LPCWSTR wszName);
int WINAPI d9_D3DPERF_EndEvent();
DWORD WINAPI d9_D3DPERF_GetStatus();
BOOL WINAPI d9_D3DPERF_QueryRepeatFrame();
void WINAPI d9_D3DPERF_SetMarker(D3DCOLOR col, LPCWSTR wszName);
void WINAPI d9_D3DPERF_SetOptions(DWORD dwOptions);
void WINAPI d9_D3DPERF_SetRegion(D3DCOLOR col, LPCWSTR wszName);
void WINAPI Direct3D9SetSwapEffectUpgradeShim(int Unknown);
IDirect3D9* WINAPI d9_Direct3DCreate9(UINT SDKVersion);
HRESULT WINAPI d9_Direct3DCreate9Ex(UINT SDKVersion, IDirect3D9Ex** ppD3D);
IDirect3D9* WINAPI d9_Direct3DCreate9On12(UINT SDKVersion, D3D9ON12_ARGS* pOverrideList, UINT NumOverrideEntries);
HRESULT WINAPI d9_Direct3DCreate9On12Ex(UINT SDKVersion, D3D9ON12_ARGS* pOverrideList, UINT NumOverrideEntries, IDirect3D9Ex** ppOutputInterface);

class m_IDirect3D9Ex
{
public:
	void SetDirectXVersion(DWORD DxVersion);
	static DWORD AdjustPOW2Caps(DWORD OriginalCaps);
	static void AdjustWindow(HMONITOR hMonitor, HWND MainhWnd, LONG displayWidth, LONG displayHeight, bool EnableWindowMode, bool FullscreenWindowMode);
};

#define DECLARE_IN_WRAPPED_PROC(procName, unused) \
	const FARPROC procName ## _in = (FARPROC)*d9_ ## procName;

namespace D3d9Wrapper
{
	VISIT_PROCS_D3D9(DECLARE_IN_WRAPPED_PROC);

	VISIT_PROCS_D3D9(EXPORT_OUT_WRAPPED_PROC);
}

#undef DECLARE_IN_WRAPPED_PROC
