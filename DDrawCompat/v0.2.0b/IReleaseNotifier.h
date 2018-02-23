#pragma once

#include <functional>

#include <Unknwnbase.h>

namespace Compat20
{
	// {7810158A-CB51-448A-8706-443A7DF6D4ED}
	DEFINE_GUID(IID_IReleaseNotifier,
		0x7810158a, 0xcb51, 0x448a, 0x87, 0x6, 0x44, 0x3a, 0x7d, 0xf6, 0xd4, 0xed);

	class IReleaseNotifier
	{
	public:
		IReleaseNotifier(const std::function<void()>& notifyHandler)
			: m_notifyHandler(notifyHandler)
		{
		}

		virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, LPVOID*)
		{
			return E_NOINTERFACE;
		}

		virtual ULONG STDMETHODCALLTYPE AddRef()
		{
			return 2;
		}

		virtual ULONG STDMETHODCALLTYPE Release()
		{
			m_notifyHandler();
			return 0;
		}

	private:
		std::function<void()> m_notifyHandler;
	};
}
