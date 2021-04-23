#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <DDrawCompat/DDrawLog.h>
#include <DDrawCompat/v0.3.0/D3dDdi/Log/AdapterFuncsLog.h>

std::ostream& operator<<(std::ostream& os, const D3DDDI_ALLOCATIONLIST& data)
{
	return Compat30::LogStruct(os)
		<< Compat30::hex(data.hAllocation)
		<< Compat30::hex(data.Value);
}

std::ostream& operator<<(std::ostream& os, const D3DDDI_PATCHLOCATIONLIST& data)
{
	return Compat30::LogStruct(os)
		<< data.AllocationIndex
		<< Compat30::hex(data.Value)
		<< Compat30::hex(data.DriverId)
		<< Compat30::hex(data.AllocationOffset)
		<< Compat30::hex(data.PatchOffset)
		<< Compat30::hex(data.SplitOffset);
}

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_CREATEDEVICE& data)
{
	return Compat30::LogStruct(os)
		<< data.hDevice
		<< data.Interface
		<< data.Version
		<< data.pCallbacks
		<< data.pCommandBuffer
		<< data.CommandBufferSize
		<< Compat30::out(Compat30::array(data.pAllocationList, data.AllocationListSize))
		<< data.AllocationListSize
		<< Compat30::out(Compat30::array(data.pPatchLocationList, data.PatchLocationListSize))
		<< data.PatchLocationListSize
		<< Compat30::out(data.pDeviceFuncs)
		<< Compat30::hex(data.Flags.Value)
		<< Compat30::hex(data.CommandBuffer);
}
