#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <Common/CompatVtable.h>
#include <D3dDdi/Adapter.h>
#include <D3dDdi/AdapterFuncs.h>
#include <D3dDdi/Device.h>
#include <D3dDdi/DeviceCallbacks.h>
#include <D3dDdi/DeviceFuncs.h>

namespace
{
	std::string bitDepthsToString(DWORD bitDepths)
	{
		std::string result;
		if (bitDepths & DDBD_8) { result += ", 8"; }
		if (bitDepths & DDBD_16) { result += ", 16"; }
		if (bitDepths & DDBD_24) { result += ", 24"; }
		if (bitDepths & DDBD_32) { result += ", 32"; }

		if (result.empty())
		{
			return "\"\"";
		}
		return '"' + result.substr(2) + '"';
	}
}

namespace D3dDdi
{
	Adapter::Adapter(const D3DDDIARG_OPENADAPTER& data)
		: m_adapter(data.hAdapter)
		, m_origVtable(CompatVtable<D3DDDI_ADAPTERFUNCS>::s_origVtable)
		, m_runtimeVersion(data.Version)
		, m_driverVersion(data.DriverVersion)
		, m_d3dExtendedCaps{}
	{
		if (m_adapter)
		{
			D3DDDIARG_GETCAPS getCaps = {};
			getCaps.Type = D3DDDICAPS_GETD3D7CAPS;
			getCaps.pData = &m_d3dExtendedCaps;
			getCaps.DataSize = sizeof(m_d3dExtendedCaps);
			m_origVtable.pfnGetCaps(m_adapter, &getCaps);

			getCaps.Type = D3DDDICAPS_DDRAW;
			getCaps.pData = &m_ddrawCaps;
			getCaps.DataSize = sizeof(m_ddrawCaps);
			m_origVtable.pfnGetCaps(m_adapter, &getCaps);
		}
	}

	DWORD Adapter::getSupportedZBufferBitDepths()
	{
		UINT formatCount = 0;
		D3DDDIARG_GETCAPS caps = {};
		caps.Type = D3DDDICAPS_GETFORMATCOUNT;
		caps.pData = &formatCount;
		caps.DataSize = sizeof(formatCount);
		m_origVtable.pfnGetCaps(m_adapter, &caps);

		std::vector<FORMATOP> formatOp(formatCount);
		caps.Type = D3DDDICAPS_GETFORMATDATA;
		caps.pData = formatOp.data();
		caps.DataSize = formatCount * sizeof(FORMATOP);
		m_origVtable.pfnGetCaps(m_adapter, &caps);

		DWORD supportedZBufferBitDepths = 0;
		for (UINT i = 0; i < formatCount; ++i)
		{
			if (formatOp[i].Operations & (FORMATOP_ZSTENCIL | FORMATOP_ZSTENCIL_WITH_ARBITRARY_COLOR_DEPTH))
			{
				switch (formatOp[i].Format)
				{
				case D3DDDIFMT_D16:
					supportedZBufferBitDepths |= DDBD_16;
					break;

				case D3DDDIFMT_X8D24:
					supportedZBufferBitDepths |= DDBD_24;
					break;

				case D3DDDIFMT_D32:
					supportedZBufferBitDepths |= DDBD_32;
					break;
				}
			}
		}

		return supportedZBufferBitDepths;
	}

	HRESULT Adapter::pfnCloseAdapter()
	{
		auto adapter = m_adapter;
		auto pfnCloseAdapter = m_origVtable.pfnCloseAdapter;
		s_adapters.erase(adapter);
		return pfnCloseAdapter(adapter);
	}

	HRESULT Adapter::pfnCreateDevice(D3DDDIARG_CREATEDEVICE* pCreateData)
	{
		DeviceCallbacks::hookVtable(*pCreateData->pCallbacks, m_runtimeVersion);
		HRESULT result = m_origVtable.pfnCreateDevice(m_adapter, pCreateData);
		if (SUCCEEDED(result))
		{
			DeviceFuncs::hookVtable(*pCreateData->pDeviceFuncs, m_driverVersion);
			Device::add(*this, pCreateData->hDevice);
		}
		return result;
	}

	HRESULT Adapter::pfnGetCaps(const D3DDDIARG_GETCAPS* pData)
	{
		HRESULT result = m_origVtable.pfnGetCaps(m_adapter, pData);
		if (FAILED(result))
		{
			return result;
		}

		switch (pData->Type)
		{
		case D3DDDICAPS_DDRAW:
			static_cast<DDRAW_CAPS*>(pData->pData)->FxCaps =
				DDRAW_FXCAPS_BLTMIRRORLEFTRIGHT | DDRAW_FXCAPS_BLTMIRRORUPDOWN;
			break;

		case D3DDDICAPS_GETD3D3CAPS:
		{
			auto& caps = static_cast<D3DNTHAL_GLOBALDRIVERDATA*>(pData->pData)->hwCaps;
			if (caps.dwFlags & D3DDD_DEVICEZBUFFERBITDEPTH)
			{
				const DWORD supportedZBufferBitDepths = getSupportedZBufferBitDepths();
				if (supportedZBufferBitDepths != caps.dwDeviceZBufferBitDepth)
				{
					LOG_ONCE("Incorrect z-buffer bit depth capabilities detected; changed from "
						<< bitDepthsToString(caps.dwDeviceZBufferBitDepth) << " to "
						<< bitDepthsToString(supportedZBufferBitDepths));
					caps.dwDeviceZBufferBitDepth = supportedZBufferBitDepths;
				}
			}
			break;
		}
		}

		return result;
	}

	std::map<HANDLE, Adapter> Adapter::s_adapters;
}
