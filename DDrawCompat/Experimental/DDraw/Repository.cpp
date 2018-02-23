#include <algorithm>
#include <map>
#include <vector>

#include "Common/CompatPtr.h"
#include "DDrawLog.h"
#include "DDraw/DirectDraw.h"
#include "DDraw/Repository.h"
#include "Dll/Procs.h"

namespace
{
	using DDraw::Repository::Surface;

	struct Repository
	{
		CompatWeakPtr<IDirectDraw7> dd;
		std::vector<Surface> surfaces;
	};

	Repository g_sysMemRepo;
	std::map<void*, Repository> g_vidMemRepos;

	CompatPtr<IDirectDraw7> createDirectDraw();
	Surface createSurface(CompatRef<IDirectDraw7> dd,
		DWORD width, DWORD height, const DDPIXELFORMAT& pf, DWORD caps);
	std::vector<Surface>::iterator findSurface(DWORD width, DWORD height, const DDPIXELFORMAT& pf,
		std::vector<Surface>& cachedSurfaces);
	void destroySmallerSurfaces(DWORD width, DWORD height, const DDPIXELFORMAT& pf,
		std::vector<Surface>& cachedSurfaces);
	Surface getSurface(CompatRef<IDirectDraw7> dd, const DDSURFACEDESC2& desc);
	void normalizePixelFormat(DDPIXELFORMAT& pf);
	void returnSurface(const Surface& surface);

	CompatPtr<IDirectDraw7> createDirectDraw()
	{
		CompatPtr<IDirectDraw7> dd;
		HRESULT result = CALL_ORIG_PROC(DirectDrawCreateEx, nullptr,
			reinterpret_cast<void**>(&dd.getRef()), IID_IDirectDraw7, nullptr);
		if (FAILED(result))
		{
			LOG_ONCE("Failed to create a DirectDraw object in the repository: " << result);
			return nullptr;
		}

		result = dd.get()->lpVtbl->SetCooperativeLevel(dd, nullptr, DDSCL_NORMAL);
		if (FAILED(result))
		{
			LOG_ONCE("Failed to set the cooperative level in the repository: " << result);
			return nullptr;
		}

		return dd;
	}

	Surface createSurface(CompatRef<IDirectDraw7> dd,
		DWORD width, DWORD height, const DDPIXELFORMAT& pf, DWORD caps)
	{
		Surface surface = {};

		surface.desc.dwSize = sizeof(surface.desc);
		surface.desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT | DDSD_CAPS;
		surface.desc.dwWidth = width;
		surface.desc.dwHeight = height;
		surface.desc.ddpfPixelFormat = pf;
		surface.desc.ddsCaps.dwCaps = caps;

		dd->CreateSurface(&dd, &surface.desc, &surface.surface.getRef(), nullptr);
		return surface;
	}

	void destroySmallerSurfaces(DWORD width, DWORD height, const DDPIXELFORMAT& pf,
		std::vector<Surface>& cachedSurfaces)
	{
		auto it = cachedSurfaces.begin();
		while (it != cachedSurfaces.end())
		{
			if (it->desc.dwWidth <= width && it->desc.dwHeight <= height &&
				0 == memcmp(&it->desc.ddpfPixelFormat, &pf, sizeof(pf)))
			{
				it->surface.release();
				it = cachedSurfaces.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	std::vector<Surface>::iterator findSurface(DWORD width, DWORD height, const DDPIXELFORMAT& pf,
		std::vector<Surface>& cachedSurfaces)
	{
		auto it = cachedSurfaces.begin();
		while (it != cachedSurfaces.end())
		{
			if (it->desc.dwWidth >= width && it->desc.dwHeight >= height &&
				0 == memcmp(&it->desc.ddpfPixelFormat, &pf, sizeof(pf)))
			{
				if (FAILED(it->surface->IsLost(it->surface)) &&
					FAILED(it->surface->Restore(it->surface)))
				{
					it->surface.release();
					it = cachedSurfaces.erase(it);
					continue;
				}
				return it;
			}
			++it;
		}

		return cachedSurfaces.end();
	}

	Repository& getRepository(CompatRef<IDirectDraw7> dd, DWORD caps)
	{
		if (caps & DDSCAPS_SYSTEMMEMORY)
		{
			g_sysMemRepo.dd = DDraw::Repository::getDirectDraw();
			return g_sysMemRepo;
		}

		void* ddObject = DDraw::getDdObject(dd.get());
		auto it = g_vidMemRepos.find(ddObject);
		if (it == g_vidMemRepos.end())
		{
			Repository repo = {};
			repo.dd = createDirectDraw().detach();
			return g_vidMemRepos[ddObject] = repo;
		}
		return it->second;
	}

	Surface getSurface(CompatRef<IDirectDraw7> dd, const DDSURFACEDESC2& desc)
	{
		Repository& repo = getRepository(dd, desc.ddsCaps.dwCaps);
		if (!repo.dd)
		{
			return Surface();
		}

		DDPIXELFORMAT pf = desc.ddpfPixelFormat;
		normalizePixelFormat(pf);

		auto it = findSurface(desc.dwWidth, desc.dwHeight, pf, repo.surfaces);
		if (it != repo.surfaces.end())
		{
			Surface cachedSurface = *it;
			repo.surfaces.erase(it);
			return cachedSurface;
		}

		const DWORD memFlag = (desc.ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY)
			? DDSCAPS_SYSTEMMEMORY : DDSCAPS_VIDEOMEMORY;
		Surface newSurface = createSurface(*repo.dd, desc.dwWidth, desc.dwHeight, pf,
			DDSCAPS_OFFSCREENPLAIN | memFlag);
		if (newSurface.surface)
		{
			newSurface.ddObject = DDraw::getDdObject(dd.get());
			destroySmallerSurfaces(desc.dwWidth, desc.dwHeight, pf, repo.surfaces);
		}
		return newSurface;
	}

	void normalizePixelFormat(DDPIXELFORMAT& pf)
	{
		if (!(pf.dwFlags & DDPF_FOURCC))
		{
			pf.dwFourCC = 0;
		}
		if (!(pf.dwFlags & (DDPF_ALPHAPIXELS | DDPF_ZPIXELS)))
		{
			pf.dwRGBAlphaBitMask = 0;
		}
	}

	void returnSurface(const Surface& surface)
	{
		if (!surface.surface)
		{
			return;
		}

		if (surface.desc.ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY)
		{
			g_sysMemRepo.surfaces.push_back(surface);
		}
		else
		{
			g_vidMemRepos[surface.ddObject].surfaces.push_back(surface);
		}
	}
}

namespace DDraw
{
	namespace Repository
	{
		ScopedSurface::ScopedSurface(CompatRef<IDirectDraw7> dd, const DDSURFACEDESC2& desc)
			: Surface(getSurface(dd, desc))
		{
			if (surface)
			{
				surface->SetColorKey(surface, DDCKEY_SRCBLT, nullptr);
				surface->SetColorKey(surface, DDCKEY_DESTBLT, nullptr);
			}
		}

		ScopedSurface::~ScopedSurface()
		{
			returnSurface(*this);
		}

		CompatWeakPtr<IDirectDraw7> getDirectDraw()
		{
			static auto dd = new CompatPtr<IDirectDraw7>(createDirectDraw());
			return *dd;
		}

		void onRelease(void* ddObject)
		{
			auto it = g_vidMemRepos.find(ddObject);
			if (it != g_vidMemRepos.end())
			{
				for (auto& surface : it->second.surfaces)
				{
					surface.surface->Release(surface.surface);
				}
				g_vidMemRepos.erase(it);
			}
		}
	}
}
