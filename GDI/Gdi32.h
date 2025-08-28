#pragma once

typedef int(WINAPI* GetDeviceCapsProc)(HDC hdc, int index);
typedef BOOL(WINAPI* SetDeviceGammaRampProc)(HDC hdc, LPVOID lpRamp);

int WINAPI gdi_GetDeviceCaps(HDC hdc, int index);
BOOL WINAPI gdi_SetDeviceGammaRamp(HDC hdc, LPVOID lpRamp);

namespace GdiWrapper
{
	EXPORT_OUT_WRAPPED_PROC(GetDeviceCaps, unused);
	EXPORT_OUT_WRAPPED_PROC(SetDeviceGammaRamp, unused);
}
