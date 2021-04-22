#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <set>

#include <Common/Hook.h>
#include <../DDrawLog.h>
#include <D3dDdi/Adapter.h>
#include <D3dDdi/AdapterCallbacks.h>
#include <D3dDdi/AdapterFuncs.h>
#include <D3dDdi/Hooks.h>
#include <D3dDdi/KernelModeThunks.h>
#include <D3dDdi/ScopedCriticalSection.h>
#include <Dll/Dll.h>

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_OPENADAPTER& data)
{
	return Compat30::LogStruct(os)
		<< data.hAdapter
		<< data.Interface
		<< data.Version
		<< data.pAdapterCallbacks
		<< Compat30::out(data.pAdapterFuncs)
		<< data.DriverVersion;
}

namespace
{
	PFND3DDDI_OPENADAPTER g_origOpenAdapter = nullptr;

	HRESULT APIENTRY openAdapter(D3DDDIARG_OPENADAPTER* pOpenData);

	FARPROC WINAPI getProcAddress(HMODULE hModule, LPCSTR lpProcName)
	{
		LOG_FUNC("GetProcAddress", hModule, lpProcName);
		if (lpProcName && std::string(lpProcName) == "OpenAdapter")
		{
			g_origOpenAdapter = reinterpret_cast<PFND3DDDI_OPENADAPTER>(CALL_ORIG_FUNC(GetProcAddress)(hModule, lpProcName));
			if (g_origOpenAdapter)
			{
				static std::set<HMODULE> hookedModules;
				if (hookedModules.find(hModule) == hookedModules.end())
				{
					Compat30::Log() << "Hooking user mode display driver: " << Compat30::funcPtrToStr(g_origOpenAdapter);
					Dll::pinModule(hModule);
					hookedModules.insert(hModule);
				}
				return reinterpret_cast<FARPROC>(&openAdapter);
			}
		}
		return LOG_RESULT(CALL_ORIG_FUNC(GetProcAddress)(hModule, lpProcName));
	}

	HRESULT APIENTRY openAdapter(D3DDDIARG_OPENADAPTER* pOpenData)
	{
		LOG_FUNC("openAdapter", pOpenData);
		D3dDdi::ScopedCriticalSection lock;
		D3dDdi::AdapterCallbacks::hookVtable(*pOpenData->pAdapterCallbacks, pOpenData->Version);
		HRESULT result = g_origOpenAdapter(pOpenData);
		if (SUCCEEDED(result))
		{
			D3dDdi::AdapterFuncs::hookVtable(*pOpenData->pAdapterFuncs, pOpenData->DriverVersion);
			D3dDdi::Adapter::add(*pOpenData);
		}
		return LOG_RESULT(result);
	}
}

namespace D3dDdi
{
	void installHooks()
	{
		Compat30::hookIatFunction(Dll::g_origDDrawModule, "GetProcAddress", getProcAddress);

		KernelModeThunks::installHooks();
	}
}
