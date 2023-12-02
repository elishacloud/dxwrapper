#pragma once

typedef int(WINAPI* GetDeviceCapsProc)(HDC hdc, int index);

int WINAPI gdi_GetDeviceCaps(HDC hdc, int index);

namespace GdiWrapper
{
	EXPORT_OUT_WRAPPED_PROC(GetDeviceCaps, unused);
}
