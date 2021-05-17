#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <type_traits>

#include <DDrawCompat/v0.3.0/Common/HResultException.h>
#include <DDrawCompat/DDrawLog.h>
#include <DDrawCompat/v0.3.0/Common/Time.h>
#include <DDrawCompat/v0.3.0/Config/Config.h>
#include <DDrawCompat/v0.3.0/D3dDdi/Adapter.h>
#include <DDrawCompat/v0.3.0/D3dDdi/Device.h>
#include <DDrawCompat/v0.3.0/D3dDdi/KernelModeThunks.h>
#include <DDrawCompat/v0.3.0/D3dDdi/Log/DeviceFuncsLog.h>
#include <DDrawCompat/v0.3.0/D3dDdi/Resource.h>
#include <DDrawCompat/v0.3.0/DDraw/Blitter.h>
#include <DDrawCompat/v0.3.0/Gdi/Palette.h>
#include <DDrawCompat/v0.3.0/Gdi/VirtualScreen.h>

namespace
{
	D3DDDI_RESOURCEFLAGS getResourceTypeFlags();
	void splitToTiles(D3DDDIARG_CREATERESOURCE& data, const UINT tileWidth, const UINT tileHeight);

	const UINT g_resourceTypeFlags = getResourceTypeFlags().Value;

	LONG divCeil(LONG n, LONG d)
	{
		return (n + d - 1) / d;
	}

	void fixResourceData(D3dDdi::Device& device, D3DDDIARG_CREATERESOURCE& data)
	{
		if (data.Flags.Primary)
		{
			data.Format = D3DDDIFMT_X8R8G8B8;
		}

		const bool isOffScreenPlain = 0 == (data.Flags.Value & g_resourceTypeFlags);
		if (D3DDDIPOOL_SYSTEMMEM == data.Pool &&
			(isOffScreenPlain || data.Flags.Texture) &&
			1 == data.SurfCount &&
			0 == data.pSurfList[0].Depth &&
			0 != D3dDdi::getFormatInfo(data.Format).bytesPerPixel)
		{
			const auto& caps = device.getAdapter().getD3dExtendedCaps();
			const auto& surfaceInfo = data.pSurfList[0];
			if (0 != caps.dwMaxTextureWidth && surfaceInfo.Width > caps.dwMaxTextureWidth ||
				0 != caps.dwMaxTextureHeight && surfaceInfo.Height > caps.dwMaxTextureHeight)
			{
				splitToTiles(data, caps.dwMaxTextureWidth, caps.dwMaxTextureHeight);
			}
		}
	}

	D3DDDI_RESOURCEFLAGS getResourceTypeFlags()
	{
		D3DDDI_RESOURCEFLAGS flags = {};
		flags.RenderTarget = 1;
		flags.ZBuffer = 1;
		flags.DMap = 1;
		flags.Points = 1;
		flags.RtPatches = 1;
		flags.NPatches = 1;
		flags.Video = 1;
		flags.CaptureBuffer = 1;
		flags.Primary = 1;
		flags.Texture = 1;
		flags.CubeMap = 1;
		flags.VertexBuffer = 1;
		flags.IndexBuffer = 1;
		flags.DecodeRenderTarget = 1;
		flags.DecodeCompressedBuffer = 1;
		flags.VideoProcessRenderTarget = 1;
		flags.Overlay = 1;
		flags.TextApi = 1;
		return flags;
	}

	void heapFree(void* p)
	{
		HeapFree(GetProcessHeap(), 0, p);
	}

	void splitToTiles(D3DDDIARG_CREATERESOURCE& data, const UINT tileWidth, const UINT tileHeight)
	{
		static std::vector<D3DDDI_SURFACEINFO> tiles;
		tiles.clear();

		const UINT bytesPerPixel = D3dDdi::getFormatInfo(data.Format).bytesPerPixel;

		for (UINT y = 0; y < data.pSurfList[0].Height; y += tileHeight)
		{
			for (UINT x = 0; x < data.pSurfList[0].Width; x += tileWidth)
			{
				D3DDDI_SURFACEINFO tile = {};
				tile.Width = min(data.pSurfList[0].Width - x, tileWidth);
				tile.Height = min(data.pSurfList[0].Height - y, tileHeight);
				tile.pSysMem = static_cast<const unsigned char*>(data.pSurfList[0].pSysMem) +
					y * data.pSurfList[0].SysMemPitch + x * bytesPerPixel;
				tile.SysMemPitch = data.pSurfList[0].SysMemPitch;
				tiles.push_back(tile);
			}
		}

		data.SurfCount = tiles.size();
		data.pSurfList = tiles.data();
		data.Flags.Texture = 0;
	}

	D3DDDIARG_CREATERESOURCE2 upgradeResourceData(const D3DDDIARG_CREATERESOURCE& data)
	{
		D3DDDIARG_CREATERESOURCE2 data2 = {};
		reinterpret_cast<D3DDDIARG_CREATERESOURCE&>(data2) = data;
		return data2;
	}
}

namespace D3dDdi
{
	Resource::Data::Data()
		: D3DDDIARG_CREATERESOURCE2{}
	{
	}

	Resource::Data::Data(const D3DDDIARG_CREATERESOURCE& data)
		: Data(upgradeResourceData(data))
	{
	}

	Resource::Data::Data(const D3DDDIARG_CREATERESOURCE2& data)
		: D3DDDIARG_CREATERESOURCE2(data)
	{
		surfaceData.reserve(data.SurfCount);
		for (UINT i = 0; i < data.SurfCount; ++i)
		{
			surfaceData.push_back(data.pSurfList[i]);
		}
		pSurfList = surfaceData.data();
	}

	template <typename Arg>
	Resource::Resource(Device& device, Arg& data, HRESULT(APIENTRY* createResourceFunc)(HANDLE, Arg*))
		: m_device(device)
		, m_handle(nullptr)
		, m_origData(data)
		, m_fixedData(data)
		, m_lockBuffer(nullptr, &heapFree)
		, m_lockResource(nullptr, ResourceDeleter(device))
	{
		if (m_origData.Flags.VertexBuffer &&
			m_origData.Flags.MightDrawFromLocked &&
			D3DDDIPOOL_SYSTEMMEM != m_origData.Pool)
		{
			throw HResultException(E_FAIL);
		}

		fixResourceData(device, reinterpret_cast<D3DDDIARG_CREATERESOURCE&>(m_fixedData));
		m_formatInfo = getFormatInfo(m_fixedData.Format);

		HRESULT result = createResourceFunc(device, reinterpret_cast<Arg*>(&m_fixedData));
		if (FAILED(result))
		{
			throw HResultException(result);
		}
		m_handle = m_fixedData.hResource;

		if (D3DDDIPOOL_SYSTEMMEM == m_fixedData.Pool &&
			0 != m_formatInfo.bytesPerPixel)
		{
			m_lockData.resize(m_fixedData.SurfCount);
			for (UINT i = 0; i < m_fixedData.SurfCount; ++i)
			{
				m_lockData[i].data = const_cast<void*>(m_fixedData.pSurfList[i].pSysMem);
				m_lockData[i].pitch = m_fixedData.pSurfList[i].SysMemPitch;
				m_lockData[i].isSysMemUpToDate = true;
			}
		}

		createLockResource();
		data.hResource = m_fixedData.hResource;
	}

	Resource::Resource(Device& device, D3DDDIARG_CREATERESOURCE& data)
		: Resource(device, data, device.getOrigVtable().pfnCreateResource)
	{
	}

	Resource::Resource(Device& device, D3DDDIARG_CREATERESOURCE2& data)
		: Resource(device, data, device.getOrigVtable().pfnCreateResource2)
	{
	}

	HRESULT Resource::blt(D3DDDIARG_BLT data)
	{
		if (!isValidRect(data.DstSubResourceIndex, data.DstRect))
		{
			return S_OK;
		}

		auto srcResource = m_device.getResource(data.hSrcResource);
		if (srcResource)
		{
			if (!srcResource->isValidRect(data.SrcSubResourceIndex, data.SrcRect))
			{
				return S_OK;
			}

			if (D3DDDIPOOL_SYSTEMMEM == m_fixedData.Pool &&
				D3DDDIPOOL_SYSTEMMEM == srcResource->m_fixedData.Pool)
			{
				return m_device.getOrigVtable().pfnBlt(m_device, &data);
			}
		}

		if (isOversized())
		{
			m_device.prepareForRendering(data.hSrcResource, data.SrcSubResourceIndex, true);
			return splitBlt(data, data.DstSubResourceIndex, data.DstRect, data.SrcRect);
		}
		else if (srcResource)
		{
			if (srcResource->isOversized())
			{
				prepareForRendering(data.DstSubResourceIndex, false);
				return srcResource->splitBlt(data, data.SrcSubResourceIndex, data.SrcRect, data.DstRect);
			}
			else if (m_fixedData.Flags.Primary)
			{
				return presentationBlt(data, *srcResource);
			}
			else
			{
				return sysMemPreferredBlt(data, *srcResource);
			}
		}
		prepareForRendering(data.DstSubResourceIndex, false);
		return m_device.getOrigVtable().pfnBlt(m_device, &data);
	}

	HRESULT Resource::bltLock(D3DDDIARG_LOCK& data)
	{
		LOG_FUNC("Resource::bltLock", data);

		auto& lockData = m_lockData[data.SubResourceIndex];
		if (!lockData.isSysMemUpToDate)
		{
			copyToSysMem(data.SubResourceIndex);
		}
		lockData.isVidMemUpToDate &= data.Flags.ReadOnly;
		lockData.qpcLastForcedLock = Time::queryPerformanceCounter();

		unsigned char* ptr = static_cast<unsigned char*>(lockData.data);
		if (data.Flags.AreaValid)
		{
			ptr += data.Area.top * lockData.pitch + data.Area.left * m_formatInfo.bytesPerPixel;
		}

		data.pSurfData = ptr;
		data.Pitch = lockData.pitch;
		++lockData.lockCount;
		return LOG_RESULT(S_OK);
	}

	HRESULT Resource::bltUnlock(const D3DDDIARG_UNLOCK& data)
	{
		LOG_FUNC("Resource::bltUnlock", data);
		if (0 != m_lockData[data.SubResourceIndex].lockCount)
		{
			--m_lockData[data.SubResourceIndex].lockCount;
		}
		return LOG_RESULT(S_OK);
	}

	void Resource::clipRect(UINT subResourceIndex, RECT& rect)
	{
		rect.left = std::max<LONG>(rect.left, 0);
		rect.top = std::max<LONG>(rect.top, 0);
		rect.right = std::min<LONG>(rect.right, m_fixedData.pSurfList[subResourceIndex].Width);
		rect.bottom = std::min<LONG>(rect.bottom, m_fixedData.pSurfList[subResourceIndex].Height);
	}

	HRESULT Resource::colorFill(D3DDDIARG_COLORFILL data)
	{
		LOG_FUNC("Resource::colorFill", data);
		clipRect(data.SubResourceIndex, data.DstRect);
		if (data.DstRect.left >= data.DstRect.right || data.DstRect.top >= data.DstRect.bottom)
		{
			return S_OK;
		}

		if (m_lockResource)
		{
			auto& lockData = m_lockData[data.SubResourceIndex];
			if (lockData.isSysMemUpToDate)
			{
				auto dstBuf = static_cast<BYTE*>(lockData.data) +
					data.DstRect.top * lockData.pitch + data.DstRect.left * m_formatInfo.bytesPerPixel;

				DDraw::Blitter::colorFill(dstBuf, lockData.pitch,
					data.DstRect.right - data.DstRect.left, data.DstRect.bottom - data.DstRect.top,
					m_formatInfo.bytesPerPixel, colorConvert(m_formatInfo, data.Color));

				m_lockData[data.SubResourceIndex].isVidMemUpToDate = false;
				return LOG_RESULT(S_OK);
			}
		}
		prepareForRendering(data.SubResourceIndex, false);
		return LOG_RESULT(m_device.getOrigVtable().pfnColorFill(m_device, &data));
	}

	HRESULT Resource::copySubResource(HANDLE dstResource, HANDLE srcResource, UINT subResourceIndex)
	{
		LOG_FUNC("Resource::copySubResource", dstResource, srcResource, subResourceIndex);
		RECT rect = {};
		rect.right = m_fixedData.pSurfList[subResourceIndex].Width;
		rect.bottom = m_fixedData.pSurfList[subResourceIndex].Height;

		D3DDDIARG_BLT data = {};
		data.hSrcResource = srcResource;
		data.SrcSubResourceIndex = subResourceIndex;
		data.SrcRect = rect;
		data.hDstResource = dstResource;
		data.DstSubResourceIndex = subResourceIndex;
		data.DstRect = rect;

		HRESULT result = m_device.getOrigVtable().pfnBlt(m_device, &data);
		if (FAILED(result))
		{
			LOG_ONCE("ERROR: Resource::copySubResource failed: " << Compat30::hex(result));
		}

		D3DDDIARG_LOCK lock = {};
		lock.hResource = m_lockResource.get();
		lock.SubResourceIndex = subResourceIndex;
		lock.Flags.NotifyOnly = 1;
		m_device.getOrigVtable().pfnLock(m_device, &lock);

		D3DDDIARG_UNLOCK unlock = {};
		unlock.hResource = m_lockResource.get();
		unlock.SubResourceIndex = subResourceIndex;
		unlock.Flags.NotifyOnly = 1;
		m_device.getOrigVtable().pfnUnlock(m_device, &unlock);

		return LOG_RESULT(result);
	}

	void Resource::copyToSysMem(UINT subResourceIndex)
	{
		copySubResource(m_lockResource.get(), m_handle, subResourceIndex);
		m_lockData[subResourceIndex].isSysMemUpToDate = true;
	}

	void Resource::copyToVidMem(UINT subResourceIndex)
	{
		copySubResource(m_handle, m_lockResource.get(), subResourceIndex);
		m_lockData[subResourceIndex].isVidMemUpToDate = true;
	}

	void Resource::createGdiLockResource()
	{
		auto gdiSurfaceDesc(Gdi::VirtualScreen::getSurfaceDesc(D3dDdi::KernelModeThunks::getMonitorRect()));
		if (!gdiSurfaceDesc.lpSurface)
		{
			return;
		}

		D3DDDI_SURFACEINFO surfaceInfo = {};
		surfaceInfo.Width = gdiSurfaceDesc.dwWidth;
		surfaceInfo.Height = gdiSurfaceDesc.dwHeight;
		surfaceInfo.pSysMem = gdiSurfaceDesc.lpSurface;
		surfaceInfo.SysMemPitch = gdiSurfaceDesc.lPitch;

		m_lockData.resize(m_fixedData.SurfCount);
		createSysMemResource({ surfaceInfo });
		if (m_lockResource)
		{
			m_lockData[0].isVidMemUpToDate = false;
		}
		else
		{
			m_lockData.clear();
		}
	}

	void Resource::createLockResource()
	{
		D3DDDI_RESOURCEFLAGS flags = {};
		flags.Value = g_resourceTypeFlags;
		flags.RenderTarget = 0;
		if (D3DDDIPOOL_SYSTEMMEM == m_fixedData.Pool ||
			0 == m_formatInfo.bytesPerPixel ||
			0 != (m_fixedData.Flags.Value & flags.Value))
		{
			return;
		}

		std::vector<D3DDDI_SURFACEINFO> surfaceInfo(m_fixedData.SurfCount);
		for (UINT i = 0; i < m_fixedData.SurfCount; ++i)
		{
			surfaceInfo[i].Width = m_fixedData.pSurfList[i].Width;
			surfaceInfo[i].Height = m_fixedData.pSurfList[i].Height;
			surfaceInfo[i].SysMemPitch = (surfaceInfo[i].Width * m_formatInfo.bytesPerPixel + 3) & ~3;
			if (i != 0)
			{
				std::uintptr_t offset = reinterpret_cast<std::uintptr_t>(surfaceInfo[i - 1].pSysMem) +
					((surfaceInfo[i - 1].SysMemPitch * surfaceInfo[i - 1].Height + 15) & ~15);
				surfaceInfo[i].pSysMem = reinterpret_cast<void*>(offset);
			}
		}

		std::uintptr_t bufferSize = reinterpret_cast<std::uintptr_t>(surfaceInfo.back().pSysMem) +
			surfaceInfo.back().SysMemPitch * surfaceInfo.back().Height + 8;
		m_lockBuffer.reset(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bufferSize));

		BYTE* bufferStart = static_cast<BYTE*>(m_lockBuffer.get());
		if (0 == reinterpret_cast<std::uintptr_t>(bufferStart) % 16)
		{
			bufferStart += 8;
		}

		for (UINT i = 0; i < m_fixedData.SurfCount; ++i)
		{
			surfaceInfo[i].pSysMem = bufferStart + reinterpret_cast<uintptr_t>(surfaceInfo[i].pSysMem);
		}

		createSysMemResource(surfaceInfo);
		if (!m_lockResource)
		{
			m_lockBuffer.reset();
			m_lockData.clear();
		}
	}

	void Resource::createSysMemResource(const std::vector<D3DDDI_SURFACEINFO>& surfaceInfo)
	{
		LOG_FUNC("Resource::createSysMemResource", Compat30::array(surfaceInfo.data(), surfaceInfo.size()));
		D3DDDIARG_CREATERESOURCE2 data = {};
		data.Format = m_fixedData.Format;
		data.Pool = D3DDDIPOOL_SYSTEMMEM;
		data.pSurfList = surfaceInfo.data();
		data.SurfCount = surfaceInfo.size();
		data.Rotation = D3DDDI_ROTATION_IDENTITY;

		HRESULT result = m_device.createPrivateResource(data);
		if (SUCCEEDED(result))
		{
			m_lockResource.reset(data.hResource);
			m_lockData.resize(surfaceInfo.size());
			auto qpcLastForcedLock = Time::queryPerformanceCounter() - Time::msToQpc(Config30::evictionTimeout);
			for (std::size_t i = 0; i < surfaceInfo.size(); ++i)
			{
				m_lockData[i].data = const_cast<void*>(surfaceInfo[i].pSysMem);
				m_lockData[i].pitch = surfaceInfo[i].SysMemPitch;
				m_lockData[i].qpcLastForcedLock = qpcLastForcedLock;
				m_lockData[i].isSysMemUpToDate = true;
				m_lockData[i].isVidMemUpToDate = true;
			}
		}

#ifdef DEBUGLOGS
		LOG_RESULT(m_lockResource.get());
#endif
	}

	void* Resource::getLockPtr(UINT subResourceIndex)
	{
		return m_lockData.empty() ? nullptr : m_lockData[subResourceIndex].data;
	}

	bool Resource::isOversized() const
	{
		return m_fixedData.SurfCount != m_origData.SurfCount;
	}

	bool Resource::isValidRect(UINT subResourceIndex, const RECT& rect)
	{
		return rect.left >= 0 && rect.top >= 0 && rect.left < rect.right && rect.top < rect.bottom &&
			rect.right <= static_cast<LONG>(m_fixedData.pSurfList[subResourceIndex].Width) &&
			rect.bottom <= static_cast<LONG>(m_fixedData.pSurfList[subResourceIndex].Height);
	}

	HRESULT Resource::lock(D3DDDIARG_LOCK& data)
	{
		if (isOversized())
		{
			if (0 != data.SubResourceIndex ||
				data.Flags.RangeValid || data.Flags.AreaValid || data.Flags.BoxValid)
			{
				LOG_ONCE("WARNING: Unsupported lock of oversized resource: " << data);
				return m_device.getOrigVtable().pfnLock(m_device, &data);
			}
			return splitLock(data, m_device.getOrigVtable().pfnLock);
		}

		if (m_lockResource)
		{
			return bltLock(data);
		}

		return m_device.getOrigVtable().pfnLock(m_device, &data);
	}

	void Resource::prepareForGdiRendering(bool isReadOnly)
	{
		if (!m_lockResource)
		{
			return;
		}

		if (!m_lockData[0].isSysMemUpToDate)
		{
			copyToSysMem(0);
		}
		m_lockData[0].isVidMemUpToDate &= isReadOnly;
		m_lockData[0].qpcLastForcedLock = Time::queryPerformanceCounter();
	}

	void Resource::prepareForRendering(UINT subResourceIndex, bool isReadOnly)
	{
		if (m_lockResource && 0 == m_lockData[subResourceIndex].lockCount)
		{
			if (!m_lockData[subResourceIndex].isVidMemUpToDate)
			{
				copyToVidMem(subResourceIndex);
			}
			m_lockData[subResourceIndex].isSysMemUpToDate &= isReadOnly;
		}
	}

	HRESULT Resource::presentationBlt(const D3DDDIARG_BLT& data, Resource& srcResource)
	{
		if (D3DDDIFMT_P8 == srcResource.m_origData.Format)
		{
			D3DDDIARG_LOCK lock = {};
			lock.hResource = m_handle;
			lock.SubResourceIndex = data.DstSubResourceIndex;
			HRESULT result = m_device.getOrigVtable().pfnLock(m_device, &lock);
			if (FAILED(result))
			{
				return result;
			}

			auto entries(Gdi::Palette::getHardwarePalette());
			DWORD pal[256] = {};
			for (UINT i = 0; i < 256; ++i)
			{
				pal[i] = (entries[i].peRed << 16) | (entries[i].peGreen << 8) | entries[i].peBlue;
			}

			auto& srcLockData = srcResource.m_lockData[data.SrcSubResourceIndex];
			for (UINT y = 0; y < srcResource.m_fixedData.surfaceData[data.SrcSubResourceIndex].Height; ++y)
			{
				auto src = static_cast<const BYTE*>(srcLockData.data) + y * srcLockData.pitch;
				auto dst = reinterpret_cast<DWORD*>(static_cast<BYTE*>(lock.pSurfData) + y * lock.Pitch);
				for (UINT x = 0; x < srcResource.m_fixedData.surfaceData[data.SrcSubResourceIndex].Width; ++x)
				{
					dst[x] = pal[*src];
					++src;
				}
			}

			D3DDDIARG_UNLOCK unlock = {};
			unlock.hResource = m_handle;
			unlock.SubResourceIndex = data.DstSubResourceIndex;
			return m_device.getOrigVtable().pfnUnlock(m_device, &unlock);
		}

		if (srcResource.m_lockResource &&
			srcResource.m_lockData[data.SrcSubResourceIndex].isSysMemUpToDate)
		{
			srcResource.copyToVidMem(data.SrcSubResourceIndex);
		}
		return m_device.getOrigVtable().pfnBlt(m_device, &data);
	}

	void Resource::setAsGdiResource(bool isGdiResource)
	{
		m_lockResource.reset();
		m_lockData.clear();
		m_lockBuffer.reset();
		if (isGdiResource)
		{
			createGdiLockResource();
		}
		else
		{
			createLockResource();
		}
	}

	HRESULT Resource::splitBlt(D3DDDIARG_BLT& data, UINT& subResourceIndex, RECT& rect, RECT& otherRect)
	{
		LOG_FUNC("Resource::splitBlt", data, subResourceIndex, rect, otherRect);

		if (0 != subResourceIndex ||
			data.SrcRect.right - data.SrcRect.left != data.DstRect.right - data.DstRect.left ||
			data.SrcRect.bottom - data.SrcRect.top != data.DstRect.bottom - data.DstRect.top ||
			data.Flags.MirrorLeftRight ||
			data.Flags.MirrorUpDown ||
			data.Flags.Rotate)
		{
			LOG_ONCE("WARNING: Unsupported blt of oversized resource: " << data);
			return LOG_RESULT(m_device.getOrigVtable().pfnBlt(m_device, &data));
		}

		const auto& caps = m_device.getAdapter().getD3dExtendedCaps();
		const auto tilesPerRow = divCeil(m_origData.pSurfList[0].Width, caps.dwMaxTextureWidth);

		const RECT origRect = rect;
		const POINT otherRectOffset = { otherRect.left - rect.left, otherRect.top - rect.top };

		POINT tilePos = {};
		tilePos.y = rect.top / static_cast<LONG>(caps.dwMaxTextureHeight);

		RECT tileRect = {};
		tileRect.top = tilePos.y * caps.dwMaxTextureHeight;
		tileRect.bottom = tileRect.top + caps.dwMaxTextureHeight;

		while (tileRect.top < origRect.bottom)
		{
			tilePos.x = origRect.left / static_cast<LONG>(caps.dwMaxTextureWidth);
			tileRect.left = tilePos.x * caps.dwMaxTextureWidth;
			tileRect.right = tileRect.left + caps.dwMaxTextureWidth;

			while (tileRect.left < origRect.right)
			{
				IntersectRect(&rect, &tileRect, &origRect);
				otherRect = rect;
				OffsetRect(&otherRect, otherRectOffset.x, otherRectOffset.y);
				OffsetRect(&rect, -tileRect.left, -tileRect.top);
				subResourceIndex = tilePos.y * tilesPerRow + tilePos.x;

				HRESULT result = m_device.getOrigVtable().pfnBlt(m_device, &data);
				if (FAILED(result))
				{
					return LOG_RESULT(result);
				}

				++tilePos.x;
				tileRect.left += caps.dwMaxTextureWidth;
				tileRect.right += caps.dwMaxTextureWidth;
			}

			++tilePos.y;
			tileRect.top += caps.dwMaxTextureHeight;
			tileRect.bottom += caps.dwMaxTextureHeight;
		}

		return LOG_RESULT(S_OK);
	}

	HRESULT Resource::sysMemPreferredBlt(const D3DDDIARG_BLT& data, Resource& srcResource)
	{
		if (m_fixedData.Format == srcResource.m_fixedData.Format &&
			!m_lockData.empty() &&
			!srcResource.m_lockData.empty())
		{
			auto& dstLockData = m_lockData[data.DstSubResourceIndex];
			auto& srcLockData = srcResource.m_lockData[data.SrcSubResourceIndex];

			bool isSysMemBltPreferred = true;
			auto now = Time::queryPerformanceCounter();
			if (D3DDDIFMT_P8 != m_fixedData.Format)
			{
				if (data.Flags.MirrorLeftRight || data.Flags.MirrorUpDown ||
					(data.Flags.SrcColorKey && !m_device.isSrcColorKeySupported()))
				{
					dstLockData.qpcLastForcedLock = now;
					srcLockData.qpcLastForcedLock = now;
				}
				else
				{
					isSysMemBltPreferred = dstLockData.isSysMemUpToDate &&
						Time::qpcToMs(now - dstLockData.qpcLastForcedLock) <= Config30::evictionTimeout;
				}
			}

			if (isSysMemBltPreferred)
			{
				if (!dstLockData.isSysMemUpToDate)
				{
					copyToSysMem(data.DstSubResourceIndex);
				}
				dstLockData.isVidMemUpToDate = false;

				if (!srcLockData.isSysMemUpToDate)
				{
					srcResource.copyToSysMem(data.SrcSubResourceIndex);
				}

				auto dstBuf = static_cast<BYTE*>(dstLockData.data) +
					data.DstRect.top * dstLockData.pitch + data.DstRect.left * m_formatInfo.bytesPerPixel;
				auto srcBuf = static_cast<const BYTE*>(srcLockData.data) +
					data.SrcRect.top * srcLockData.pitch + data.SrcRect.left * m_formatInfo.bytesPerPixel;

				DDraw::Blitter::blt(
					dstBuf,
					dstLockData.pitch,
					data.DstRect.right - data.DstRect.left,
					data.DstRect.bottom - data.DstRect.top,
					srcBuf,
					srcLockData.pitch,
					(1 - 2 * data.Flags.MirrorLeftRight) * (data.SrcRect.right - data.SrcRect.left),
					(1 - 2 * data.Flags.MirrorUpDown) * (data.SrcRect.bottom - data.SrcRect.top),
					m_formatInfo.bytesPerPixel,
					data.Flags.DstColorKey ? reinterpret_cast<const DWORD*>(&data.ColorKey) : nullptr,
					data.Flags.SrcColorKey ? reinterpret_cast<const DWORD*>(&data.ColorKey) : nullptr);

				return S_OK;
			}
		}

		prepareForRendering(data.DstSubResourceIndex, false);
		srcResource.prepareForRendering(data.SrcSubResourceIndex, true);
		return m_device.getOrigVtable().pfnBlt(m_device, &data);
	}

	template <typename Arg>
	HRESULT Resource::splitLock(Arg& data, HRESULT(APIENTRY *lockFunc)(HANDLE, Arg*))
	{
		LOG_FUNC("Resource::splitLock", data, lockFunc);
		std::remove_const<Arg>::type tmpData = data;
		HRESULT result = lockFunc(m_device, &data);
		if (SUCCEEDED(result))
		{
			for (UINT i = 1; i < m_fixedData.SurfCount; ++i)
			{
				tmpData.SubResourceIndex = i;
				lockFunc(m_device, &tmpData);
			}
		}
		return LOG_RESULT(result);
	}

	HRESULT Resource::unlock(const D3DDDIARG_UNLOCK& data)
	{
		if (isOversized())
		{
			if (0 != data.SubResourceIndex)
			{
				LOG_ONCE("WARNING: Unsupported unlock of oversized resource: " << data);
				return m_device.getOrigVtable().pfnUnlock(m_device, &data);
			}
			return splitLock(data, m_device.getOrigVtable().pfnUnlock);
		}
		else if (m_lockResource)
		{
			return bltUnlock(data);
		}

		return m_device.getOrigVtable().pfnUnlock(m_device, &data);
	}
}
