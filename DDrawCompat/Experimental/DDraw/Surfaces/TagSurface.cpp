#include <algorithm>
#include <map>

#include "DDraw/DirectDraw.h"
#include "DDraw/Repository.h"
#include "DDraw/Surfaces/TagSurface.h"

namespace
{
	std::map<void*, DDraw::TagSurface*> g_tagSurfaces;
}

namespace DDraw
{
	TagSurface::~TagSurface()
	{
		Repository::onRelease(m_ddObject);
		g_tagSurfaces.erase(std::find_if(g_tagSurfaces.begin(), g_tagSurfaces.end(),
			[=](auto& i) { return i.second == this; }));
	}

	HRESULT TagSurface::create(CompatRef<IDirectDraw> dd)
	{
		DDSURFACEDESC desc = {};
		desc.dwSize = sizeof(desc);
		desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
		desc.dwWidth = 1;
		desc.dwHeight = 1;
		desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;

		IDirectDrawSurface* surface = nullptr;
		HRESULT result = Surface::create(dd, desc, surface);
		if (SUCCEEDED(result))
		{
			std::unique_ptr<Surface> privateData(new TagSurface());
			g_tagSurfaces[getDdObject(dd.get())] = static_cast<TagSurface*>(privateData.get());
			CompatPtr<IDirectDrawSurface7> surface7(Compat::queryInterface<IDirectDrawSurface7>(surface));
			attach(*surface7, privateData);
		}
		return result;
	}

	TagSurface* TagSurface::get(void* ddObject)
	{
		auto it = g_tagSurfaces.find(ddObject);
		return it != g_tagSurfaces.end() ? it->second : nullptr;
	}
}
