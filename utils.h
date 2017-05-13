#pragma once

#include "writememory\writememory.h"
#include "disasm\initdisasm.h"
#include "fullscreen.h"

// utils.cpp
void GetOSVersion();
void GetProcessNameAndPID();
void Shell(char*);
void DisableHighDPIScaling();
void SetSingleProcessAffinity(bool);
void SetSingleCoreAffinity();

// ddrawcompat (dllmaincompat.cpp)
bool StartDdrawCompat(HINSTANCE);
void UnloadDdrawCompat();

// dxwnd (init.cpp)
void InitDxWnd();
void DxWndEndHook();