#pragma once

#include "DDrawProcs.h"

namespace Compat20
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
