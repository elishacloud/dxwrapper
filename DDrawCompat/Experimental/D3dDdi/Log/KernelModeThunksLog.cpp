#include "DDrawLog.h"
#include "D3dDdi/Log/KernelModeThunksLog.h"

std::ostream& operator<<(std::ostream& os, const D3DKMT_CREATECONTEXT& data)
{
	return Compat::LogStruct(os)
		<< Compat::hex(data.hDevice)
		<< data.NodeOrdinal
		<< data.EngineAffinity
		<< Compat::hex(data.Flags.Value)
		<< data.pPrivateDriverData
		<< data.PrivateDriverDataSize
		<< data.ClientHint
		<< Compat::hex(data.hContext)
		<< data.pCommandBuffer
		<< data.CommandBufferSize
		<< Compat::out(Compat::array(data.pAllocationList, data.AllocationListSize))
		<< data.AllocationListSize
		<< Compat::out(Compat::array(data.pPatchLocationList, data.PatchLocationListSize))
		<< data.PatchLocationListSize
		<< Compat::hex(data.CommandBuffer);
}

std::ostream& operator<<(std::ostream& os, const D3DKMT_CREATECONTEXTVIRTUAL& data)
{
	return Compat::LogStruct(os)
		<< Compat::hex(data.hDevice)
		<< data.NodeOrdinal
		<< data.EngineAffinity
		<< Compat::hex(data.Flags.Value)
		<< data.pPrivateDriverData
		<< data.PrivateDriverDataSize
		<< data.ClientHint
		<< Compat::hex(data.hContext);
}

std::ostream& operator<<(std::ostream& os, const D3DKMT_CREATEDEVICE& data)
{
	return Compat::LogStruct(os)
		<< Compat::hex(data.hAdapter)
		<< Compat::hex(*reinterpret_cast<const DWORD*>(&data.Flags))
		<< Compat::hex(data.hDevice)
		<< data.pCommandBuffer
		<< data.CommandBufferSize
		<< Compat::out(Compat::array(data.pAllocationList, data.AllocationListSize))
		<< data.AllocationListSize
		<< Compat::out(Compat::array(data.pPatchLocationList, data.PatchLocationListSize))
		<< data.PatchLocationListSize;
}

std::ostream& operator<<(std::ostream& os, const D3DKMT_DESTROYCONTEXT& data)
{
	return Compat::LogStruct(os)
		<< Compat::hex(data.hContext);
}

std::ostream& operator<<(std::ostream& os, const D3DKMT_DESTROYDEVICE& data)
{
	return Compat::LogStruct(os)
		<< Compat::hex(data.hDevice);
}

std::ostream& operator<<(std::ostream& os, const D3DKMT_PRESENT& data)
{
	return Compat::LogStruct(os)
		<< Compat::hex(data.hDevice)
		<< data.hWindow
		<< data.VidPnSourceId
		<< Compat::hex(data.hSource)
		<< Compat::hex(data.hDestination)
		<< Compat::hex(data.Color)
		<< data.DstRect
		<< data.SrcRect
		<< data.SubRectCnt
		<< Compat::array(data.pSrcSubRects, data.SubRectCnt)
		<< data.PresentCount
		<< data.FlipInterval
		<< Compat::hex(data.Flags.Value)
		<< data.BroadcastContextCount
		<< Compat::hex(Compat::array(data.BroadcastContext, data.BroadcastContextCount))
		<< data.PresentLimitSemaphore
		<< data.PresentHistoryToken
		<< data.pPresentRegions
		<< Compat::hex(data.hAdapter)
		<< data.Duration
		<< Compat::hex(Compat::array(data.BroadcastSrcAllocation, data.BroadcastContextCount))
		<< Compat::hex(Compat::array(data.BroadcastDstAllocation, data.BroadcastContextCount))
		<< data.PrivateDriverDataSize
		<< data.pPrivateDriverData
		<< static_cast<UINT>(data.bOptimizeForComposition);
}

std::ostream& operator<<(std::ostream& os, const D3DKMT_SETQUEUEDLIMIT& data)
{
	return Compat::LogStruct(os)
		<< Compat::hex(data.hDevice)
		<< data.Type
		<< data.VidPnSourceId
		<< data.QueuedPendingFlipLimit;
}

std::ostream& operator<<(std::ostream& os, const D3DKMT_SETVIDPNSOURCEOWNER& data)
{
	return Compat::LogStruct(os)
		<< Compat::hex(data.hDevice)
		<< Compat::array(data.pType, data.VidPnSourceCount)
		<< Compat::array(data.pVidPnSourceId, data.VidPnSourceCount)
		<< data.VidPnSourceCount;
}

std::ostream& operator<<(std::ostream& os, const D3DKMT_SETVIDPNSOURCEOWNER1& data)
{
	return Compat::LogStruct(os)
		<< data.Version0
		<< Compat::hex(data.Flags.Value);
}
