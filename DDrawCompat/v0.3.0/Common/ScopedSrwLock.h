#pragma once

#include <Windows.h>

namespace Compat30
{
	class SrwLock : public SRWLOCK
	{
	public:
		SrwLock() : SRWLOCK(SRWLOCK_INIT)
		{
		}
	};

	class ScopedSrwLockExclusive
	{
	public:
		ScopedSrwLockExclusive(SRWLOCK& lock) : m_lock(lock)
		{
			AcquireSRWLockExclusive(&m_lock);
		}

		~ScopedSrwLockExclusive()
		{
			ReleaseSRWLockExclusive(&m_lock);
		}

	private:
		SRWLOCK& m_lock;
	};

	class ScopedSrwLockShared
	{
	public:
		ScopedSrwLockShared(SRWLOCK& lock) : m_lock(lock)
		{
			AcquireSRWLockShared(&m_lock);
		}

		~ScopedSrwLockShared()
		{
			ReleaseSRWLockShared(&m_lock);
		}

	private:
		SRWLOCK& m_lock;
	};
}
