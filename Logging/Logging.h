#pragma once

//#undef D3D8TO9NOLOG
//#define DDRAWCOMPATLOG
//#define FULLSCREENLOG
//#define SETTINGSLOG

#ifndef DDRAWLOG_H
#include "DDrawCompat\DDrawLog.h"
#endif

namespace Compat
{
}

namespace Logging
{
	using namespace Compat;
	void LogFormat(char * fmt, ...);
	void LogOSVersion();
	void LogProcessNameAndPID();
	void LogComputerManufacturer();
	void LogVideoCard();
}
