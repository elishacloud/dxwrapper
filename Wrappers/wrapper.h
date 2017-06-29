#pragma once

void CallReturn();
static constexpr FARPROC jmpaddr = (FARPROC)*CallReturn;

void DllAttach();
HMODULE LoadDll(DWORD);
void DllDetach();