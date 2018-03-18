#include "DDrawCompat\DDrawLog.h"
#include "D3dDdi/Log/DeviceCallbacksLog.h"

std::ostream& operator<<(std::ostream& os, const D3DDDI_ALLOCATIONINFO& data)
{
	return Compat::LogStruct(os)
		<< Compat::hex(data.hAllocation)
		<< data.pSystemMem
		<< data.pPrivateDriverData
		<< data.PrivateDriverDataSize
		<< data.VidPnSourceId
		<< Compat::hex(data.Flags.Value);
}

std::ostream& operator<<(std::ostream& os, const D3DDDICB_ALLOCATE& data)
{
	return Compat::LogStruct(os)
		<< data.pPrivateDriverData
		<< data.PrivateDriverDataSize
		<< data.hResource
		<< Compat::hex(data.hKMResource)
		<< data.NumAllocations
		<< Compat::array(data.pAllocationInfo, data.NumAllocations);
}

std::ostream& operator<<(std::ostream& os, const D3DKMT_CREATEALLOCATION& data)
{
	return Compat::LogStruct(os)
		<< Compat::hex(data.hDevice)
		<< Compat::hex(data.hResource)
		<< Compat::hex(data.hGlobalShare)
		<< data.pPrivateRuntimeData
		<< data.PrivateRuntimeDataSize
		<< data.pPrivateDriverData
		<< data.PrivateDriverDataSize
		<< data.NumAllocations
		<< Compat::array(data.pAllocationInfo, data.NumAllocations)
		<< Compat::hex(*reinterpret_cast<const DWORD*>(&data.Flags))
		<< data.hPrivateRuntimeResourceHandle;
}

std::ostream& operator<<(std::ostream& os, const D3DDDICB_DEALLOCATE& data)
{
	return Compat::LogStruct(os)
		<< data.hResource
		<< data.NumAllocations
		<< Compat::hex(Compat::array(data.HandleList, data.NumAllocations));
}

std::ostream& operator<<(std::ostream& os, const D3DDDICB_DEALLOCATE2& data)
{
	return Compat::LogStruct(os)
		<< data.hResource
		<< data.NumAllocations
		<< Compat::hex(Compat::array(data.HandleList, data.NumAllocations))
		<< Compat::hex(data.Flags.Value);
}

std::ostream& operator<<(std::ostream& os, const D3DDDICB_LOCK& data)
{
	return Compat::LogStruct(os)
		<< Compat::hex(data.hAllocation)
		<< data.PrivateDriverData
		<< data.NumPages
		<< Compat::array(data.pPages, data.NumPages)
		<< data.pData
		<< Compat::hex(data.Flags.Value)
		<< Compat::hex(data.GpuVirtualAddress);
}

std::ostream& operator<<(std::ostream& os, const D3DDDICB_LOCK2& data)
{
	return Compat::LogStruct(os)
		<< Compat::hex(data.hAllocation)
		<< Compat::hex(data.Flags.Value)
		<< data.pData;
}

std::ostream& operator<<(std::ostream& os, const D3DDDICB_PRESENT& data)
{
	return Compat::LogStruct(os)
		<< Compat::hex(data.hSrcAllocation)
		<< Compat::hex(data.hDstAllocation)
		<< data.hContext
		<< data.BroadcastContextCount
		<< Compat::array(data.BroadcastContext, data.BroadcastContextCount)
		<< Compat::hex(data.BroadcastSrcAllocation)
		<< Compat::hex(data.BroadcastDstAllocation)
		<< data.PrivateDriverDataSize
		<< data.pPrivateDriverData
		<< static_cast<UINT>(data.bOptimizeForComposition);
}

std::ostream& operator<<(std::ostream& os, const D3DDDICB_UNLOCK& data)
{
	return Compat::LogStruct(os)
		<< data.NumAllocations
		<< Compat::hex(Compat::array(data.phAllocations, data.NumAllocations));
}

std::ostream& operator<<(std::ostream& os, const D3DDDICB_UNLOCK2& data)
{
	return Compat::LogStruct(os)
		<< Compat::hex(data.hAllocation);
}
