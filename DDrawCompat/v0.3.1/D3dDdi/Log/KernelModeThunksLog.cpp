#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <DDrawCompat/DDrawLog.h>
#include <DDrawCompat/v0.3.1/D3dDdi/Log/KernelModeThunksLog.h>

std::ostream& operator<<(std::ostream& os, const LUID& luid)
{
	return Compat31::LogStruct(os)
		<< Compat31::hex(luid.LowPart)
		<< Compat31::hex(luid.HighPart);
}

std::ostream& operator<<(std::ostream& os, const D3DKMT_CREATECONTEXT& data)
{
	return Compat31::LogStruct(os)
		<< Compat31::hex(data.hDevice)
		<< data.NodeOrdinal
		<< data.EngineAffinity
		<< Compat31::hex(data.Flags.Value)
		<< data.pPrivateDriverData
		<< data.PrivateDriverDataSize
		<< data.ClientHint
		<< Compat31::hex(data.hContext)
		<< data.pCommandBuffer
		<< data.CommandBufferSize
		<< Compat31::out(Compat31::array(data.pAllocationList, data.AllocationListSize))
		<< data.AllocationListSize
		<< Compat31::out(Compat31::array(data.pPatchLocationList, data.PatchLocationListSize))
		<< data.PatchLocationListSize
		<< Compat31::hex(data.CommandBuffer);
}

std::ostream& operator<<(std::ostream& os, const D3DKMT_CREATECONTEXTVIRTUAL& data)
{
	return Compat31::LogStruct(os)
		<< Compat31::hex(data.hDevice)
		<< data.NodeOrdinal
		<< data.EngineAffinity
		<< Compat31::hex(data.Flags.Value)
		<< data.pPrivateDriverData
		<< data.PrivateDriverDataSize
		<< data.ClientHint
		<< Compat31::hex(data.hContext);
}

std::ostream& operator<<(std::ostream& os, const D3DKMT_CREATEDCFROMMEMORY& data)
{
	return Compat31::LogStruct(os)
		<< data.pMemory
		<< data.Format
		<< data.Width
		<< data.Height
		<< data.Pitch
		<< data.hDeviceDc
		<< data.pColorTable
		<< Compat31::out(data.hDc)
		<< Compat31::out(data.hBitmap);
}

std::ostream& operator<<(std::ostream& os, const D3DKMT_CREATEDEVICE& data)
{
	return Compat31::LogStruct(os)
		<< Compat31::hex(data.hAdapter)
		<< Compat31::hex(*reinterpret_cast<const DWORD*>(&data.Flags))
		<< Compat31::hex(data.hDevice)
		<< data.pCommandBuffer
		<< data.CommandBufferSize
		<< Compat31::out(Compat31::array(data.pAllocationList, data.AllocationListSize))
		<< data.AllocationListSize
		<< Compat31::out(Compat31::array(data.pPatchLocationList, data.PatchLocationListSize))
		<< data.PatchLocationListSize;
}

std::ostream& operator<<(std::ostream& os, const D3DKMT_DESTROYCONTEXT& data)
{
	return Compat31::LogStruct(os)
		<< Compat31::hex(data.hContext);
}

std::ostream& operator<<(std::ostream& os, const D3DKMT_DESTROYDEVICE& data)
{
	return Compat31::LogStruct(os)
		<< Compat31::hex(data.hDevice);
}

std::ostream& operator<<(std::ostream& os, const D3DKMT_OPENADAPTERFROMHDC& data)
{
	return Compat31::LogStruct(os)
		<< data.hDc
		<< Compat31::hex(data.hAdapter)
		<< data.AdapterLuid
		<< data.VidPnSourceId;
}

std::ostream& operator<<(std::ostream& os, const D3DKMT_PRESENT& data)
{
	return Compat31::LogStruct(os)
		<< Compat31::hex(data.hDevice)
		<< data.hWindow
		<< data.VidPnSourceId
		<< Compat31::hex(data.hSource)
		<< Compat31::hex(data.hDestination)
		<< Compat31::hex(data.Color)
		<< data.DstRect
		<< data.SrcRect
		<< data.SubRectCnt
		<< Compat31::array(data.pSrcSubRects, data.SubRectCnt)
		<< data.PresentCount
		<< data.FlipInterval
		<< Compat31::hex(data.Flags.Value)
		<< data.BroadcastContextCount
		<< Compat31::hex(Compat31::array(data.BroadcastContext, data.BroadcastContextCount))
		<< data.PresentLimitSemaphore
		<< data.PresentHistoryToken
		<< data.pPresentRegions
		<< Compat31::hex(data.hAdapter)
		<< data.Duration
		<< Compat31::hex(Compat31::array(data.BroadcastSrcAllocation, data.BroadcastContextCount))
		<< Compat31::hex(Compat31::array(data.BroadcastDstAllocation, data.BroadcastContextCount))
		<< data.PrivateDriverDataSize
		<< data.pPrivateDriverData
		<< static_cast<UINT>(data.bOptimizeForComposition);
}

std::ostream& operator<<(std::ostream& os, const D3DKMT_QUERYADAPTERINFO& data)
{
	return Compat31::LogStruct(os)
		<< Compat31::hex(data.hAdapter)
		<< data.Type
		<< data.pPrivateDriverData
		<< data.PrivateDriverDataSize;
}

std::ostream& operator<<(std::ostream& os, const D3DKMT_SETQUEUEDLIMIT& data)
{
	return Compat31::LogStruct(os)
		<< Compat31::hex(data.hDevice)
		<< data.Type
		<< data.VidPnSourceId
		<< data.QueuedPendingFlipLimit;
}

std::ostream& operator<<(std::ostream& os, const D3DKMT_SETVIDPNSOURCEOWNER& data)
{
	return Compat31::LogStruct(os)
		<< Compat31::hex(data.hDevice)
		<< Compat31::array(data.pType, data.VidPnSourceCount)
		<< Compat31::array(data.pVidPnSourceId, data.VidPnSourceCount)
		<< data.VidPnSourceCount;
}

std::ostream& operator<<(std::ostream& os, const D3DKMT_SETVIDPNSOURCEOWNER1& data)
{
	return Compat31::LogStruct(os)
		<< data.Version0
		<< Compat31::hex(data.Flags.Value);
}
