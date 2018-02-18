#pragma once

#include <vector>

namespace ShardProcs
{
	extern FARPROC DllCanUnloadNow_var;
	extern FARPROC DllGetClassObject_var;
	extern FARPROC DebugSetMute_var;
}
namespace ddraw
{
	extern FARPROC AcquireDDThreadLock_var;
	extern FARPROC CheckFullscreen_var;
	extern FARPROC CompleteCreateSysmemSurface_var;
	extern FARPROC D3DParseUnknownCommand_var;
	extern FARPROC DDGetAttachedSurfaceLcl_var;
	extern FARPROC DDInternalLock_var;
	extern FARPROC DDInternalUnlock_var;
	extern FARPROC DSoundHelp_var;
	extern FARPROC DirectDrawCreate_var;
	extern FARPROC DirectDrawCreateClipper_var;
	extern FARPROC DirectDrawCreateEx_var;
	extern FARPROC DirectDrawEnumerateA_var;
	extern FARPROC DirectDrawEnumerateExA_var;
	extern FARPROC DirectDrawEnumerateExW_var;
	extern FARPROC DirectDrawEnumerateW_var;
	extern FARPROC GetDDSurfaceLocal_var;
	extern FARPROC GetOLEThunkData_var;
	extern FARPROC GetSurfaceFromDC_var;
	extern FARPROC RegisterSpecialCase_var;
	extern FARPROC ReleaseDDThreadLock_var;
	extern FARPROC SetAppCompatData_var;
	HMODULE Load(const char *strName);
}
namespace d3d8
{
	extern FARPROC Direct3D8EnableMaximizedWindowedModeShim_var;
	extern FARPROC Direct3DCreate8_var;
	HMODULE Load(const char *strName);
}
namespace d3d9
{
	extern FARPROC Direct3DShaderValidatorCreate9_var;
	extern FARPROC PSGPError_var;
	extern FARPROC PSGPSampleTexture_var;
	extern FARPROC D3DPERF_BeginEvent_var;
	extern FARPROC D3DPERF_EndEvent_var;
	extern FARPROC D3DPERF_GetStatus_var;
	extern FARPROC D3DPERF_QueryRepeatFrame_var;
	extern FARPROC D3DPERF_SetMarker_var;
	extern FARPROC D3DPERF_SetOptions_var;
	extern FARPROC D3DPERF_SetRegion_var;
	extern FARPROC DebugSetLevel_var;
	extern FARPROC Direct3D9EnableMaximizedWindowedModeShim_var;
	extern FARPROC Direct3DCreate9_var;
	extern FARPROC Direct3DCreate9Ex_var;
	HMODULE Load(const char *strName);
}
namespace dsound
{
	extern FARPROC DirectSoundCreate_var;
	extern FARPROC DirectSoundCreate8_var;
	extern FARPROC GetDeviceID_var;
	extern FARPROC DirectSoundEnumerateA_var;
	extern FARPROC DirectSoundEnumerateW_var;
	extern FARPROC DirectSoundCaptureCreate_var;
	extern FARPROC DirectSoundCaptureEnumerateA_var;
	extern FARPROC DirectSoundCaptureEnumerateW_var;
	extern FARPROC DirectSoundCaptureCreate8_var;
	extern FARPROC DirectSoundFullDuplexCreate_var;
	extern FARPROC DllGetClassObject_var;
	extern FARPROC DllCanUnloadNow_var;
	HMODULE Load(const char *strName);
}
namespace Wrapper
{
	bool ValidProcAddress(FARPROC ProcAddress);
	void ShimProc(FARPROC &var, FARPROC in, FARPROC &out);
	HMODULE CreateWrapper(const char *ProxyDll, const char *WrapperMode);
}
