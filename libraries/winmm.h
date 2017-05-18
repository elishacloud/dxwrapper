#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

void UnLoadwinmm();
void _timeBeginPeriod(UINT);
void _timeEndPeriod(UINT);