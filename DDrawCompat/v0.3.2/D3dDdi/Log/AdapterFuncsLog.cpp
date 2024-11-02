#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <DDrawCompat/DDrawLog.h>
#include <DDrawCompat/v0.3.2/D3dDdi/Log/AdapterFuncsLog.h>

std::ostream& operator<<(std::ostream& os, const D3DDDI_ALLOCATIONLIST& data)
{
	return Compat32::LogStruct(os)
		<< Compat32::hex(data.hAllocation)
		<< Compat32::hex(data.Value);
}

std::ostream& operator<<(std::ostream& os, const D3DDDI_PATCHLOCATIONLIST& data)
{
	return Compat32::LogStruct(os)
		<< data.AllocationIndex
		<< Compat32::hex(data.Value)
		<< Compat32::hex(data.DriverId)
		<< Compat32::hex(data.AllocationOffset)
		<< Compat32::hex(data.PatchOffset)
		<< Compat32::hex(data.SplitOffset);
}

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_CREATEDEVICE& data)
{
	return Compat32::LogStruct(os)
		<< data.hDevice
		<< data.Interface
		<< data.Version
		<< data.pCallbacks
		<< data.pCommandBuffer
		<< data.CommandBufferSize
		<< Compat32::out(Compat32::array(data.pAllocationList, data.AllocationListSize))
		<< data.AllocationListSize
		<< Compat32::out(Compat32::array(data.pPatchLocationList, data.PatchLocationListSize))
		<< data.PatchLocationListSize
		<< Compat32::out(data.pDeviceFuncs)
		<< Compat32::hex(data.Flags.Value)
		<< Compat32::hex(data.CommandBuffer);
}
