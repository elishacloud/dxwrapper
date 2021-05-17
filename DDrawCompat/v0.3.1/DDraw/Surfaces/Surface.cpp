#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <initguid.h>

#include <DDrawCompat/v0.3.1/Common/CompatPtr.h>
#include <DDrawCompat/v0.3.1/D3dDdi/Device.h>
#include <DDrawCompat/v0.3.1/D3dDdi/Resource.h>
#include <DDrawCompat/v0.3.1/DDraw/DirectDrawClipper.h>
#include <DDrawCompat/v0.3.1/DDraw/DirectDrawSurface.h>
#include <DDrawCompat/v0.3.1/DDraw/Surfaces/Surface.h>
#include <DDrawCompat/v0.3.1/DDraw/Surfaces/SurfaceImpl.h>
#include <DDrawCompat/v0.3.1/Win32/DisplayMode.h>

// {C62D8849-DFAC-4454-A1E8-DA67446426BA}
DEFINE_GUID(IID_CompatSurfacePrivateData,
	0xc62d8849, 0xdfac, 0x4454, 0xa1, 0xe8, 0xda, 0x67, 0x44, 0x64, 0x26, 0xba);

namespace DDraw
{
	HRESULT STDMETHODCALLTYPE Surface::QueryInterface(REFIID, LPVOID*)
	{
		return E_NOINTERFACE;
	}

	ULONG STDMETHODCALLTYPE Surface::AddRef()
	{
		return ++m_refCount;
	}

	ULONG STDMETHODCALLTYPE Surface::Release()
	{
		DWORD refCount = --m_refCount;
		if (0 == refCount)
		{
			delete this;
		}
		return refCount;
	}

	Surface::Surface(DWORD origCaps)
		: m_origCaps(origCaps)
		, m_refCount(0)
		, m_sizeOverride{}
	{
	}

	Surface::~Surface()
	{
		DirectDrawClipper::setClipper(*this, nullptr);
	}

	void Surface::attach(CompatRef<IDirectDrawSurface7> dds, std::unique_ptr<Surface> privateData)
	{
		if (SUCCEEDED(dds->SetPrivateData(&dds, IID_CompatSurfacePrivateData,
			privateData.get(), sizeof(privateData.get()), DDSPD_IUNKNOWNPOINTER)))
		{
			privateData->createImpl();
			privateData->m_surface = &dds;
			privateData.release();
		}
	}

	template <typename TDirectDraw, typename TSurface, typename TSurfaceDesc>
	HRESULT Surface::create(
		CompatRef<TDirectDraw> dd, TSurfaceDesc desc, TSurface*& surface, std::unique_ptr<Surface> privateData)
	{
		if ((desc.ddsCaps.dwCaps & DDSCAPS_3DDEVICE) &&
			((desc.dwFlags & DDSD_PIXELFORMAT) && (desc.ddpfPixelFormat.dwRGBBitCount <= 8)) ||
			(!(desc.dwFlags & DDSD_PIXELFORMAT) && Win32::DisplayMode::getBpp() <= 8))
		{
			desc.ddsCaps.dwCaps &= ~DDSCAPS_3DDEVICE;
		}

		HRESULT result = dd->CreateSurface(&dd, &desc, &surface, nullptr);
		if (FAILED(result))
		{
			return result;
		}

		auto surface7(CompatPtr<IDirectDrawSurface7>::from(surface));
		if (desc.ddsCaps.dwCaps & DDSCAPS_COMPLEX)
		{
			auto attachedSurfaces(DirectDrawSurface::getAllAttachedSurfaces(*surface7));
			for (DWORD i = 0; i < attachedSurfaces.size(); ++i)
			{
				attach(*attachedSurfaces[i], std::make_unique<Surface>(privateData->m_origCaps));
			}
		}

		attach(*surface7, std::move(privateData));

		return result;
	}

	template HRESULT Surface::create(
		CompatRef<IDirectDraw> dd, DDSURFACEDESC desc, IDirectDrawSurface*& surface, std::unique_ptr<Surface> privateData);
	template HRESULT Surface::create(
		CompatRef<IDirectDraw2> dd, DDSURFACEDESC desc, IDirectDrawSurface*& surface, std::unique_ptr<Surface> privateData);
	template HRESULT Surface::create(
		CompatRef<IDirectDraw4> dd, DDSURFACEDESC2 desc, IDirectDrawSurface4*& surface, std::unique_ptr<Surface> privateData);
	template HRESULT Surface::create(
		CompatRef<IDirectDraw7> dd, DDSURFACEDESC2 desc, IDirectDrawSurface7*& surface, std::unique_ptr<Surface> privateData);

	void Surface::createImpl()
	{
		m_impl.reset(new SurfaceImpl<IDirectDrawSurface>(this));
		m_impl2.reset(new SurfaceImpl<IDirectDrawSurface2>(this));
		m_impl3.reset(new SurfaceImpl<IDirectDrawSurface3>(this));
		m_impl4.reset(new SurfaceImpl<IDirectDrawSurface4>(this));
		m_impl7.reset(new SurfaceImpl<IDirectDrawSurface7>(this));
	}

	template <>
	SurfaceImpl<IDirectDrawSurface>* Surface::getImpl<IDirectDrawSurface>() const { return m_impl.get(); }
	template <>
	SurfaceImpl<IDirectDrawSurface2>* Surface::getImpl<IDirectDrawSurface2>() const { return m_impl2.get(); }
	template <>
	SurfaceImpl<IDirectDrawSurface3>* Surface::getImpl<IDirectDrawSurface3>() const { return m_impl3.get(); }
	template <>
	SurfaceImpl<IDirectDrawSurface4>* Surface::getImpl<IDirectDrawSurface4>() const { return m_impl4.get(); }
	template <>
	SurfaceImpl<IDirectDrawSurface7>* Surface::getImpl<IDirectDrawSurface7>() const { return m_impl7.get(); }

	template <typename TSurface>
	Surface* Surface::getSurface(TSurface& dds)
	{
		Surface* surface = nullptr;
		DWORD surfacePtrSize = sizeof(surface);

		// This can get called during surface release so a proper QueryInterface would be dangerous
		CompatVtable<IDirectDrawSurface7Vtbl>::s_origVtable.GetPrivateData(
			reinterpret_cast<IDirectDrawSurface7*>(&dds),
			IID_CompatSurfacePrivateData, &surface, &surfacePtrSize);

		return surface;
	}

	template Surface* Surface::getSurface(IDirectDrawSurface& dds);
	template Surface* Surface::getSurface(IDirectDrawSurface2& dds);
	template Surface* Surface::getSurface(IDirectDrawSurface3& dds);
	template Surface* Surface::getSurface(IDirectDrawSurface4& dds);
	template Surface* Surface::getSurface(IDirectDrawSurface7& dds);

	void Surface::restore()
	{
	}

	void Surface::setSizeOverride(DWORD width, DWORD height)
	{
		m_sizeOverride.cx = width;
		m_sizeOverride.cy = height;
	}
}
