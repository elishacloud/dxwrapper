#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

bool OpenD3DDDI(HDC hDC);
bool CloseD3DDDI();
bool D3DDDIWaitForVsync();
bool D3DDDIGetVideoMemory(DWORD& TotalMemory, DWORD& AvailableMemory);
