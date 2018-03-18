#include "DDrawCompat\DDrawLog.h"
#include "D3dDdi/Log/AdapterFuncsLog.h"

std::ostream& operator<<(std::ostream& os, const D3DDDI_ALLOCATIONLIST& data)
{
	return Compat::LogStruct(os)
		<< Compat::hex(data.hAllocation)
		<< Compat::hex(data.Value);
}

std::ostream& operator<<(std::ostream& os, const D3DDDI_PATCHLOCATIONLIST& data)
{
	return Compat::LogStruct(os)
		<< data.AllocationIndex
		<< Compat::hex(data.Value)
		<< Compat::hex(data.DriverId)
		<< Compat::hex(data.AllocationOffset)
		<< Compat::hex(data.PatchOffset)
		<< Compat::hex(data.SplitOffset);
}

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_CREATEDEVICE& data)
{
	return Compat::LogStruct(os)
		<< data.hDevice
		<< data.Interface
		<< data.Version
		<< data.pCallbacks
		<< data.pCommandBuffer
		<< data.CommandBufferSize
		<< Compat::out(Compat::array(data.pAllocationList, data.AllocationListSize))
		<< data.AllocationListSize
		<< Compat::out(Compat::array(data.pPatchLocationList, data.PatchLocationListSize))
		<< data.PatchLocationListSize
		<< Compat::out(data.pDeviceFuncs)
		<< Compat::hex(data.Flags.Value)
		<< Compat::hex(data.CommandBuffer);
}
