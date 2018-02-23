#include <algorithm>
#include <vector>

#include "CompatDirectDraw.h"
#include "CompatDirectDrawSurface.h"
#include "DDrawLog.h"
#include "DDrawProcs.h"
#include "DDrawRepository.h"

namespace Compat21
{
	namespace
	{
		using DDrawRepository::Surface;

		static std::vector<Surface> g_sysMemSurfaces;
		static std::vector<Surface> g_vidMemSurfaces;

		IDirectDraw7* createDirectDraw();
		Surface createSurface(DWORD width, DWORD height, const DDPIXELFORMAT& pf, DWORD caps);
		std::vector<Surface>::iterator findSurface(DWORD width, DWORD height, const DDPIXELFORMAT& pf,
			std::vector<Surface>& cachedSurfaces);
		void destroySmallerSurfaces(DWORD width, DWORD height, const DDPIXELFORMAT& pf,
			std::vector<Surface>& cachedSurfaces);
		Surface getSurface(const DDSURFACEDESC2& desc);
		void normalizePixelFormat(DDPIXELFORMAT& pf);
		void returnSurface(const Surface& surface);

		IDirectDraw7* createDirectDraw()
		{
			IDirectDraw7* dd = nullptr;
			HRESULT result = CALL_ORIG_DDRAW(DirectDrawCreateEx, nullptr,
				reinterpret_cast<void**>(&dd), IID_IDirectDraw7, nullptr);
			if (FAILED(result))
			{
				Logging::Log() << "Failed to create a DirectDraw object in the repository: " << result;
				return nullptr;
			}

			result = dd->lpVtbl->SetCooperativeLevel(dd, nullptr, DDSCL_NORMAL);
			if (FAILED(result))
			{
				Logging::Log() << "Failed to set the cooperative level in the repository: " << result;
				dd->lpVtbl->Release(dd);
				return nullptr;
			}

			return dd;
		}

		Surface createSurface(DWORD width, DWORD height, const DDPIXELFORMAT& pf, DWORD caps)
		{
			Surface surface = {};
			IDirectDraw7* dd = DDrawRepository::getDirectDraw();
			if (!dd)
			{
				return surface;
			}

			surface.desc.dwSize = sizeof(surface.desc);
			surface.desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT | DDSD_CAPS;
			surface.desc.dwWidth = width;
			surface.desc.dwHeight = height;
			surface.desc.ddpfPixelFormat = pf;
			surface.desc.ddsCaps.dwCaps = caps;

			CompatDirectDraw<IDirectDraw7>::s_origVtable.CreateSurface(
				dd, &surface.desc, &surface.surface, nullptr);
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
					CompatDirectDrawSurface<IDirectDrawSurface7>::s_origVtable.Release(it->surface);
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
			auto& origVtable = CompatDirectDrawSurface<IDirectDrawSurface7>::s_origVtable;

			auto it = cachedSurfaces.begin();
			while (it != cachedSurfaces.end())
			{
				if (it->desc.dwWidth >= width && it->desc.dwHeight >= height &&
					0 == memcmp(&it->desc.ddpfPixelFormat, &pf, sizeof(pf)))
				{
					if (FAILED(origVtable.IsLost(it->surface)) && FAILED(origVtable.Restore(it->surface)))
					{
						origVtable.Release(it->surface);
						it = cachedSurfaces.erase(it);
						continue;
					}
					return it;
				}
				++it;
			}

			return cachedSurfaces.end();
		}

		Surface getSurface(const DDSURFACEDESC2& desc)
		{
			std::vector<Surface>& cachedSurfaces =
				(desc.ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY) ? g_sysMemSurfaces : g_vidMemSurfaces;

			DDPIXELFORMAT pf = desc.ddpfPixelFormat;
			normalizePixelFormat(pf);

			auto it = findSurface(desc.dwWidth, desc.dwHeight, pf, cachedSurfaces);
			if (it != cachedSurfaces.end())
			{
				Surface cachedSurface = *it;
				cachedSurfaces.erase(it);
				return cachedSurface;
			}

			Surface newSurface = createSurface(desc.dwWidth, desc.dwHeight, pf,
				DDSCAPS_OFFSCREENPLAIN | (desc.ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY));
			if (newSurface.surface)
			{
				destroySmallerSurfaces(desc.dwWidth, desc.dwHeight, pf, cachedSurfaces);
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
				g_sysMemSurfaces.push_back(surface);
			}
			else
			{
				g_vidMemSurfaces.push_back(surface);
			}
		}
	}

	namespace DDrawRepository
	{
		ScopedSurface::ScopedSurface(const DDSURFACEDESC2& desc)
			: Surface(getSurface(desc))
		{
		}

		ScopedSurface::~ScopedSurface()
		{
			returnSurface(*this);
		}

		IDirectDraw7* getDirectDraw()
		{
			static IDirectDraw7* dd = createDirectDraw();
			return dd;
		}
	}
}
