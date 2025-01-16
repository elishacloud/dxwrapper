#pragma once

#include "commdlg.h"

typedef BOOL(WINAPI* GetOpenFileNameAProc)(LPOPENFILENAMEA lpOpenFile);
typedef BOOL(WINAPI* GetOpenFileNameWProc)(LPOPENFILENAMEW lpOpenFile);
typedef BOOL(WINAPI* GetSaveFileNameAProc)(LPOPENFILENAMEA lpOpenFile);
typedef BOOL(WINAPI* GetSaveFileNameWProc)(LPOPENFILENAMEW lpOpenFile);

BOOL WINAPI comdlg_GetOpenFileNameA(LPOPENFILENAMEA lpOpenFile);
BOOL WINAPI comdlg_GetOpenFileNameW(LPOPENFILENAMEW lpOpenFile);
BOOL WINAPI comdlg_GetSaveFileNameA(LPOPENFILENAMEA lpOpenFile);
BOOL WINAPI comdlg_GetSaveFileNameW(LPOPENFILENAMEW lpOpenFile);

namespace GdiWrapper
{
	EXPORT_OUT_WRAPPED_PROC(GetOpenFileNameA, unused);
	EXPORT_OUT_WRAPPED_PROC(GetOpenFileNameW, unused);
	EXPORT_OUT_WRAPPED_PROC(GetSaveFileNameA, unused);
	EXPORT_OUT_WRAPPED_PROC(GetSaveFileNameW, unused);
}
