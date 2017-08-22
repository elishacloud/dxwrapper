#pragma once

//#define D3D9LOGGING
//#undef D3D8TO9NOLOG
//#define DDRAWCOMPATLOG
//#define FULLSCREENLOG

#ifndef DDRAWLOG_H
#include "DDrawLog.h"
#endif

namespace Logging
{
	using namespace Compat;
	void LogFormat(char * fmt, ...);
	void LogOSVersion();
	void LogProcessNameAndPID();
}
