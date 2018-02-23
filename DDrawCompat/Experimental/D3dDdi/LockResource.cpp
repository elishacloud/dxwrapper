#include "D3dDdi/DeviceFuncs.h"
#include "D3dDdi/LockResource.h"

namespace
{
	D3DDDI_DEVICEFUNCS& getOrigVtable(HANDLE device)
	{
		return D3dDdi::DeviceFuncs::s_origVtables.at(device);
	}
}

namespace D3dDdi
{
	LockResource::LockResource(HANDLE device, HANDLE origResource, HANDLE lockResource,
		const D3DDDI_SURFACEINFO* surfaceInfo, UINT surfaceInfoCount)
		: m_device(device)
		, m_origResource(origResource)
		, m_lockResource(lockResource)
	{
		for (UINT i = 0; i < surfaceInfoCount; ++i)
		{
			m_subResources.emplace_back(*this, i, surfaceInfo[i].Width, surfaceInfo[i].Height);
		}
	}

	LockResource::~LockResource()
	{
	}

	LockResource::SubResource::SubResource(LockResource& parent, UINT index, UINT width, UINT height)
		: m_isOrigUpToDate(true)
		, m_isLockUpToDate(true)
		, m_parent(&parent)
		, m_index(index)
		, m_width(width)
		, m_height(height)
	{
	}

	HRESULT LockResource::SubResource::blt(HANDLE dstResource, HANDLE srcResource)
	{
		RECT rect = { 0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height) };

		D3DDDIARG_BLT bltData = {};
		bltData.hSrcResource = srcResource;
		bltData.SrcSubResourceIndex = m_index;
		bltData.SrcRect = rect;
		bltData.hDstResource = dstResource;
		bltData.DstSubResourceIndex = m_index;
		bltData.DstRect = rect;
		bltData.Flags.Point = 1;

		return getOrigVtable(m_parent->m_device).pfnBlt(m_parent->m_device, &bltData);
	}

	void LockResource::SubResource::updateLock()
	{
		if (!m_isLockUpToDate)
		{
			blt(m_parent->m_lockResource, m_parent->m_origResource);
			m_isLockUpToDate = true;
		}
	}

	void LockResource::SubResource::updateOrig()
	{
		if (!m_isOrigUpToDate)
		{
			blt(m_parent->m_origResource, m_parent->m_lockResource);
			m_isOrigUpToDate = true;
		}
	}
}
