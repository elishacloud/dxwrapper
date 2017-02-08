#pragma once

#include "DDrawProcs.h"

namespace Compat
{
	class DDrawScopedThreadLock
	{
	public:
		DDrawScopedThreadLock()
		{
			origProcs.AcquireDDThreadLock();
		}

		~DDrawScopedThreadLock()
		{
			origProcs.ReleaseDDThreadLock();
		}
	};
}
