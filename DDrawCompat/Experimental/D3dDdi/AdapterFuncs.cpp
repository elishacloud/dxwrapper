#include <map>

#include "D3dDdi/AdapterFuncs.h"
#include "D3dDdi/DeviceCallbacks.h"
#include "D3dDdi/DeviceFuncs.h"

namespace
{
	std::map<HANDLE, D3DNTHAL_D3DEXTENDEDCAPS> g_d3dExtendedCaps;

	HRESULT APIENTRY closeAdapter(HANDLE hAdapter)
	{
		HRESULT result = D3dDdi::AdapterFuncs::s_origVtables.at(hAdapter).pfnCloseAdapter(hAdapter);
		if (SUCCEEDED(result))
		{
			D3dDdi::AdapterFuncs::s_origVtables.erase(hAdapter);
			g_d3dExtendedCaps.erase(hAdapter);
		}
		return result;
	}

	HRESULT APIENTRY createDevice(HANDLE hAdapter, D3DDDIARG_CREATEDEVICE* pCreateData)
	{
		D3dDdi::DeviceCallbacks::hookVtable(pCreateData->pCallbacks);
		HRESULT result = D3dDdi::AdapterFuncs::s_origVtables.at(hAdapter).pfnCreateDevice(
			hAdapter, pCreateData);
		if (SUCCEEDED(result))
		{
			D3dDdi::DeviceFuncs::hookDriverVtable(pCreateData->hDevice, pCreateData->pDeviceFuncs);
			D3dDdi::DeviceFuncs::onCreateDevice(hAdapter, pCreateData->hDevice);
		}
		return result;
	}

	HRESULT APIENTRY getCaps(HANDLE hAdapter, const D3DDDIARG_GETCAPS* pData)
	{
		HRESULT result = D3dDdi::AdapterFuncs::s_origVtables.at(hAdapter).pfnGetCaps(hAdapter, pData);
		if (SUCCEEDED(result) && D3DDDICAPS_DDRAW == pData->Type)
		{
			static_cast<DDRAW_CAPS*>(pData->pData)->FxCaps = 0;
		}
		return result;
	}
}

namespace D3dDdi
{
	const D3DNTHAL_D3DEXTENDEDCAPS& AdapterFuncs::getD3dExtendedCaps(HANDLE adapter)
	{
		static D3DNTHAL_D3DEXTENDEDCAPS emptyCaps = {};
		auto it = g_d3dExtendedCaps.find(adapter);
		return it != g_d3dExtendedCaps.end() ? it->second : emptyCaps;
	}

	void AdapterFuncs::onOpenAdapter(HANDLE adapter)
	{
		D3DNTHAL_D3DEXTENDEDCAPS d3dExtendedCaps = {};
		D3DDDIARG_GETCAPS getCaps = {};
		getCaps.Type = D3DDDICAPS_GETD3D7CAPS;
		getCaps.pData = &d3dExtendedCaps;
		getCaps.DataSize = sizeof(d3dExtendedCaps);

		D3dDdi::AdapterFuncs::s_origVtables.at(adapter).pfnGetCaps(adapter, &getCaps);
		g_d3dExtendedCaps[adapter] = d3dExtendedCaps;
	}

	void AdapterFuncs::setCompatVtable(D3DDDI_ADAPTERFUNCS& vtable)
	{
		vtable.pfnCloseAdapter = &closeAdapter;
		vtable.pfnCreateDevice = &createDevice;
		vtable.pfnGetCaps = &getCaps;
	}
}
