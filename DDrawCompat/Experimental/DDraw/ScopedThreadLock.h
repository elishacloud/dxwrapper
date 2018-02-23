#pragma once

#include "Dll/Procs.h"

namespace DDraw
{
	class ScopedThreadLock
	{
	public:
		ScopedThreadLock()
		{
			Dll::g_origProcs.AcquireDDThreadLock();
		}

		~ScopedThreadLock()
		{
			Dll::g_origProcs.ReleaseDDThreadLock();
		}
	};
}
