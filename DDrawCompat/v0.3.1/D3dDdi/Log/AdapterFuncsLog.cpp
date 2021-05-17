#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <DDrawCompat/DDrawLog.h>
#include <DDrawCompat/v0.3.1/D3dDdi/Log/AdapterFuncsLog.h>

std::ostream& operator<<(std::ostream& os, const D3DDDI_ALLOCATIONLIST& data)
{
	return Compat31::LogStruct(os)
		<< Compat31::hex(data.hAllocation)
		<< Compat31::hex(data.Value);
}

std::ostream& operator<<(std::ostream& os, const D3DDDI_PATCHLOCATIONLIST& data)
{
	return Compat31::LogStruct(os)
		<< data.AllocationIndex
		<< Compat31::hex(data.Value)
		<< Compat31::hex(data.DriverId)
		<< Compat31::hex(data.AllocationOffset)
		<< Compat31::hex(data.PatchOffset)
		<< Compat31::hex(data.SplitOffset);
}

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_CREATEDEVICE& data)
{
	return Compat31::LogStruct(os)
		<< data.hDevice
		<< data.Interface
		<< data.Version
		<< data.pCallbacks
		<< data.pCommandBuffer
		<< data.CommandBufferSize
		<< Compat31::out(Compat31::array(data.pAllocationList, data.AllocationListSize))
		<< data.AllocationListSize
		<< Compat31::out(Compat31::array(data.pPatchLocationList, data.PatchLocationListSize))
		<< data.PatchLocationListSize
		<< Compat31::out(data.pDeviceFuncs)
		<< Compat31::hex(data.Flags.Value)
		<< Compat31::hex(data.CommandBuffer);
}
