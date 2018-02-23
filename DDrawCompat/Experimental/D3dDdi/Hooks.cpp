#define CINTERFACE

#include <set>

#include <Windows.h>
#include <d3d.h>
#include <d3dumddi.h>
#include <..\km\d3dkmthk.h>

#include "Common/Hook.h"
#include "DDrawLog.h"
#include "D3dDdi/AdapterCallbacks.h"
#include "D3dDdi/AdapterFuncs.h"
#include "D3dDdi/KernelModeThunks.h"

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_OPENADAPTER& data)
{
	return Compat::LogStruct(os)
		<< data.hAdapter
		<< data.Interface
		<< data.Version
		<< data.pAdapterCallbacks
		<< Compat::out(data.pAdapterFuncs)
		<< data.DriverVersion;
}

namespace
{
	UINT g_ddiVersion = 0;
	std::wstring g_hookedUmdFileName;
	PFND3DDDI_OPENADAPTER g_origOpenAdapter = nullptr;

	void hookOpenAdapter(const std::wstring& umdFileName);
	HRESULT APIENTRY openAdapter(D3DDDIARG_OPENADAPTER* pOpenData);
	void unhookOpenAdapter();

	void hookOpenAdapter(const std::wstring& umdFileName)
	{
		g_hookedUmdFileName = umdFileName;
		HMODULE module = LoadLibraryW(umdFileName.c_str());
		if (module)
		{
			Compat::hookFunction(module, "OpenAdapter",
				reinterpret_cast<void*&>(g_origOpenAdapter), &openAdapter);
			FreeLibrary(module);
		}
	}

	HRESULT APIENTRY openAdapter(D3DDDIARG_OPENADAPTER* pOpenData)
	{
		Compat::LogEnter("openAdapter", pOpenData);
		D3dDdi::AdapterCallbacks::hookVtable(pOpenData->pAdapterCallbacks);
		HRESULT result = g_origOpenAdapter(pOpenData);
		if (SUCCEEDED(result))
		{
			static std::set<std::wstring> hookedUmdFileNames;
			if (hookedUmdFileNames.find(g_hookedUmdFileName) == hookedUmdFileNames.end())
			{
				Compat::Log() << "Hooking user mode display driver: " << g_hookedUmdFileName.c_str();
				hookedUmdFileNames.insert(g_hookedUmdFileName);
			}
			g_ddiVersion = min(pOpenData->Version, pOpenData->DriverVersion);
			D3dDdi::AdapterFuncs::hookDriverVtable(pOpenData->hAdapter, pOpenData->pAdapterFuncs);
			D3dDdi::AdapterFuncs::onOpenAdapter(pOpenData->hAdapter);
		}
		Compat::LogLeave("openAdapter", pOpenData) << result;
		return result;
	}

	void unhookOpenAdapter()
	{
		if (g_origOpenAdapter)
		{
			Compat::unhookFunction(g_origOpenAdapter);
			g_hookedUmdFileName.clear();
		}
	}
}

namespace D3dDdi
{
	UINT getDdiVersion()
	{
		return g_ddiVersion;
	}

	void installHooks()
	{
		KernelModeThunks::installHooks();
	}

	void onUmdFileNameQueried(const std::wstring& umdFileName)
	{
		if (g_hookedUmdFileName != umdFileName)
		{
			unhookOpenAdapter();
			hookOpenAdapter(umdFileName);
		}
	}

	void uninstallHooks()
	{
		unhookOpenAdapter();
	}
}
