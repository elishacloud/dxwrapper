#pragma once

#include <DDrawCompat/v0.3.0/Common/ScopedCriticalSection.h>

namespace D3dDdi
{
	class ScopedCriticalSection : public Compat30::ScopedCriticalSection
	{
	public:
		ScopedCriticalSection() : Compat30::ScopedCriticalSection(s_cs) {}

	private:
		static Compat30::CriticalSection s_cs;
	};
}
