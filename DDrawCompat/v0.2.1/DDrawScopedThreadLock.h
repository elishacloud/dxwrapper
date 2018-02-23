#pragma once

#include "DDrawProcs.h"

namespace Compat21
{
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
}
