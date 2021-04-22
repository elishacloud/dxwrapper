#pragma once

#include <memory>
#include <vector>

#include <d3d.h>
#include <d3dumddi.h>

#include <D3dDdi/FormatInfo.h>

namespace D3dDdi
{
	class Device;

	class Resource
	{
	public:
		Resource(Device& device, D3DDDIARG_CREATERESOURCE& data);
		Resource(Device& device, D3DDDIARG_CREATERESOURCE2& data);

		Resource(const Resource&) = delete;
		Resource& operator=(const Resource&) = delete;

		Resource(Resource&&) = default;
		Resource& operator=(Resource&&) = default;

		operator HANDLE() const { return m_handle; }

		HRESULT blt(D3DDDIARG_BLT data);
		HRESULT colorFill(D3DDDIARG_COLORFILL data);
		void* getLockPtr(UINT subResourceIndex);
		HRESULT lock(D3DDDIARG_LOCK& data);
		void prepareForGdiRendering(bool isReadOnly);
		void prepareForRendering(UINT subResourceIndex, bool isReadOnly);
		void setAsGdiResource(bool isGdiResource);
		HRESULT unlock(const D3DDDIARG_UNLOCK& data);

	private:
		class Data : public D3DDDIARG_CREATERESOURCE2
		{
		public:
			Data();
			Data(const D3DDDIARG_CREATERESOURCE& data);
			Data(const D3DDDIARG_CREATERESOURCE2& data);

			Data(const Data&) = delete;
			Data& operator=(const Data&) = delete;

			Data(Data&&) = default;
			Data& operator=(Data&&) = default;

			std::vector<D3DDDI_SURFACEINFO> surfaceData;
		};

		struct LockData
		{
			void* data;
			UINT pitch;
			UINT lockCount;
			long long qpcLastForcedLock;
			bool isSysMemUpToDate;
			bool isVidMemUpToDate;
		};

		class ResourceDeleter
		{
		public:
			ResourceDeleter(Device& device) : m_device(device) {}
			void operator()(HANDLE resource) { m_device.getOrigVtable().pfnDestroyResource(m_device, resource); }

		private:
			Device& m_device;
		};

		template <typename Arg>
		Resource(Device& device, Arg& data, HRESULT(APIENTRY *createResourceFunc)(HANDLE, Arg*));

		HRESULT bltLock(D3DDDIARG_LOCK& data);
		HRESULT bltUnlock(const D3DDDIARG_UNLOCK& data);
		void clipRect(UINT subResourceIndex, RECT& rect);
		HRESULT copySubResource(HANDLE dstResource, HANDLE srcResource, UINT subResourceIndex);
		void copyToSysMem(UINT subResourceIndex);
		void copyToVidMem(UINT subResourceIndex);
		void createGdiLockResource();
		void createLockResource();
		void createSysMemResource(const std::vector<D3DDDI_SURFACEINFO>& surfaceInfo);
		bool isOversized() const;
		bool isValidRect(UINT subResourceIndex, const RECT& rect);
		HRESULT presentationBlt(const D3DDDIARG_BLT& data, Resource& srcResource);
		HRESULT splitBlt(D3DDDIARG_BLT& data, UINT& subResourceIndex, RECT& rect, RECT& otherRect);

		template <typename Arg>
		HRESULT splitLock(Arg& data, HRESULT(APIENTRY *lockFunc)(HANDLE, Arg*));

		HRESULT sysMemPreferredBlt(const D3DDDIARG_BLT& data, Resource& srcResource);

		Device& m_device;
		HANDLE m_handle;
		Data m_origData;
		Data m_fixedData;
		FormatInfo m_formatInfo;
		std::unique_ptr<void, void(*)(void*)> m_lockBuffer;
		std::vector<LockData> m_lockData;
		std::unique_ptr<void, ResourceDeleter> m_lockResource;
	};
}
