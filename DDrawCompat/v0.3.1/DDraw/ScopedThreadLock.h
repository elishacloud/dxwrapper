#pragma once

#include <DDrawCompat/v0.3.1/Dll/Dll.h>

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
