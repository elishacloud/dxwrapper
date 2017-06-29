#pragma once

void LogOSVersion();
void LogProcessNameAndPID();
void Shell(char*);
void DisableHighDPIScaling();
void SetAppCompat();
FARPROC GetFunctionAddress(HMODULE, LPCSTR, FARPROC = nullptr);