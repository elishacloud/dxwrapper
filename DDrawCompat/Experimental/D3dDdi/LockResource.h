#pragma once

#define CINTERFACE

#include <vector>

#include <d3d.h>
#include <d3dumddi.h>

namespace D3dDdi
{
	class LockResource
	{
	public:
		struct SubResource
		{
		public:
			SubResource(LockResource& parent, UINT index, UINT width, UINT height);

			LockResource& getParent() const { return *m_parent; }
			void updateLock();
			void updateOrig();

			bool m_isOrigUpToDate;
			bool m_isLockUpToDate;

		private:
			HRESULT blt(HANDLE dstResource, HANDLE srcResource);

			LockResource* m_parent;
			UINT m_index;
			UINT m_width;
			UINT m_height;
		};

		LockResource(HANDLE device, HANDLE origResource, HANDLE lockResource,
			const D3DDDI_SURFACEINFO* surfaceInfo, UINT surfaceInfoCount);
		~LockResource();
		LockResource(const LockResource&) = delete;

		HANDLE getHandle() const { return m_lockResource; }
		SubResource& getSubResource(UINT index) { return m_subResources[index]; }

	private:
		HANDLE m_device;
		HANDLE m_origResource;
		HANDLE m_lockResource;
		std::vector<SubResource> m_subResources;
	};
}
