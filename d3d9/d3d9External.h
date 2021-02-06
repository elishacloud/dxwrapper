#pragma once

#include "Wrappers\wrapper.h"

typedef DWORD D3DCOLOR;
struct __declspec(uuid("81BDCBCA-64D4-426d-AE8D-AD0147F4275C")) IDirect3D9;
struct __declspec(uuid("02177241-69FC-400C-8FF1-93A44DF6861D")) IDirect3D9Ex;

int WINAPI d9_D3DPERF_BeginEvent(D3DCOLOR col, LPCWSTR wszName);
int WINAPI d9_D3DPERF_EndEvent();
DWORD WINAPI d9_D3DPERF_GetStatus();
BOOL WINAPI d9_D3DPERF_QueryRepeatFrame();
void WINAPI d9_D3DPERF_SetMarker(D3DCOLOR col, LPCWSTR wszName);
void WINAPI d9_D3DPERF_SetOptions(DWORD dwOptions);
void WINAPI d9_D3DPERF_SetRegion(D3DCOLOR col, LPCWSTR wszName);
IDirect3D9 *WINAPI d9_Direct3DCreate9(UINT SDKVersion);
HRESULT WINAPI d9_Direct3DCreate9Ex(UINT SDKVersion, IDirect3D9Ex **ppD3D);

#define DECLARE_IN_WRAPPED_PROC(procName, unused) \
	const FARPROC procName ## _in = (FARPROC)*d9_ ## procName;

#define EXPORT_OUT_WRAPPED_PROC(procName, unused) \
	extern FARPROC procName ## _out;

namespace D3d9Wrapper
{
	VISIT_PROCS_D3D9(DECLARE_IN_WRAPPED_PROC);

	VISIT_PROCS_D3D9(EXPORT_OUT_WRAPPED_PROC);
}

#undef DECLARE_IN_WRAPPED_PROC
#undef EXPORT_OUT_WRAPPED_PROC
