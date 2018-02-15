#pragma once

typedef DWORD D3DCOLOR;
struct __declspec(uuid("81BDCBCA-64D4-426d-AE8D-AD0147F4275C")) IDirect3D9;
struct __declspec(uuid("02177241-69FC-400C-8FF1-93A44DF6861D")) IDirect3D9Ex;

HRESULT WINAPI dd_Direct3DShaderValidatorCreate9();
HRESULT WINAPI dd_PSGPError();
HRESULT WINAPI dd_PSGPSampleTexture();
int WINAPI dd_D3DPERF_BeginEvent(D3DCOLOR col, LPCWSTR wszName);
int WINAPI dd_D3DPERF_EndEvent();
DWORD WINAPI dd_D3DPERF_GetStatus();
BOOL WINAPI dd_D3DPERF_QueryRepeatFrame();
void WINAPI dd_D3DPERF_SetMarker(D3DCOLOR col, LPCWSTR wszName);
void WINAPI dd_D3DPERF_SetOptions(DWORD dwOptions);
void WINAPI dd_D3DPERF_SetRegion(D3DCOLOR col, LPCWSTR wszName);
HRESULT WINAPI dd_DebugSetLevel(DWORD dw1);
void WINAPI dd_DebugSetMute();
void WINAPI dd_Direct3D9EnableMaximizedWindowedModeShim();
IDirect3D9 *WINAPI dd_Direct3DCreate9(UINT SDKVersion);
HRESULT WINAPI dd_Direct3DCreate9Ex(UINT SDKVersion, IDirect3D9Ex **ppD3D);

namespace D3d9Wrapper
{
	constexpr FARPROC Direct3DShaderValidatorCreate9_in = (FARPROC)*dd_Direct3DShaderValidatorCreate9;
	constexpr FARPROC PSGPError_in = (FARPROC)*dd_PSGPError;
	constexpr FARPROC PSGPSampleTexture_in = (FARPROC)*dd_PSGPSampleTexture;
	constexpr FARPROC D3DPERF_BeginEvent_in = (FARPROC)*dd_D3DPERF_BeginEvent;
	constexpr FARPROC D3DPERF_EndEvent_in = (FARPROC)*dd_D3DPERF_EndEvent;
	constexpr FARPROC D3DPERF_GetStatus_in = (FARPROC)*dd_D3DPERF_GetStatus;
	constexpr FARPROC D3DPERF_QueryRepeatFrame_in = (FARPROC)*dd_D3DPERF_QueryRepeatFrame;
	constexpr FARPROC D3DPERF_SetMarker_in = (FARPROC)*dd_D3DPERF_SetMarker;
	constexpr FARPROC D3DPERF_SetOptions_in = (FARPROC)*dd_D3DPERF_SetOptions;
	constexpr FARPROC D3DPERF_SetRegion_in = (FARPROC)*dd_D3DPERF_SetRegion;
	constexpr FARPROC DebugSetLevel_in = (FARPROC)*dd_DebugSetLevel;
	constexpr FARPROC DebugSetMute_in = (FARPROC)*dd_DebugSetMute;
	constexpr FARPROC Direct3D9EnableMaximizedWindowedModeShim_in = (FARPROC)*dd_Direct3D9EnableMaximizedWindowedModeShim;
	constexpr FARPROC Direct3DCreate9_in = (FARPROC)*dd_Direct3DCreate9;
	constexpr FARPROC Direct3DCreate9Ex_in = (FARPROC)*dd_Direct3DCreate9Ex;
	extern FARPROC Direct3DShaderValidatorCreate9_out;
	extern FARPROC PSGPError_out;
	extern FARPROC PSGPSampleTexture_out;
	extern FARPROC D3DPERF_BeginEvent_out;
	extern FARPROC D3DPERF_EndEvent_out;
	extern FARPROC D3DPERF_GetStatus_out;
	extern FARPROC D3DPERF_QueryRepeatFrame_out;
	extern FARPROC D3DPERF_SetMarker_out;
	extern FARPROC D3DPERF_SetOptions_out;
	extern FARPROC D3DPERF_SetRegion_out;
	extern FARPROC DebugSetLevel_out;
	extern FARPROC DebugSetMute_out;
	extern FARPROC Direct3D9EnableMaximizedWindowedModeShim_out;
	extern FARPROC Direct3DCreate9_out;
	extern FARPROC Direct3DCreate9Ex_out;
}
