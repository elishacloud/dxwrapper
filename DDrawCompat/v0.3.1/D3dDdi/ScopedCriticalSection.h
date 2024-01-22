#pragma once

#include <DDrawCompat/v0.3.1/Common/ScopedCriticalSection.h>

namespace D3dDdi
{
	class ScopedCriticalSection : public Compat32::ScopedCriticalSection
	{
	public:
		ScopedCriticalSection() : Compat32::ScopedCriticalSection(s_cs) {}

	private:
		static Compat32::CriticalSection s_cs;
	};
}
