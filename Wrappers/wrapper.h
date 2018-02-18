#pragma once

namespace ShardProcs
{
	extern "C" void __stdcall DllCanUnloadNow();
	extern "C" void __stdcall DllGetClassObject();
	extern "C" void __stdcall DebugSetMute();
	extern FARPROC DllCanUnloadNow_var;
	extern FARPROC DllGetClassObject_var;
	extern FARPROC DebugSetMute_var;
}
namespace ddraw
{
	extern "C" void __stdcall AcquireDDThreadLock();
	extern "C" void __stdcall CheckFullscreen();
	extern "C" void __stdcall CompleteCreateSysmemSurface();
	extern "C" void __stdcall D3DParseUnknownCommand();
	extern "C" void __stdcall DDGetAttachedSurfaceLcl();
	extern "C" void __stdcall DDInternalLock();
	extern "C" void __stdcall DDInternalUnlock();
	extern "C" void __stdcall DSoundHelp();
	extern "C" void __stdcall DirectDrawCreate();
	extern "C" void __stdcall DirectDrawCreateClipper();
	extern "C" void __stdcall DirectDrawCreateEx();
	extern "C" void __stdcall DirectDrawEnumerateA();
	extern "C" void __stdcall DirectDrawEnumerateExA();
	extern "C" void __stdcall DirectDrawEnumerateExW();
	extern "C" void __stdcall DirectDrawEnumerateW();
	extern "C" void __stdcall GetDDSurfaceLocal();
	extern "C" void __stdcall GetOLEThunkData();
	extern "C" void __stdcall GetSurfaceFromDC();
	extern "C" void __stdcall RegisterSpecialCase();
	extern "C" void __stdcall ReleaseDDThreadLock();
	extern "C" void __stdcall SetAppCompatData();
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
	extern "C" void __stdcall Direct3DShaderValidatorCreate9();
	extern "C" void __stdcall PSGPError();
	extern "C" void __stdcall PSGPSampleTexture();
	extern "C" void __stdcall D3DPERF_BeginEvent();
	extern "C" void __stdcall D3DPERF_EndEvent();
	extern "C" void __stdcall D3DPERF_GetStatus();
	extern "C" void __stdcall D3DPERF_QueryRepeatFrame();
	extern "C" void __stdcall D3DPERF_SetMarker();
	extern "C" void __stdcall D3DPERF_SetOptions();
	extern "C" void __stdcall D3DPERF_SetRegion();
	extern "C" void __stdcall DebugSetLevel();
	extern "C" void __stdcall Direct3D9EnableMaximizedWindowedModeShim();
	extern "C" void __stdcall Direct3DCreate9();
	extern "C" void __stdcall Direct3DCreate9Ex();
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
	extern "C" void __stdcall DirectSoundCreate();
	extern "C" void __stdcall DirectSoundCreate8();
	extern "C" void __stdcall GetDeviceID();
	extern "C" void __stdcall DirectSoundEnumerateA();
	extern "C" void __stdcall DirectSoundEnumerateW();
	extern "C" void __stdcall DirectSoundCaptureCreate();
	extern "C" void __stdcall DirectSoundCaptureEnumerateA();
	extern "C" void __stdcall DirectSoundCaptureEnumerateW();
	extern "C" void __stdcall DirectSoundCaptureCreate8();
	extern "C" void __stdcall DirectSoundFullDuplexCreate();
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
	// Shared Functions
	bool ValidProcAddress(FARPROC ProcAddress);
	void ShimProc(FARPROC &var, FARPROC in, FARPROC &out);
	HMODULE CreateWrapper(const char *ProxyDll, const char *WrapperMode);
}
