#pragma once

#include <d3d.h>
#include <d3dumddi.h>

#include <Common/VtableVisitor.h>

template <>
struct VtableForEach<D3DDDI_DEVICECALLBACKS>
{
	template <typename Vtable, typename Visitor>
	static void forEach(Visitor& visitor, UINT version)
	{
		DD_VISIT(pfnAllocateCb);
		DD_VISIT(pfnDeallocateCb);
		DD_VISIT(pfnSetPriorityCb);
		DD_VISIT(pfnQueryResidencyCb);
		DD_VISIT(pfnSetDisplayModeCb);
		DD_VISIT(pfnPresentCb);
		DD_VISIT(pfnRenderCb);
		DD_VISIT(pfnLockCb);
		DD_VISIT(pfnUnlockCb);
		DD_VISIT(pfnEscapeCb);
		DD_VISIT(pfnCreateOverlayCb);
		DD_VISIT(pfnUpdateOverlayCb);
		DD_VISIT(pfnFlipOverlayCb);
		DD_VISIT(pfnDestroyOverlayCb);
		DD_VISIT(pfnCreateContextCb);
		DD_VISIT(pfnDestroyContextCb);
		DD_VISIT(pfnCreateSynchronizationObjectCb);
		DD_VISIT(pfnDestroySynchronizationObjectCb);
		DD_VISIT(pfnWaitForSynchronizationObjectCb);
		DD_VISIT(pfnSignalSynchronizationObjectCb);
		DD_VISIT(pfnSetAsyncCallbacksCb);
		DD_VISIT(pfnSetDisplayPrivateDriverFormatCb);

		if (version >= D3D_UMD_INTERFACE_VERSION_WIN8)
		{
			DD_VISIT(pfnOfferAllocationsCb);
			DD_VISIT(pfnReclaimAllocationsCb);
			DD_VISIT(pfnCreateSynchronizationObject2Cb);
			DD_VISIT(pfnWaitForSynchronizationObject2Cb);
			DD_VISIT(pfnSignalSynchronizationObject2Cb);
			DD_VISIT(pfnPresentMultiPlaneOverlayCb);
		}

		if (version >= D3D_UMD_INTERFACE_VERSION_WDDM1_3)
		{
			DD_VISIT(pfnLogUMDMarkerCb);
		}

		if (version >= D3D_UMD_INTERFACE_VERSION_WDDM2_0)
		{
			DD_VISIT(pfnMakeResidentCb);
			DD_VISIT(pfnEvictCb);
			DD_VISIT(pfnWaitForSynchronizationObjectFromCpuCb);
			DD_VISIT(pfnSignalSynchronizationObjectFromCpuCb);
			DD_VISIT(pfnWaitForSynchronizationObjectFromGpuCb);
			DD_VISIT(pfnSignalSynchronizationObjectFromGpuCb);
			DD_VISIT(pfnCreatePagingQueueCb);
			DD_VISIT(pfnDestroyPagingQueueCb);
			DD_VISIT(pfnLock2Cb);
			DD_VISIT(pfnUnlock2Cb);
			DD_VISIT(pfnInvalidateCacheCb);
			DD_VISIT(pfnReserveGpuVirtualAddressCb);
			DD_VISIT(pfnMapGpuVirtualAddressCb);
			DD_VISIT(pfnFreeGpuVirtualAddressCb);
			DD_VISIT(pfnUpdateGpuVirtualAddressCb);
			DD_VISIT(pfnCreateContextVirtualCb);
			DD_VISIT(pfnSubmitCommandCb);
			DD_VISIT(pfnDeallocate2Cb);
			DD_VISIT(pfnSignalSynchronizationObjectFromGpu2Cb);
			DD_VISIT(pfnReclaimAllocations2Cb);
			DD_VISIT(pfnGetResourcePresentPrivateDriverDataCb);
		}

		if (version >= D3D_UMD_INTERFACE_VERSION_WDDM2_1_1)
		{
			DD_VISIT(pfnUpdateAllocationPropertyCb);
			DD_VISIT(pfnOfferAllocations2Cb);
		}

		if (version >= D3D_UMD_INTERFACE_VERSION_WDDM2_1_2)
		{
			DD_VISIT(pfnReclaimAllocations3Cb);
			DD_VISIT(pfnAcquireResourceCb);
		}

		if (version >= D3D_UMD_INTERFACE_VERSION_WDDM2_1_3)
		{
			DD_VISIT(pfnReleaseResourceCb);
		}

		if (version >= D3D_UMD_INTERFACE_VERSION_WDDM2_2_1)
		{
			DD_VISIT(pfnCreateHwContextCb);
			DD_VISIT(pfnDestroyHwContextCb);
			DD_VISIT(pfnCreateHwQueueCb);
			DD_VISIT(pfnDestroyHwQueueCb);
			DD_VISIT(pfnSubmitCommandToHwQueueCb);
			DD_VISIT(pfnSubmitWaitForSyncObjectsToHwQueueCb);
			DD_VISIT(pfnSubmitSignalSyncObjectsToHwQueueCb);
		}

		if (version >= D3D_UMD_INTERFACE_VERSION_WDDM2_4_2)
		{
			DD_VISIT(pfnSubmitPresentBltToHwQueueCb);
		}

		if (version >= D3D_UMD_INTERFACE_VERSION_WDDM2_5_2)
		{
			DD_VISIT(pfnSubmitPresentToHwQueueCb);
		}
	}
};
