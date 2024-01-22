#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <DDrawCompat/DDrawLog.h>
#include <DDrawCompat/v0.3.1/D3dDdi/Log/DeviceCallbacksLog.h>

std::ostream& operator<<(std::ostream& os, const D3DDDI_ALLOCATIONINFO& data)
{
	return Compat32::LogStruct(os)
		<< Compat32::hex(data.hAllocation)
		<< data.pSystemMem
		<< data.pPrivateDriverData
		<< data.PrivateDriverDataSize
		<< data.VidPnSourceId
		<< Compat32::hex(data.Flags.Value);
}

std::ostream& operator<<(std::ostream& os, const D3DDDICB_ALLOCATE& data)
{
	return Compat32::LogStruct(os)
		<< data.pPrivateDriverData
		<< data.PrivateDriverDataSize
		<< data.hResource
		<< Compat32::hex(data.hKMResource)
		<< data.NumAllocations
		<< Compat32::array(data.pAllocationInfo, data.NumAllocations);
}

std::ostream& operator<<(std::ostream& os, const D3DKMT_CREATEALLOCATION& data)
{
	return Compat32::LogStruct(os)
		<< Compat32::hex(data.hDevice)
		<< Compat32::hex(data.hResource)
		<< Compat32::hex(data.hGlobalShare)
		<< data.pPrivateRuntimeData
		<< data.PrivateRuntimeDataSize
		<< data.pPrivateDriverData
		<< data.PrivateDriverDataSize
		<< data.NumAllocations
		<< Compat32::array(data.pAllocationInfo, data.NumAllocations)
		<< Compat32::hex(*reinterpret_cast<const DWORD*>(&data.Flags))
		<< data.hPrivateRuntimeResourceHandle;
}

std::ostream& operator<<(std::ostream& os, const D3DDDICB_DEALLOCATE& data)
{
	return Compat32::LogStruct(os)
		<< data.hResource
		<< data.NumAllocations
		<< Compat32::hex(Compat32::array(data.HandleList, data.NumAllocations));
}

std::ostream& operator<<(std::ostream& os, const D3DDDICB_DEALLOCATE2& data)
{
	return Compat32::LogStruct(os)
		<< data.hResource
		<< data.NumAllocations
		<< Compat32::hex(Compat32::array(data.HandleList, data.NumAllocations))
		<< Compat32::hex(data.Flags.Value);
}

std::ostream& operator<<(std::ostream& os, const D3DDDICB_LOCK& data)
{
	return Compat32::LogStruct(os)
		<< Compat32::hex(data.hAllocation)
		<< data.PrivateDriverData
		<< data.NumPages
		<< Compat32::array(data.pPages, data.NumPages)
		<< data.pData
		<< Compat32::hex(data.Flags.Value)
		<< Compat32::hex(data.GpuVirtualAddress);
}

std::ostream& operator<<(std::ostream& os, const D3DDDICB_LOCK2& data)
{
	return Compat32::LogStruct(os)
		<< Compat32::hex(data.hAllocation)
		<< Compat32::hex(data.Flags.Value)
		<< data.pData;
}

std::ostream& operator<<(std::ostream& os, const D3DDDICB_PRESENT& data)
{
	return Compat32::LogStruct(os)
		<< Compat32::hex(data.hSrcAllocation)
		<< Compat32::hex(data.hDstAllocation)
		<< data.hContext
		<< data.BroadcastContextCount
		<< Compat32::array(data.BroadcastContext, data.BroadcastContextCount)
		<< Compat32::hex(data.BroadcastSrcAllocation)
		<< Compat32::hex(data.BroadcastDstAllocation)
		<< data.PrivateDriverDataSize
		<< data.pPrivateDriverData
		<< static_cast<UINT>(data.bOptimizeForComposition);
}

std::ostream& operator<<(std::ostream& os, const D3DDDICB_UNLOCK& data)
{
	return Compat32::LogStruct(os)
		<< data.NumAllocations
		<< Compat32::hex(Compat32::array(data.phAllocations, data.NumAllocations));
}

std::ostream& operator<<(std::ostream& os, const D3DDDICB_UNLOCK2& data)
{
	return Compat32::LogStruct(os)
		<< Compat32::hex(data.hAllocation);
}
