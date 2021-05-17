#pragma once

#include <DDrawCompat/v0.3.1/Common/ScopedCriticalSection.h>

namespace D3dDdi
{
	class ScopedCriticalSection : public Compat31::ScopedCriticalSection
	{
	public:
		ScopedCriticalSection() : Compat31::ScopedCriticalSection(s_cs) {}

	private:
		static Compat31::CriticalSection s_cs;
	};
}
