#pragma once

#ifndef DDRAWLOG_H
#include "DDrawLog.h"
#endif

namespace Logging
{
	using namespace Compat;
	void LogText(char*);
	void LogOSVersion();
	void LogProcessNameAndPID();
}
