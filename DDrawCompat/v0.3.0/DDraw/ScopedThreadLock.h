#pragma once

#include <DDrawCompat/v0.3.0/Dll/Dll.h>

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
