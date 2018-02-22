#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef _Return_type_success_(return == 0) UINT MMRESULT;   /* error return code, 0 means no error */

MMRESULT timeBeginPeriod(UINT);
MMRESULT timeEndPeriod(UINT);
