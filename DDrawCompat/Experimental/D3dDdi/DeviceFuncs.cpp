#include <functional>
#include <map>

#include "D3dDdi/AdapterFuncs.h"
#include "D3dDdi/DeviceFuncs.h"
#include "D3dDdi/LockResource.h"
#include "D3dDdi/KernelModeThunks.h"
#include "D3dDdi/OversizedResource.h"

namespace
{
	struct Resource
	{
		HANDLE device;
		HANDLE resource;

		Resource() : device(nullptr), resource(nullptr) {}
		Resource(HANDLE device, HANDLE resource) : device(device), resource(resource) {}

		bool operator<(const Resource& rhs) const
		{
			return device < rhs.device || (device == rhs.device && resource < rhs.resource);
		}
	};

	class ResourceReplacer
	{
	public:
		ResourceReplacer(HANDLE device, const HANDLE& resource, UINT subResourceIndex);
		~ResourceReplacer();
		
		D3dDdi::LockResource::SubResource* getSubResource() const { return m_subResource; }

	private:
		HANDLE& m_resource;
		HANDLE m_origResource;
		D3dDdi::LockResource::SubResource* m_subResource;
	};

	D3DDDI_DEVICEFUNCS& getOrigVtable(HANDLE device);
	D3DDDI_RESOURCEFLAGS getResourceTypeFlags();
	bool isVidMemPool(D3DDDI_POOL pool);
	D3dDdi::LockResource::SubResource* replaceWithActiveResource(
		HANDLE device, HANDLE& resource, UINT subResourceIndex);

	std::map<HANDLE, HANDLE> g_deviceToAdapter;
	std::map<Resource, D3dDdi::LockResource> g_lockResources;
	std::map<Resource, D3dDdi::OversizedResource> g_oversizedResources;
	std::map<HANDLE, D3dDdi::LockResource::SubResource*> g_renderTargets;
	Resource g_sharedPrimary;
	const UINT g_resourceTypeFlags = getResourceTypeFlags().Value;

	ResourceReplacer::ResourceReplacer(HANDLE device, const HANDLE& resource, UINT subResourceIndex)
		: m_resource(const_cast<HANDLE&>(resource))
		, m_origResource(resource)
	{
		m_subResource = replaceWithActiveResource(device, m_resource, subResourceIndex);
	}

	ResourceReplacer::~ResourceReplacer()
	{
		m_resource = m_origResource;
	}

	template <typename CreateResourceArg, typename CreateResourceFunc>
	HRESULT createOversizedResource(HANDLE device, CreateResourceArg& resourceData,
		CreateResourceFunc origCreateResource, const D3DNTHAL_D3DEXTENDEDCAPS& caps)
	{
		D3DDDI_SURFACEINFO compatSurfaceInfo = resourceData.pSurfList[0];
		if (0 != caps.dwMaxTextureWidth && compatSurfaceInfo.Width > caps.dwMaxTextureWidth)
		{
			compatSurfaceInfo.Width = caps.dwMaxTextureWidth;
		}
		if (0 != caps.dwMaxTextureHeight && compatSurfaceInfo.Height > caps.dwMaxTextureHeight)
		{
			compatSurfaceInfo.Height = caps.dwMaxTextureHeight;
		}

		const D3DDDI_SURFACEINFO* origSurfList = resourceData.pSurfList;
		resourceData.pSurfList = &compatSurfaceInfo;
		HRESULT result = origCreateResource(device, &resourceData);
		resourceData.pSurfList = origSurfList;
		
		if (SUCCEEDED(result))
		{
			auto it = g_deviceToAdapter.find(device);
			if (it != g_deviceToAdapter.end())
			{
				D3dDdi::OversizedResource oversizedResource(
					it->second, device, resourceData.Format, origSurfList[0]);
				g_oversizedResources[Resource(device, resourceData.hResource)] = oversizedResource;
			}
		}

		return result;
	}

	template <typename CreateResourceArg, typename CreateResourceFunc>
	HRESULT createResource(HANDLE device, CreateResourceArg* resourceData,
		CreateResourceFunc origCreateResource)
	{
		const bool isOffScreenPlain = 0 == (resourceData->Flags.Value & g_resourceTypeFlags);
		if (D3DDDIPOOL_SYSTEMMEM == resourceData->Pool &&
			(isOffScreenPlain || resourceData->Flags.Texture) &&
			D3dDdi::OversizedResource::isSupportedFormat(resourceData->Format) &&
			1 == resourceData->SurfCount)
		{
			auto it = g_deviceToAdapter.find(device);
			if (it != g_deviceToAdapter.end())
			{
				const auto& caps = D3dDdi::AdapterFuncs::getD3dExtendedCaps(it->second);
				const auto& surfaceInfo = resourceData->pSurfList[0];
				if (0 != caps.dwMaxTextureWidth && surfaceInfo.Width > caps.dwMaxTextureWidth ||
					0 != caps.dwMaxTextureHeight && surfaceInfo.Height > caps.dwMaxTextureHeight)
				{
					return createOversizedResource(device, *resourceData, origCreateResource, caps);
				}
			}
		}

		HANDLE origResourceHandle = resourceData->hResource;
		HRESULT result = origCreateResource(device, resourceData);
		if (SUCCEEDED(result) && resourceData->Flags.RenderTarget && !resourceData->Flags.Primary &&
			isVidMemPool(resourceData->Pool))
		{
			CreateResourceArg lockResourceData = {};
			lockResourceData.Format = resourceData->Format;
			lockResourceData.Pool = resourceData->Pool;
			lockResourceData.pSurfList = resourceData->pSurfList;
			lockResourceData.SurfCount = resourceData->SurfCount;
			lockResourceData.hResource = origResourceHandle;
			lockResourceData.Flags.CpuOptimized = 1;

			if (SUCCEEDED(origCreateResource(device, &lockResourceData)))
			{
				g_lockResources.emplace(std::piecewise_construct,
					std::forward_as_tuple(device, resourceData->hResource),
					std::forward_as_tuple(device, resourceData->hResource, lockResourceData.hResource,
						lockResourceData.pSurfList, lockResourceData.SurfCount));
			}
		}

		return result;
	}

	D3DDDI_DEVICEFUNCS& getOrigVtable(HANDLE device)
	{
		return D3dDdi::DeviceFuncs::s_origVtables.at(device);
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

	bool isVidMemPool(D3DDDI_POOL pool)
	{
		return D3DDDIPOOL_VIDEOMEMORY == pool ||
			D3DDDIPOOL_LOCALVIDMEM == pool ||
			D3DDDIPOOL_NONLOCALVIDMEM == pool;
	}

	HRESULT renderFunc(HANDLE device, std::function<HRESULT()> origFunc)
	{
		auto it = g_renderTargets.find(device);
		if (it != g_renderTargets.end())
		{
			it->second->updateOrig();
		}

		HRESULT result = origFunc();
		if (SUCCEEDED(result) && it != g_renderTargets.end())
		{
			it->second->m_isLockUpToDate = false;
		}
		return result;
	}

	D3dDdi::LockResource::SubResource* replaceWithActiveResource(
		HANDLE device, HANDLE& resource, UINT subResourceIndex)
	{
		auto it = g_lockResources.find(Resource(device, resource));
		if (it == g_lockResources.end())
		{
			return nullptr;
		}

		auto& subResource = it->second.getSubResource(subResourceIndex);
		if (subResource.m_isLockUpToDate)
		{
			resource = it->second.getHandle();
		}
		return &subResource;
	}

	HRESULT APIENTRY blt(HANDLE hDevice, const D3DDDIARG_BLT* pData)
	{
		ResourceReplacer srcReplacer(hDevice, pData->hSrcResource, pData->SrcSubResourceIndex);
		ResourceReplacer dstReplacer(hDevice, pData->hDstResource, pData->DstSubResourceIndex);
		auto dstSubResource = dstReplacer.getSubResource();

		HRESULT result = S_OK;
		auto it = g_oversizedResources.find(Resource(hDevice, pData->hSrcResource));
		if (it != g_oversizedResources.end())
		{
			result = it->second.bltFrom(*pData);
		}
		else
		{
			it = g_oversizedResources.find(Resource(hDevice, pData->hDstResource));
			if (it != g_oversizedResources.end())
			{
				result = it->second.bltTo(*pData);
			}
			else
			{
				result = getOrigVtable(hDevice).pfnBlt(hDevice, pData);
			}
		}
		
		if (SUCCEEDED(result) && dstSubResource && dstSubResource->m_isLockUpToDate)
		{
			dstSubResource->m_isOrigUpToDate = false;
		}
		return result;
	}

	HRESULT APIENTRY colorFill(HANDLE hDevice, const D3DDDIARG_COLORFILL* pData)
	{
		ResourceReplacer replacer(hDevice, pData->hResource, pData->SubResourceIndex);
		auto subResource = replacer.getSubResource();
			
		HRESULT result = getOrigVtable(hDevice).pfnColorFill(hDevice, pData);
		if (SUCCEEDED(result) && subResource && subResource->m_isLockUpToDate)
		{
			subResource->m_isOrigUpToDate = false;
		}
		return result;
	}

	HRESULT APIENTRY createResource(HANDLE hDevice, D3DDDIARG_CREATERESOURCE* pResource)
	{
		return createResource(hDevice, pResource, getOrigVtable(hDevice).pfnCreateResource);
	}

	HRESULT APIENTRY createResource2(HANDLE hDevice, D3DDDIARG_CREATERESOURCE2* pResource2)
	{
		return createResource(hDevice, pResource2, getOrigVtable(hDevice).pfnCreateResource2);
	}

	HRESULT APIENTRY destroyDevice(HANDLE hDevice)
	{
		HRESULT result = getOrigVtable(hDevice).pfnDestroyDevice(hDevice);
		if (SUCCEEDED(result))
		{
			D3dDdi::DeviceFuncs::s_origVtables.erase(hDevice);
			g_deviceToAdapter.erase(hDevice);
		}
		return result;
	}

	HRESULT APIENTRY destroyResource(HANDLE hDevice, HANDLE hResource)
	{
		const bool isSharedPrimary =
			hDevice == g_sharedPrimary.device && hResource == g_sharedPrimary.resource;
		if (isSharedPrimary)
		{
			D3dDdi::KernelModeThunks::releaseVidPnSources();
		}

		HRESULT result = getOrigVtable(hDevice).pfnDestroyResource(hDevice, hResource);
		if (SUCCEEDED(result))
		{
			auto lockResourceIt = g_lockResources.find(Resource(hDevice, hResource));
			if (lockResourceIt != g_lockResources.end())
			{
				auto renderTarget = g_renderTargets.find(hDevice);
				if (renderTarget != g_renderTargets.end() &&
					&renderTarget->second->getParent() == &lockResourceIt->second)
				{
					g_renderTargets.erase(hDevice);
				}

				getOrigVtable(hDevice).pfnDestroyResource(hDevice, lockResourceIt->second.getHandle());
				g_lockResources.erase(lockResourceIt);
			}

			auto oversizedResourceIt = g_oversizedResources.find(Resource(hDevice, hResource));
			if (oversizedResourceIt != g_oversizedResources.end())
			{
				g_oversizedResources.erase(oversizedResourceIt);
			}

			if (isSharedPrimary)
			{
				g_sharedPrimary = {};
			}
		}
		return result;
	}

	HRESULT APIENTRY lock(HANDLE hDevice, D3DDDIARG_LOCK* pData)
	{
		auto it = g_lockResources.find(Resource(hDevice, pData->hResource));
		if (it != g_lockResources.end())
		{
			auto& subResource = it->second.getSubResource(pData->SubResourceIndex);
			subResource.updateLock();

			HANDLE origResourceHandle = pData->hResource;
			pData->hResource = it->second.getHandle();
			HRESULT result = getOrigVtable(hDevice).pfnLock(hDevice, pData);
			pData->hResource = origResourceHandle;

			if (SUCCEEDED(result) && !pData->Flags.ReadOnly)
			{
				subResource.m_isOrigUpToDate = false;
			}

			return result;
		}
		return getOrigVtable(hDevice).pfnLock(hDevice, pData);
	}

	HRESULT APIENTRY openResource(HANDLE hDevice, D3DDDIARG_OPENRESOURCE* pResource)
	{
		HRESULT result = getOrigVtable(hDevice).pfnOpenResource(hDevice, pResource);
		if (SUCCEEDED(result) && pResource->Flags.Fullscreen)
		{
			g_sharedPrimary = Resource(hDevice, pResource->hResource);
		}
		return result;
	}

	HRESULT APIENTRY present(HANDLE hDevice, const D3DDDIARG_PRESENT* pData)
	{
		auto it = g_lockResources.find(Resource(hDevice, pData->hSrcResource));
		if (it != g_lockResources.end())
		{
			it->second.getSubResource(pData->SrcSubResourceIndex).updateOrig();
		}
		return getOrigVtable(hDevice).pfnPresent(hDevice, pData);
	}

	HRESULT APIENTRY present1(HANDLE hDevice, D3DDDIARG_PRESENT1* pPresentData)
	{
		for (UINT i = 0; i < pPresentData->SrcResources; ++i)
		{
			auto it = g_lockResources.find(Resource(hDevice, pPresentData->phSrcResources[i].hResource));
			if (it != g_lockResources.end())
			{
				it->second.getSubResource(pPresentData->phSrcResources[i].SubResourceIndex).updateOrig();
			}
		}
		return getOrigVtable(hDevice).pfnPresent1(hDevice, pPresentData);
	}

	template <typename DeviceFuncMemberPtr, DeviceFuncMemberPtr origFunc, typename... Params>
	HRESULT WINAPI renderFunc(HANDLE device, Params... params)
	{
		return renderFunc(device,
			[=]() { return (getOrigVtable(device).*origFunc)(device, params...); });
	}

	HRESULT APIENTRY setRenderTarget(HANDLE hDevice, const D3DDDIARG_SETRENDERTARGET* pData)
	{
		HRESULT result = getOrigVtable(hDevice).pfnSetRenderTarget(hDevice, pData);
		if (SUCCEEDED(result))
		{
			auto it = g_lockResources.find(Resource(hDevice, pData->hRenderTarget));
			if (it != g_lockResources.end())
			{
				g_renderTargets[hDevice] = &it->second.getSubResource(pData->SubResourceIndex);
			}
			else
			{
				g_renderTargets.erase(hDevice);
			}
		}
		return result;
	}

	HRESULT APIENTRY unlock(HANDLE hDevice, const D3DDDIARG_UNLOCK* pData)
	{
		auto it = g_lockResources.find(Resource(hDevice, pData->hResource));
		if (it != g_lockResources.end())
		{
			HANDLE origResource = pData->hResource;
			const_cast<HANDLE&>(pData->hResource) = it->second.getHandle();
			HRESULT result = getOrigVtable(hDevice).pfnUnlock(hDevice, pData);
			const_cast<HANDLE&>(pData->hResource) = origResource;
			return result;
		}
		return getOrigVtable(hDevice).pfnUnlock(hDevice, pData);
	}

	HRESULT APIENTRY updateWInfo(HANDLE hDevice, const D3DDDIARG_WINFO* pData)
	{
		if (pData && 1.0f == pData->WNear && 1.0f == pData->WFar)
		{
			D3DDDIARG_WINFO wInfo = {};
			wInfo.WNear = 0.0f;
			wInfo.WFar = 1.0f;
			return getOrigVtable(hDevice).pfnUpdateWInfo(hDevice, &wInfo);
		}
		return getOrigVtable(hDevice).pfnUpdateWInfo(hDevice, pData);
	}
}

#define RENDER_FUNC(func) renderFunc<decltype(&D3DDDI_DEVICEFUNCS::func), &D3DDDI_DEVICEFUNCS::func>

namespace D3dDdi
{
	void DeviceFuncs::onCreateDevice(HANDLE adapter, HANDLE device)
	{
		g_deviceToAdapter[device] = adapter;
	}

	void DeviceFuncs::setCompatVtable(D3DDDI_DEVICEFUNCS& vtable)
	{
		vtable.pfnBlt = &blt;
		vtable.pfnClear = &RENDER_FUNC(pfnClear);
		vtable.pfnColorFill = &colorFill;
		vtable.pfnCreateResource = &createResource;
		vtable.pfnCreateResource2 = &createResource2;
		vtable.pfnDestroyDevice = &destroyDevice;
		vtable.pfnDestroyResource = &destroyResource;
		vtable.pfnDrawIndexedPrimitive = &RENDER_FUNC(pfnDrawIndexedPrimitive);
		vtable.pfnDrawIndexedPrimitive2 = &RENDER_FUNC(pfnDrawIndexedPrimitive2);
		vtable.pfnDrawPrimitive = &RENDER_FUNC(pfnDrawPrimitive);
		vtable.pfnDrawPrimitive2 = &RENDER_FUNC(pfnDrawPrimitive2);
		vtable.pfnDrawRectPatch = &RENDER_FUNC(pfnDrawRectPatch);
		vtable.pfnDrawTriPatch = &RENDER_FUNC(pfnDrawTriPatch);
		vtable.pfnLock = &lock;
		vtable.pfnOpenResource = &openResource;
		vtable.pfnPresent = &present;
		vtable.pfnPresent1 = &present1;
		vtable.pfnSetRenderTarget = &setRenderTarget;
		vtable.pfnUnlock = &unlock;
		vtable.pfnUpdateWInfo = &updateWInfo;
	}
}
