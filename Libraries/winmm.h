#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifndef _TIMERAPI_H_
#define _TIMERAPI_H_
#endif

typedef _Return_type_success_(return == 0) UINT MMRESULT;   /* error return code, 0 means no error */

MMRESULT timeBeginPeriod(UINT);
MMRESULT timeEndPeriod(UINT);
DWORD timeGetTime();
