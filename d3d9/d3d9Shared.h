#pragma once

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

IDirect3D9* WINAPI d9_Direct3DCreate9On12(UINT SDKVersion, D3D9ON12_ARGS* pOverrideList, UINT NumOverrideEntries);
HRESULT WINAPI d9_Direct3DCreate9On12Ex(UINT SDKVersion, D3D9ON12_ARGS* pOverrideList, UINT NumOverrideEntries, IDirect3D9Ex** ppOutputInterface);
