#pragma once

#include <map>

#include <d3d.h>
#include <d3dnthal.h>
#include <d3dumddi.h>

namespace D3dDdi
{
	class Adapter
	{
	public:
		Adapter(const D3DDDIARG_OPENADAPTER& data);
		Adapter(const Adapter&) = delete;
		Adapter(Adapter&&) = delete;
		Adapter& operator=(const Adapter&) = delete;
		Adapter& operator=(Adapter&&) = delete;

		operator HANDLE() const { return m_adapter; }

		const D3DNTHAL_D3DEXTENDEDCAPS& getD3dExtendedCaps() const { return m_d3dExtendedCaps; }
		const DDRAW_CAPS& getDDrawCaps() const { return m_ddrawCaps; }
		const D3DDDI_ADAPTERFUNCS& getOrigVtable() const { return m_origVtable; }

		HRESULT pfnCloseAdapter();
		HRESULT pfnCreateDevice(D3DDDIARG_CREATEDEVICE* pCreateData);
		HRESULT pfnGetCaps(const D3DDDIARG_GETCAPS* pData);

		static void add(const D3DDDIARG_OPENADAPTER& data) { s_adapters.emplace(data.hAdapter, data); }
		static Adapter& get(HANDLE adapter) { return s_adapters.find(adapter)->second; }

	private:
		DWORD getSupportedZBufferBitDepths();

		HANDLE m_adapter;
		D3DDDI_ADAPTERFUNCS m_origVtable;
		UINT m_runtimeVersion;
		UINT m_driverVersion;

		D3DNTHAL_D3DEXTENDEDCAPS m_d3dExtendedCaps;
		DDRAW_CAPS m_ddrawCaps;

		static std::map<HANDLE, Adapter> s_adapters;
	};
}
