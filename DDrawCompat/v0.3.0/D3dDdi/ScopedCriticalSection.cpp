#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <D3dDdi/ScopedCriticalSection.h>

namespace D3dDdi
{
	Compat30::CriticalSection ScopedCriticalSection::s_cs;
}
