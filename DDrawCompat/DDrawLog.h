#pragma once

#define CINTERFACE

#include <ddraw.h>
// Debug mode: disable DDrawCompat debug logs
#ifdef _DEBUG
#define LogDebug LogNothing
#define LogEnter LogNothingEnter
#define LogLeave LogNothingLeave
#include "Logging\Logging.h"
#undef LogDebug
#undef LogEnter
#undef LogLeave
namespace Logging
{
	class LogNull
	{
	public:
		template <typename T> LogNull& operator<<(const T&) { return *this; }
	};

	typedef LogNull LogDebug;

	class LogEnter : public LogNull
	{
	public:
		template <typename... Params> LogEnter(const char*, Params...) {}
	};

	typedef LogEnter LogLeave;
}
#ifdef HOOK_FUNCTION
#undef _DEBUG
#endif

// Release mode: normal logging
#else
#include "Logging\Logging.h"
#endif

namespace Compat
{
	using namespace Logging;
}

namespace Compat20
{
	namespace Compat
	{
		using namespace Logging;
	}
}

namespace Compat21
{
	namespace Compat
	{
		using namespace Logging;
	}
}

namespace Compat32
{
	using namespace Logging;
}
