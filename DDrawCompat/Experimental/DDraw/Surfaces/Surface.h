#pragma once

#define CINTERFACE

#include <memory>

#include <ddraw.h>

#include "Common/CompatPtr.h"
#include "Common/CompatRef.h"

namespace DDraw
{
	template <typename TSurface> class SurfaceImpl;
	template <typename TSurface> class SurfaceImpl2;

	class Surface
	{
	public:
		virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, LPVOID*);
		virtual ULONG STDMETHODCALLTYPE AddRef();
		virtual ULONG STDMETHODCALLTYPE Release();

		virtual ~Surface();

		template <typename TDirectDraw, typename TSurface, typename TSurfaceDesc>
		static HRESULT create(CompatRef<TDirectDraw> dd, TSurfaceDesc desc, TSurface*& surface);

		template <typename TSurface>
		static Surface* getSurface(TSurface& dds);

		template <typename TSurface>
		SurfaceImpl<TSurface>* getImpl() const;

	protected:
		Surface();

		static void attach(CompatRef<IDirectDrawSurface7> dds, std::unique_ptr<Surface>& privateData);

		void* m_ddObject;
		std::unique_ptr<SurfaceImpl<IDirectDrawSurface>> m_impl;
		std::unique_ptr<SurfaceImpl<IDirectDrawSurface2>> m_impl2;
		std::unique_ptr<SurfaceImpl<IDirectDrawSurface3>> m_impl3;
		std::unique_ptr<SurfaceImpl<IDirectDrawSurface4>> m_impl4;
		std::unique_ptr<SurfaceImpl<IDirectDrawSurface7>> m_impl7;

	private:
		template <typename TDirectDrawSurface>
		friend class SurfaceImpl2;

		static HRESULT WINAPI attachToLinkedSurfaces(
			IDirectDrawSurface7* surface, DDSURFACEDESC2* desc, void* rootSurface);
		virtual void createImpl();

		IID m_ddId;
		DWORD m_refCount;
	};
}
