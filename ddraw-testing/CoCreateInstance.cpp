#define INITGUID

#define DIRECTINPUT_VERSION 0x0700
#include <ddraw.h>
#include <ddrawex.h>
#include <dinput.h>
#include <dsound.h>

#include "ddraw-testing.h"
#include "testing-harness.h"

static void TestDirectDrawFactory(IDirectDrawFactory* pFactory)
{
	if (!pFactory)
	{
		Logging::LogFormat("Error: TestDirectDrawFactory: pFactory is NULL");
		return;
	}

	Logging::LogFormat("        TestDirectDrawFactory: Factory=%p", pFactory);

	IDirectDraw* pDirectDraw = nullptr;

	HRESULT hr = pFactory->CreateDirectDraw(
		nullptr,                 // GUID of driver, NULL = default
		DDhWnd,                  // hwnd for the device
		DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN, // Flags
		0,                       // Reserved
		nullptr,                 // IUnknown outer
		&pDirectDraw);           // IDirectDraw returned

	Logging::LogFormat(
		"        IDirectDrawFactory::CreateDirectDraw: hr=0x%08X, IDirectDraw=%p",
		(unsigned int)hr,
		pDirectDraw);

	if (SUCCEEDED(hr))
	{
		pDirectDraw->Release();
		Logging::Log() << "SUCCEEDED!!!!!!!!!!!!! IDirectDrawFactory(IID_DirectDraw)";
	}
	else
	{
		Logging::Log() << "FAILED!!!!!!!!!!!!! IDirectDrawFactory(IID_DirectDraw)";
	}
}

void CoCreateInstance()
{
	Logging::LogFormat("========== DirectDraw COM Test ==========");

	HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

	if (FAILED(hr) && hr != RPC_E_CHANGED_MODE)
	{
		Logging::LogFormat("CoInitializeEx failed: 0x%08X", (unsigned int)hr);
		return;
	}

	for (const auto& test : tests)
	{
		Logging::LogFormat("");
		Logging::LogFormat("----- %s -----", test.name);

		//
		// Test CoGetClassObject()
		//

		IClassFactory* pClassFactory = nullptr;

		Logging::LogFormat("        Calling CoGetClassObject(%s)", test.name);

		hr = CoGetClassObject(
			test.clsid,
			CLSCTX_INPROC_SERVER,
			nullptr,
			IID_IClassFactory,
			(void**)&pClassFactory);

		Logging::LogFormat(
			"        CoGetClassObject(%s) -> hr=0x%08X, factory=%p",
			test.name,
			(unsigned int)hr,
			pClassFactory);

		if (SUCCEEDED(hr))
		{
			//
			// Use the class factory just like a normal COM client.
			//

			void* pObject = nullptr;

			Logging::LogFormat(
				"        Calling IClassFactory::CreateInstance(%s -> %s)",
				test.name,
				GetTestIIDName(test.iid));

			hr = pClassFactory->CreateInstance(
				nullptr,
				test.iid,
				&pObject);

			Logging::LogFormat(
				"        CreateInstance(%s -> %s) -> hr=0x%08X, object=%p",
				test.name,
				GetTestIIDName(test.iid),
				(unsigned int)hr,
				pObject);

			if (SUCCEEDED(hr))
			{
				IUnknown* pUnknown = (IUnknown*)pObject;

				if (test.iid == IID_IDirectDrawFactory)
				{
					TestDirectDrawFactory((IDirectDrawFactory*)pObject);
				}

				pUnknown->Release();
			}

			pClassFactory->Release();
		}

		if (SUCCEEDED(hr))
		{
			Logging::Log() << "SUCCEEDED!!!!!!!!!!!!! CoGetClassObject(" << GetTestIIDName(test.iid) << ")";
		}
		else
		{
			Logging::Log() << "FAILED!!!!!!!!!!!!! CoGetClassObject(" << GetTestIIDName(test.iid) << ")";
		}

		//
		// Test CoCreateInstance()
		//

		void* pObject = nullptr;

		Logging::LogFormat(
			"        Calling CoCreateInstance(%s -> %s)",
			test.name,
			GetTestIIDName(test.iid));

		hr = CoCreateInstance(
			test.clsid,
			nullptr,
			CLSCTX_INPROC_SERVER,
			test.iid,
			&pObject);

		Logging::LogFormat(
			"        CoCreateInstance(%s -> %s) -> hr=0x%08X, object=%p",
			test.name,
			GetTestIIDName(test.iid),
			(unsigned int)hr,
			pObject);

		if (SUCCEEDED(hr))
		{
			IUnknown* pUnknown = (IUnknown*)pObject;

			if (test.iid == IID_IDirectDrawFactory)
			{
				TestDirectDrawFactory((IDirectDrawFactory*)pObject);
			}

			pUnknown->Release();
		}

		if (SUCCEEDED(hr))
		{
			Logging::Log() << "SUCCEEDED!!!!!!!!!!!!! CoCreateInstance(" << GetTestIIDName(test.iid) << ")";
		}
		else
		{
			Logging::Log() << "FAILED!!!!!!!!!!!!! CoCreateInstance(" << GetTestIIDName(test.iid) << ")";
		}
	}

	if (hr != RPC_E_CHANGED_MODE)
	{
		CoUninitialize();
	}

	Logging::LogFormat("");
	Logging::LogFormat("========== DirectDraw COM Test Complete ==========");
}
