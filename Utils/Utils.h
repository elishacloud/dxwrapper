#pragma once

int filterException(int, PEXCEPTION_POINTERS);
void GetOSVersion();
void GetProcessNameAndPID();
void Shell(char*);
void DisableHighDPIScaling();
void SetAppCompat();
DWORD GetMyThreadId(HANDLE);
FARPROC GetFunctionAddress(HMODULE, LPCSTR, FARPROC = nullptr);