#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <DDrawCompat/v0.3.1/D3dDdi/ScopedCriticalSection.h>

namespace D3dDdi
{
	Compat32::CriticalSection ScopedCriticalSection::s_cs;
}
