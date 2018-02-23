#include "D3dDdi/AdapterFuncs.h"
#include "D3dDdi/DeviceFuncs.h"
#include "D3dDdi/OversizedResource.h"

namespace
{
	UINT getBytesPerPixel(D3DDDIFORMAT format)
	{
		switch (format)
		{
		case D3DDDIFMT_R5G6B5:
		case D3DDDIFMT_X1R5G5B5:
		case D3DDDIFMT_A1R5G5B5:
			return 2;

		case D3DDDIFMT_R8G8B8:
			return 3;

		case D3DDDIFMT_A8R8G8B8:
		case D3DDDIFMT_X8R8G8B8:
		case D3DDDIFMT_A8B8G8R8:
		case D3DDDIFMT_X8B8G8R8:
			return 4;

		default:
			return 0;
		}
	}
}

namespace D3dDdi
{
	OversizedResource::OversizedResource()
		: m_adapter(nullptr)
		, m_device(nullptr)
		, m_format(D3DDDIFMT_UNKNOWN)
		, m_surfaceInfo()
	{
	}

	OversizedResource::OversizedResource(
		HANDLE adapter, HANDLE device, D3DDDIFORMAT format, const D3DDDI_SURFACEINFO& surfaceInfo)
		: m_adapter(adapter)
		, m_device(device)
		, m_format(format)
		, m_surfaceInfo(surfaceInfo)
	{
	}

	HRESULT OversizedResource::blt(D3DDDIARG_BLT& data, HANDLE& resource, RECT& rect)
	{
		const auto& caps = D3dDdi::AdapterFuncs::getD3dExtendedCaps(m_adapter);
		if (rect.right <= static_cast<LONG>(caps.dwMaxTextureWidth) &&
			rect.bottom <= static_cast<LONG>(caps.dwMaxTextureHeight))
		{
			return D3dDdi::DeviceFuncs::s_origVtables.at(m_device).pfnBlt(m_device, &data);
		}

		HANDLE origResource = resource;
		RECT origRect = rect;
		HANDLE bltResource = createBltResource(rect);

		if (bltResource)
		{
			resource = bltResource;
			rect = RECT{ 0, 0, rect.right - rect.left, rect.bottom - rect.top };
		}

		const auto& deviceFuncs = D3dDdi::DeviceFuncs::s_origVtables.at(m_device);
		HRESULT result = deviceFuncs.pfnBlt(m_device, &data);

		if (bltResource)
		{
			resource = origResource;
			rect = origRect;
			deviceFuncs.pfnDestroyResource(m_device, bltResource);
		}

		return result;
	}

	HRESULT OversizedResource::bltFrom(D3DDDIARG_BLT data)
	{
		return blt(data, data.hSrcResource, data.SrcRect);
	}

	HRESULT OversizedResource::bltTo(D3DDDIARG_BLT data)
	{
		return blt(data, data.hDstResource, data.DstRect);
	}

	HANDLE OversizedResource::createBltResource(RECT bltRect)
	{
		const RECT surfaceRect = {
			0, 0, static_cast<LONG>(m_surfaceInfo.Width), static_cast<LONG>(m_surfaceInfo.Height) };
		IntersectRect(&bltRect, &surfaceRect, &bltRect);

		D3DDDI_SURFACEINFO bltSurfaceInfo = {};
		bltSurfaceInfo.Width = bltRect.right - bltRect.left;
		bltSurfaceInfo.Height = bltRect.bottom - bltRect.top;
		bltSurfaceInfo.pSysMem = static_cast<const unsigned char*>(m_surfaceInfo.pSysMem) +
			bltRect.top * m_surfaceInfo.SysMemPitch +
			bltRect.left * getBytesPerPixel(m_format);
		bltSurfaceInfo.SysMemPitch = m_surfaceInfo.SysMemPitch;

		D3DDDIARG_CREATERESOURCE2 bltResourceData = {};
		bltResourceData.Format = m_format;
		bltResourceData.Pool = D3DDDIPOOL_SYSTEMMEM;
		bltResourceData.pSurfList = &bltSurfaceInfo;
		bltResourceData.SurfCount = 1;

		const auto& deviceFuncs = D3dDdi::DeviceFuncs::s_origVtables.at(m_device);
		if (deviceFuncs.pfnCreateResource2)
		{
			deviceFuncs.pfnCreateResource2(m_device, &bltResourceData);
		}
		else
		{
			deviceFuncs.pfnCreateResource(m_device,
				reinterpret_cast<D3DDDIARG_CREATERESOURCE*>(&bltResourceData));
		}
		return bltResourceData.hResource;
	}

	bool OversizedResource::isSupportedFormat(D3DDDIFORMAT format)
	{
		return 0 != getBytesPerPixel(format);
	}
}
