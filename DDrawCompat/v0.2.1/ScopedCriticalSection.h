#pragma once

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

namespace Compat
{
	class ScopedCriticalSection
	{
	public:
		ScopedCriticalSection(CRITICAL_SECTION& cs)
			: m_cs(cs), m_isLocked(true)
		{
			EnterCriticalSection(&m_cs);
		}

		~ScopedCriticalSection()
		{
			unlock();
		}

		void unlock()
		{
			if (m_isLocked)
			{
				LeaveCriticalSection(&m_cs);
				m_isLocked = false;
			}
		}

	private:
		CRITICAL_SECTION& m_cs;
		bool m_isLocked;
	};
};
