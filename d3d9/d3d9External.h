#pragma once

#define INITGUID

#include <d3d9.h>
#include "Wrappers\wrapper.h"

// Enable for testing only
//#define ENABLE_DEBUGOVERLAY

#ifdef ENABLE_DEBUGOVERLAY
#include "DebugOverlay.h"
extern DebugOverlay DOverlay;
LRESULT WINAPI ImGuiWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
#endif

typedef DWORD D3DCOLOR;
struct __declspec(uuid("81BDCBCA-64D4-426d-AE8D-AD0147F4275C")) IDirect3D9;
struct __declspec(uuid("02177241-69FC-400C-8FF1-93A44DF6861D")) IDirect3D9Ex;

#define MAX_D3D9ON12_QUEUES        2

typedef struct _D3D9ON12_ARGS
{
	BOOL Enable9On12;
	IUnknown* pD3D12Device;
	IUnknown* ppD3D12Queues[MAX_D3D9ON12_QUEUES];
	UINT NumQueues;
	UINT NodeMask;
} D3D9ON12_ARGS;

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

#define DECLARE_IN_WRAPPED_PROC(procName, unused) \
	const FARPROC procName ## _in = (FARPROC)*d9_ ## procName;

extern bool EnableWndProcHook;

namespace D3d9Wrapper
{
	VISIT_PROCS_D3D9(DECLARE_IN_WRAPPED_PROC);

	VISIT_PROCS_D3D9(EXPORT_OUT_WRAPPED_PROC);
}

#undef DECLARE_IN_WRAPPED_PROC
