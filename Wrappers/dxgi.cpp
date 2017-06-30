/**
* Copyright (C) 2017 Elisha Riedlinger
*
* This software is  provided 'as-is', without any express  or implied  warranty. In no event will the
* authors be held liable for any damages arising from the use of this software.
* Permission  is granted  to anyone  to use  this software  for  any  purpose,  including  commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*   1. The origin of this software must not be misrepresented; you must not claim that you  wrote the
*      original  software. If you use this  software  in a product, an  acknowledgment in the product
*      documentation would be appreciated but is not required.
*   2. Altered source versions must  be plainly  marked as such, and  must not be  misrepresented  as
*      being the original software.
*   3. This notice may not be removed or altered from any source distribution.
*/

#include "Settings\Settings.h"
#include "wrapper.h"
#include "Utils\Utils.h"

struct dxgi_dll
{
	HMODULE dll = nullptr;
	FARPROC ApplyCompatResolutionQuirking = jmpaddr;
	FARPROC CompatString = jmpaddr;
	FARPROC CompatValue = jmpaddr;
	FARPROC DXGIDumpJournal = jmpaddr;
	FARPROC DXGIRevertToSxS = jmpaddr;
	FARPROC PIXBeginCapture = jmpaddr;
	FARPROC PIXEndCapture = jmpaddr;
	FARPROC PIXGetCaptureState = jmpaddr;
	FARPROC SetAppCompatStringPointer = jmpaddr;
	FARPROC CreateDXGIFactory1 = jmpaddr;
	FARPROC CreateDXGIFactory2 = jmpaddr;
	FARPROC CreateDXGIFactory = jmpaddr;
	FARPROC DXGID3D10CreateDevice = jmpaddr;
	FARPROC DXGID3D10CreateLayeredDevice = jmpaddr;
	FARPROC DXGID3D10ETWRundown = jmpaddr;
	FARPROC DXGID3D10GetLayeredDeviceSize = jmpaddr;
	FARPROC DXGID3D10RegisterLayers = jmpaddr;
	FARPROC DXGIGetDebugInterface1 = jmpaddr;
	FARPROC DXGIReportAdapterConfiguration = jmpaddr;
	FARPROC D3DKMTCloseAdapter = jmpaddr;
	FARPROC D3DKMTDestroyAllocation = jmpaddr;
	FARPROC D3DKMTDestroyContext = jmpaddr;
	FARPROC D3DKMTDestroyDevice = jmpaddr;
	FARPROC D3DKMTDestroySynchronizationObject = jmpaddr;
	FARPROC D3DKMTQueryAdapterInfo = jmpaddr;
	FARPROC D3DKMTSetDisplayPrivateDriverFormat = jmpaddr;
	FARPROC D3DKMTSignalSynchronizationObject = jmpaddr;
	FARPROC D3DKMTUnlock = jmpaddr;
	FARPROC OpenAdapter10 = jmpaddr;
	FARPROC OpenAdapter10_2 = jmpaddr;
	FARPROC D3DKMTCreateAllocation = jmpaddr;
	FARPROC D3DKMTCreateContext = jmpaddr;
	FARPROC D3DKMTCreateDevice = jmpaddr;
	FARPROC D3DKMTCreateSynchronizationObject = jmpaddr;
	FARPROC D3DKMTEscape = jmpaddr;
	FARPROC D3DKMTGetContextSchedulingPriority = jmpaddr;
	FARPROC D3DKMTGetDeviceState = jmpaddr;
	FARPROC D3DKMTGetDisplayModeList = jmpaddr;
	FARPROC D3DKMTGetMultisampleMethodList = jmpaddr;
	FARPROC D3DKMTGetRuntimeData = jmpaddr;
	FARPROC D3DKMTGetSharedPrimaryHandle = jmpaddr;
	FARPROC D3DKMTLock = jmpaddr;
	FARPROC D3DKMTOpenAdapterFromHdc = jmpaddr;
	FARPROC D3DKMTOpenResource = jmpaddr;
	FARPROC D3DKMTPresent = jmpaddr;
	FARPROC D3DKMTQueryAllocationResidency = jmpaddr;
	FARPROC D3DKMTQueryResourceInfo = jmpaddr;
	FARPROC D3DKMTRender = jmpaddr;
	FARPROC D3DKMTSetAllocationPriority = jmpaddr;
	FARPROC D3DKMTSetContextSchedulingPriority = jmpaddr;
	FARPROC D3DKMTSetDisplayMode = jmpaddr;
	FARPROC D3DKMTSetGammaRamp = jmpaddr;
	FARPROC D3DKMTSetVidPnSourceOwner = jmpaddr;
	FARPROC D3DKMTWaitForSynchronizationObject = jmpaddr;
	FARPROC D3DKMTWaitForVerticalBlankEvent = jmpaddr;
} dxgi;

__declspec(naked) void FakeApplyCompatResolutionQuirking() { _asm { jmp[dxgi.ApplyCompatResolutionQuirking] } }
__declspec(naked) void FakeCompatString() { _asm { jmp[dxgi.CompatString] } }
__declspec(naked) void FakeCompatValue() { _asm { jmp[dxgi.CompatValue] } }
__declspec(naked) void FakeDXGIDumpJournal() { _asm { jmp[dxgi.DXGIDumpJournal] } }
__declspec(naked) void FakeDXGIRevertToSxS() { _asm { jmp[dxgi.DXGIRevertToSxS] } }
__declspec(naked) void FakePIXBeginCapture() { _asm { jmp[dxgi.PIXBeginCapture] } }
__declspec(naked) void FakePIXEndCapture() { _asm { jmp[dxgi.PIXEndCapture] } }
__declspec(naked) void FakePIXGetCaptureState() { _asm { jmp[dxgi.PIXGetCaptureState] } }
__declspec(naked) void FakeSetAppCompatStringPointer() { _asm { jmp[dxgi.SetAppCompatStringPointer] } }
__declspec(naked) void FakeCreateDXGIFactory1() { _asm { jmp[dxgi.CreateDXGIFactory1] } }
__declspec(naked) void FakeCreateDXGIFactory2() { _asm { jmp[dxgi.CreateDXGIFactory2] } }
__declspec(naked) void FakeCreateDXGIFactory() { _asm { jmp[dxgi.CreateDXGIFactory] } }
__declspec(naked) void FakeDXGID3D10CreateDevice() { _asm { jmp[dxgi.DXGID3D10CreateDevice] } }
__declspec(naked) void FakeDXGID3D10CreateLayeredDevice() { _asm { jmp[dxgi.DXGID3D10CreateLayeredDevice] } }
__declspec(naked) void FakeDXGID3D10ETWRundown() { _asm { jmp[dxgi.DXGID3D10ETWRundown] } }
__declspec(naked) void FakeDXGID3D10GetLayeredDeviceSize() { _asm { jmp[dxgi.DXGID3D10GetLayeredDeviceSize] } }
__declspec(naked) void FakeDXGID3D10RegisterLayers() { _asm { jmp[dxgi.DXGID3D10RegisterLayers] } }
__declspec(naked) void FakeDXGIGetDebugInterface1() { _asm { jmp[dxgi.DXGIGetDebugInterface1] } }
__declspec(naked) void FakeDXGIReportAdapterConfiguration() { _asm { jmp[dxgi.DXGIReportAdapterConfiguration] } }
__declspec(naked) void FakeD3DKMTCloseAdapter() { _asm { jmp[dxgi.D3DKMTCloseAdapter] } }
__declspec(naked) void FakeD3DKMTDestroyAllocation() { _asm { jmp[dxgi.D3DKMTDestroyAllocation] } }
__declspec(naked) void FakeD3DKMTDestroyContext() { _asm { jmp[dxgi.D3DKMTDestroyContext] } }
__declspec(naked) void FakeD3DKMTDestroyDevice() { _asm { jmp[dxgi.D3DKMTDestroyDevice] } }
__declspec(naked) void FakeD3DKMTDestroySynchronizationObject() { _asm { jmp[dxgi.D3DKMTDestroySynchronizationObject] } }
__declspec(naked) void FakeD3DKMTQueryAdapterInfo() { _asm { jmp[dxgi.D3DKMTQueryAdapterInfo] } }
__declspec(naked) void FakeD3DKMTSetDisplayPrivateDriverFormat() { _asm { jmp[dxgi.D3DKMTSetDisplayPrivateDriverFormat] } }
__declspec(naked) void FakeD3DKMTSignalSynchronizationObject() { _asm { jmp[dxgi.D3DKMTSignalSynchronizationObject] } }
__declspec(naked) void FakeD3DKMTUnlock() { _asm { jmp[dxgi.D3DKMTUnlock] } }
__declspec(naked) void FakeOpenAdapter10() { _asm { jmp[dxgi.OpenAdapter10] } }
__declspec(naked) void FakeOpenAdapter10_2() { _asm { jmp[dxgi.OpenAdapter10_2] } }
__declspec(naked) void FakeD3DKMTCreateAllocation() { _asm { jmp[dxgi.D3DKMTCreateAllocation] } }
__declspec(naked) void FakeD3DKMTCreateContext() { _asm { jmp[dxgi.D3DKMTCreateContext] } }
__declspec(naked) void FakeD3DKMTCreateDevice() { _asm { jmp[dxgi.D3DKMTCreateDevice] } }
__declspec(naked) void FakeD3DKMTCreateSynchronizationObject() { _asm { jmp[dxgi.D3DKMTCreateSynchronizationObject] } }
__declspec(naked) void FakeD3DKMTEscape() { _asm { jmp[dxgi.D3DKMTEscape] } }
__declspec(naked) void FakeD3DKMTGetContextSchedulingPriority() { _asm { jmp[dxgi.D3DKMTGetContextSchedulingPriority] } }
__declspec(naked) void FakeD3DKMTGetDeviceState() { _asm { jmp[dxgi.D3DKMTGetDeviceState] } }
__declspec(naked) void FakeD3DKMTGetDisplayModeList() { _asm { jmp[dxgi.D3DKMTGetDisplayModeList] } }
__declspec(naked) void FakeD3DKMTGetMultisampleMethodList() { _asm { jmp[dxgi.D3DKMTGetMultisampleMethodList] } }
__declspec(naked) void FakeD3DKMTGetRuntimeData() { _asm { jmp[dxgi.D3DKMTGetRuntimeData] } }
__declspec(naked) void FakeD3DKMTGetSharedPrimaryHandle() { _asm { jmp[dxgi.D3DKMTGetSharedPrimaryHandle] } }
__declspec(naked) void FakeD3DKMTLock() { _asm { jmp[dxgi.D3DKMTLock] } }
__declspec(naked) void FakeD3DKMTOpenAdapterFromHdc() { _asm { jmp[dxgi.D3DKMTOpenAdapterFromHdc] } }
__declspec(naked) void FakeD3DKMTOpenResource() { _asm { jmp[dxgi.D3DKMTOpenResource] } }
__declspec(naked) void FakeD3DKMTPresent() { _asm { jmp[dxgi.D3DKMTPresent] } }
__declspec(naked) void FakeD3DKMTQueryAllocationResidency() { _asm { jmp[dxgi.D3DKMTQueryAllocationResidency] } }
__declspec(naked) void FakeD3DKMTQueryResourceInfo() { _asm { jmp[dxgi.D3DKMTQueryResourceInfo] } }
__declspec(naked) void FakeD3DKMTRender() { _asm { jmp[dxgi.D3DKMTRender] } }
__declspec(naked) void FakeD3DKMTSetAllocationPriority() { _asm { jmp[dxgi.D3DKMTSetAllocationPriority] } }
__declspec(naked) void FakeD3DKMTSetContextSchedulingPriority() { _asm { jmp[dxgi.D3DKMTSetContextSchedulingPriority] } }
__declspec(naked) void FakeD3DKMTSetDisplayMode() { _asm { jmp[dxgi.D3DKMTSetDisplayMode] } }
__declspec(naked) void FakeD3DKMTSetGammaRamp() { _asm { jmp[dxgi.D3DKMTSetGammaRamp] } }
__declspec(naked) void FakeD3DKMTSetVidPnSourceOwner() { _asm { jmp[dxgi.D3DKMTSetVidPnSourceOwner] } }
__declspec(naked) void FakeD3DKMTWaitForSynchronizationObject() { _asm { jmp[dxgi.D3DKMTWaitForSynchronizationObject] } }
__declspec(naked) void FakeD3DKMTWaitForVerticalBlankEvent() { _asm { jmp[dxgi.D3DKMTWaitForVerticalBlankEvent] } }

void LoadDxgi()
{
	// Load real dll
	dxgi.dll = LoadDll(dtype.dxgi);

	// Load dll functions
	if (dxgi.dll)
	{
		dxgi.ApplyCompatResolutionQuirking = GetFunctionAddress(dxgi.dll, "ApplyCompatResolutionQuirking", jmpaddr);
		dxgi.CompatString = GetFunctionAddress(dxgi.dll, "CompatString", jmpaddr);
		dxgi.CompatValue = GetFunctionAddress(dxgi.dll, "CompatValue", jmpaddr);
		dxgi.DXGIDumpJournal = GetFunctionAddress(dxgi.dll, "DXGIDumpJournal", jmpaddr);
		dxgi.DXGIRevertToSxS = GetFunctionAddress(dxgi.dll, "DXGIRevertToSxS", jmpaddr);
		dxgi.PIXBeginCapture = GetFunctionAddress(dxgi.dll, "PIXBeginCapture", jmpaddr);
		dxgi.PIXEndCapture = GetFunctionAddress(dxgi.dll, "PIXEndCapture", jmpaddr);
		dxgi.PIXGetCaptureState = GetFunctionAddress(dxgi.dll, "PIXGetCaptureState", jmpaddr);
		dxgi.SetAppCompatStringPointer = GetFunctionAddress(dxgi.dll, "SetAppCompatStringPointer", jmpaddr);
		dxgi.CreateDXGIFactory1 = GetFunctionAddress(dxgi.dll, "CreateDXGIFactory1", jmpaddr);
		dxgi.CreateDXGIFactory2 = GetFunctionAddress(dxgi.dll, "CreateDXGIFactory2", jmpaddr);
		dxgi.CreateDXGIFactory = GetFunctionAddress(dxgi.dll, "CreateDXGIFactory", jmpaddr);
		dxgi.DXGID3D10CreateDevice = GetFunctionAddress(dxgi.dll, "DXGID3D10CreateDevice", jmpaddr);
		dxgi.DXGID3D10CreateLayeredDevice = GetFunctionAddress(dxgi.dll, "DXGID3D10CreateLayeredDevice", jmpaddr);
		dxgi.DXGID3D10ETWRundown = GetFunctionAddress(dxgi.dll, "DXGID3D10ETWRundown", jmpaddr);
		dxgi.DXGID3D10GetLayeredDeviceSize = GetFunctionAddress(dxgi.dll, "DXGID3D10GetLayeredDeviceSize", jmpaddr);
		dxgi.DXGID3D10RegisterLayers = GetFunctionAddress(dxgi.dll, "DXGID3D10RegisterLayers", jmpaddr);
		dxgi.DXGIGetDebugInterface1 = GetFunctionAddress(dxgi.dll, "DXGIGetDebugInterface1", jmpaddr);
		dxgi.DXGIReportAdapterConfiguration = GetFunctionAddress(dxgi.dll, "DXGIReportAdapterConfiguration", jmpaddr);
		dxgi.D3DKMTCloseAdapter = GetFunctionAddress(dxgi.dll, "D3DKMTCloseAdapter", jmpaddr);
		dxgi.D3DKMTDestroyAllocation = GetFunctionAddress(dxgi.dll, "D3DKMTDestroyAllocation", jmpaddr);
		dxgi.D3DKMTDestroyContext = GetFunctionAddress(dxgi.dll, "D3DKMTDestroyContext", jmpaddr);
		dxgi.D3DKMTDestroyDevice = GetFunctionAddress(dxgi.dll, "D3DKMTDestroyDevice", jmpaddr);
		dxgi.D3DKMTDestroySynchronizationObject = GetFunctionAddress(dxgi.dll, "D3DKMTDestroySynchronizationObject", jmpaddr);
		dxgi.D3DKMTQueryAdapterInfo = GetFunctionAddress(dxgi.dll, "D3DKMTQueryAdapterInfo", jmpaddr);
		dxgi.D3DKMTSetDisplayPrivateDriverFormat = GetFunctionAddress(dxgi.dll, "D3DKMTSetDisplayPrivateDriverFormat", jmpaddr);
		dxgi.D3DKMTSignalSynchronizationObject = GetFunctionAddress(dxgi.dll, "D3DKMTSignalSynchronizationObject", jmpaddr);
		dxgi.D3DKMTUnlock = GetFunctionAddress(dxgi.dll, "D3DKMTUnlock", jmpaddr);
		dxgi.OpenAdapter10 = GetFunctionAddress(dxgi.dll, "OpenAdapter10", jmpaddr);
		dxgi.OpenAdapter10_2 = GetFunctionAddress(dxgi.dll, "OpenAdapter10_2", jmpaddr);
		dxgi.D3DKMTCreateAllocation = GetFunctionAddress(dxgi.dll, "D3DKMTCreateAllocation", jmpaddr);
		dxgi.D3DKMTCreateContext = GetFunctionAddress(dxgi.dll, "D3DKMTCreateContext", jmpaddr);
		dxgi.D3DKMTCreateDevice = GetFunctionAddress(dxgi.dll, "D3DKMTCreateDevice", jmpaddr);
		dxgi.D3DKMTCreateSynchronizationObject = GetFunctionAddress(dxgi.dll, "D3DKMTCreateSynchronizationObject", jmpaddr);
		dxgi.D3DKMTEscape = GetFunctionAddress(dxgi.dll, "D3DKMTEscape", jmpaddr);
		dxgi.D3DKMTGetContextSchedulingPriority = GetFunctionAddress(dxgi.dll, "D3DKMTGetContextSchedulingPriority", jmpaddr);
		dxgi.D3DKMTGetDeviceState = GetFunctionAddress(dxgi.dll, "D3DKMTGetDeviceState", jmpaddr);
		dxgi.D3DKMTGetDisplayModeList = GetFunctionAddress(dxgi.dll, "D3DKMTGetDisplayModeList", jmpaddr);
		dxgi.D3DKMTGetMultisampleMethodList = GetFunctionAddress(dxgi.dll, "D3DKMTGetMultisampleMethodList", jmpaddr);
		dxgi.D3DKMTGetRuntimeData = GetFunctionAddress(dxgi.dll, "D3DKMTGetRuntimeData", jmpaddr);
		dxgi.D3DKMTGetSharedPrimaryHandle = GetFunctionAddress(dxgi.dll, "D3DKMTGetSharedPrimaryHandle", jmpaddr);
		dxgi.D3DKMTLock = GetFunctionAddress(dxgi.dll, "D3DKMTLock", jmpaddr);
		dxgi.D3DKMTOpenAdapterFromHdc = GetFunctionAddress(dxgi.dll, "D3DKMTOpenAdapterFromHdc", jmpaddr);
		dxgi.D3DKMTOpenResource = GetFunctionAddress(dxgi.dll, "D3DKMTOpenResource", jmpaddr);
		dxgi.D3DKMTPresent = GetFunctionAddress(dxgi.dll, "D3DKMTPresent", jmpaddr);
		dxgi.D3DKMTQueryAllocationResidency = GetFunctionAddress(dxgi.dll, "D3DKMTQueryAllocationResidency", jmpaddr);
		dxgi.D3DKMTQueryResourceInfo = GetFunctionAddress(dxgi.dll, "D3DKMTQueryResourceInfo", jmpaddr);
		dxgi.D3DKMTRender = GetFunctionAddress(dxgi.dll, "D3DKMTRender", jmpaddr);
		dxgi.D3DKMTSetAllocationPriority = GetFunctionAddress(dxgi.dll, "D3DKMTSetAllocationPriority", jmpaddr);
		dxgi.D3DKMTSetContextSchedulingPriority = GetFunctionAddress(dxgi.dll, "D3DKMTSetContextSchedulingPriority", jmpaddr);
		dxgi.D3DKMTSetDisplayMode = GetFunctionAddress(dxgi.dll, "D3DKMTSetDisplayMode", jmpaddr);
		dxgi.D3DKMTSetGammaRamp = GetFunctionAddress(dxgi.dll, "D3DKMTSetGammaRamp", jmpaddr);
		dxgi.D3DKMTSetVidPnSourceOwner = GetFunctionAddress(dxgi.dll, "D3DKMTSetVidPnSourceOwner", jmpaddr);
		dxgi.D3DKMTWaitForSynchronizationObject = GetFunctionAddress(dxgi.dll, "D3DKMTWaitForSynchronizationObject", jmpaddr);
		dxgi.D3DKMTWaitForVerticalBlankEvent = GetFunctionAddress(dxgi.dll, "D3DKMTWaitForVerticalBlankEvent", jmpaddr);
	}
}