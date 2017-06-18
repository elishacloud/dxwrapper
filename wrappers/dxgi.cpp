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

struct dxgi_dll
{
	HMODULE dll = nullptr;
	FARPROC ApplyCompatResolutionQuirking;
	FARPROC CompatString;
	FARPROC CompatValue;
	FARPROC DXGIDumpJournal;
	FARPROC DXGIRevertToSxS;
	FARPROC PIXBeginCapture;
	FARPROC PIXEndCapture;
	FARPROC PIXGetCaptureState;
	FARPROC SetAppCompatStringPointer;
	FARPROC CreateDXGIFactory1;
	FARPROC CreateDXGIFactory2;
	FARPROC CreateDXGIFactory;
	FARPROC DXGID3D10CreateDevice;
	FARPROC DXGID3D10CreateLayeredDevice;
	FARPROC DXGID3D10ETWRundown;
	FARPROC DXGID3D10GetLayeredDeviceSize;
	FARPROC DXGID3D10RegisterLayers;
	FARPROC DXGIGetDebugInterface1;
	FARPROC DXGIReportAdapterConfiguration;
	FARPROC D3DKMTCloseAdapter;
	FARPROC D3DKMTDestroyAllocation;
	FARPROC D3DKMTDestroyContext;
	FARPROC D3DKMTDestroyDevice;
	FARPROC D3DKMTDestroySynchronizationObject;
	FARPROC D3DKMTQueryAdapterInfo;
	FARPROC D3DKMTSetDisplayPrivateDriverFormat;
	FARPROC D3DKMTSignalSynchronizationObject;
	FARPROC D3DKMTUnlock;
	FARPROC OpenAdapter10;
	FARPROC OpenAdapter10_2;
	FARPROC D3DKMTCreateAllocation;
	FARPROC D3DKMTCreateContext;
	FARPROC D3DKMTCreateDevice;
	FARPROC D3DKMTCreateSynchronizationObject;
	FARPROC D3DKMTEscape;
	FARPROC D3DKMTGetContextSchedulingPriority;
	FARPROC D3DKMTGetDeviceState;
	FARPROC D3DKMTGetDisplayModeList;
	FARPROC D3DKMTGetMultisampleMethodList;
	FARPROC D3DKMTGetRuntimeData;
	FARPROC D3DKMTGetSharedPrimaryHandle;
	FARPROC D3DKMTLock;
	FARPROC D3DKMTOpenAdapterFromHdc;
	FARPROC D3DKMTOpenResource;
	FARPROC D3DKMTPresent;
	FARPROC D3DKMTQueryAllocationResidency;
	FARPROC D3DKMTQueryResourceInfo;
	FARPROC D3DKMTRender;
	FARPROC D3DKMTSetAllocationPriority;
	FARPROC D3DKMTSetContextSchedulingPriority;
	FARPROC D3DKMTSetDisplayMode;
	FARPROC D3DKMTSetGammaRamp;
	FARPROC D3DKMTSetVidPnSourceOwner;
	FARPROC D3DKMTWaitForSynchronizationObject;
	FARPROC D3DKMTWaitForVerticalBlankEvent;
} dxgi;

__declspec(naked) void FakeApplyCompatResolutionQuirking() { _asm { jmp [dxgi.ApplyCompatResolutionQuirking] } }
__declspec(naked) void FakeCompatString() { _asm { jmp [dxgi.CompatString] } }
__declspec(naked) void FakeCompatValue() { _asm { jmp [dxgi.CompatValue] } }
__declspec(naked) void FakeDXGIDumpJournal() { _asm { jmp [dxgi.DXGIDumpJournal] } }
__declspec(naked) void FakeDXGIRevertToSxS() { _asm { jmp [dxgi.DXGIRevertToSxS] } }
__declspec(naked) void FakePIXBeginCapture() { _asm { jmp [dxgi.PIXBeginCapture] } }
__declspec(naked) void FakePIXEndCapture() { _asm { jmp [dxgi.PIXEndCapture] } }
__declspec(naked) void FakePIXGetCaptureState() { _asm { jmp [dxgi.PIXGetCaptureState] } }
__declspec(naked) void FakeSetAppCompatStringPointer() { _asm { jmp [dxgi.SetAppCompatStringPointer] } }
__declspec(naked) void FakeCreateDXGIFactory1() { _asm { jmp [dxgi.CreateDXGIFactory1] } }
__declspec(naked) void FakeCreateDXGIFactory2() { _asm { jmp [dxgi.CreateDXGIFactory2] } }
__declspec(naked) void FakeCreateDXGIFactory() { _asm { jmp [dxgi.CreateDXGIFactory] } }
__declspec(naked) void FakeDXGID3D10CreateDevice() { _asm { jmp [dxgi.DXGID3D10CreateDevice] } }
__declspec(naked) void FakeDXGID3D10CreateLayeredDevice() { _asm { jmp [dxgi.DXGID3D10CreateLayeredDevice] } }
__declspec(naked) void FakeDXGID3D10ETWRundown() { _asm { jmp [dxgi.DXGID3D10ETWRundown] } }
__declspec(naked) void FakeDXGID3D10GetLayeredDeviceSize() { _asm { jmp [dxgi.DXGID3D10GetLayeredDeviceSize] } }
__declspec(naked) void FakeDXGID3D10RegisterLayers() { _asm { jmp [dxgi.DXGID3D10RegisterLayers] } }
__declspec(naked) void FakeDXGIGetDebugInterface1() { _asm { jmp [dxgi.DXGIGetDebugInterface1] } }
__declspec(naked) void FakeDXGIReportAdapterConfiguration() { _asm { jmp [dxgi.DXGIReportAdapterConfiguration] } }
__declspec(naked) void FakeD3DKMTCloseAdapter() { _asm { jmp [dxgi.D3DKMTCloseAdapter] } }
__declspec(naked) void FakeD3DKMTDestroyAllocation() { _asm { jmp [dxgi.D3DKMTDestroyAllocation] } }
__declspec(naked) void FakeD3DKMTDestroyContext() { _asm { jmp [dxgi.D3DKMTDestroyContext] } }
__declspec(naked) void FakeD3DKMTDestroyDevice() { _asm { jmp [dxgi.D3DKMTDestroyDevice] } }
__declspec(naked) void FakeD3DKMTDestroySynchronizationObject() { _asm { jmp [dxgi.D3DKMTDestroySynchronizationObject] } }
__declspec(naked) void FakeD3DKMTQueryAdapterInfo() { _asm { jmp [dxgi.D3DKMTQueryAdapterInfo] } }
__declspec(naked) void FakeD3DKMTSetDisplayPrivateDriverFormat() { _asm { jmp [dxgi.D3DKMTSetDisplayPrivateDriverFormat] } }
__declspec(naked) void FakeD3DKMTSignalSynchronizationObject() { _asm { jmp [dxgi.D3DKMTSignalSynchronizationObject] } }
__declspec(naked) void FakeD3DKMTUnlock() { _asm { jmp [dxgi.D3DKMTUnlock] } }
__declspec(naked) void FakeOpenAdapter10() { _asm { jmp [dxgi.OpenAdapter10] } }
__declspec(naked) void FakeOpenAdapter10_2() { _asm { jmp [dxgi.OpenAdapter10_2] } }
__declspec(naked) void FakeD3DKMTCreateAllocation() { _asm { jmp [dxgi.D3DKMTCreateAllocation] } }
__declspec(naked) void FakeD3DKMTCreateContext() { _asm { jmp [dxgi.D3DKMTCreateContext] } }
__declspec(naked) void FakeD3DKMTCreateDevice() { _asm { jmp [dxgi.D3DKMTCreateDevice] } }
__declspec(naked) void FakeD3DKMTCreateSynchronizationObject() { _asm { jmp [dxgi.D3DKMTCreateSynchronizationObject] } }
__declspec(naked) void FakeD3DKMTEscape() { _asm { jmp [dxgi.D3DKMTEscape] } }
__declspec(naked) void FakeD3DKMTGetContextSchedulingPriority() { _asm { jmp [dxgi.D3DKMTGetContextSchedulingPriority] } }
__declspec(naked) void FakeD3DKMTGetDeviceState() { _asm { jmp [dxgi.D3DKMTGetDeviceState] } }
__declspec(naked) void FakeD3DKMTGetDisplayModeList() { _asm { jmp [dxgi.D3DKMTGetDisplayModeList] } }
__declspec(naked) void FakeD3DKMTGetMultisampleMethodList() { _asm { jmp [dxgi.D3DKMTGetMultisampleMethodList] } }
__declspec(naked) void FakeD3DKMTGetRuntimeData() { _asm { jmp [dxgi.D3DKMTGetRuntimeData] } }
__declspec(naked) void FakeD3DKMTGetSharedPrimaryHandle() { _asm { jmp [dxgi.D3DKMTGetSharedPrimaryHandle] } }
__declspec(naked) void FakeD3DKMTLock() { _asm { jmp [dxgi.D3DKMTLock] } }
__declspec(naked) void FakeD3DKMTOpenAdapterFromHdc() { _asm { jmp [dxgi.D3DKMTOpenAdapterFromHdc] } }
__declspec(naked) void FakeD3DKMTOpenResource() { _asm { jmp [dxgi.D3DKMTOpenResource] } }
__declspec(naked) void FakeD3DKMTPresent() { _asm { jmp [dxgi.D3DKMTPresent] } }
__declspec(naked) void FakeD3DKMTQueryAllocationResidency() { _asm { jmp [dxgi.D3DKMTQueryAllocationResidency] } }
__declspec(naked) void FakeD3DKMTQueryResourceInfo() { _asm { jmp [dxgi.D3DKMTQueryResourceInfo] } }
__declspec(naked) void FakeD3DKMTRender() { _asm { jmp [dxgi.D3DKMTRender] } }
__declspec(naked) void FakeD3DKMTSetAllocationPriority() { _asm { jmp [dxgi.D3DKMTSetAllocationPriority] } }
__declspec(naked) void FakeD3DKMTSetContextSchedulingPriority() { _asm { jmp [dxgi.D3DKMTSetContextSchedulingPriority] } }
__declspec(naked) void FakeD3DKMTSetDisplayMode() { _asm { jmp [dxgi.D3DKMTSetDisplayMode] } }
__declspec(naked) void FakeD3DKMTSetGammaRamp() { _asm { jmp [dxgi.D3DKMTSetGammaRamp] } }
__declspec(naked) void FakeD3DKMTSetVidPnSourceOwner() { _asm { jmp [dxgi.D3DKMTSetVidPnSourceOwner] } }
__declspec(naked) void FakeD3DKMTWaitForSynchronizationObject() { _asm { jmp [dxgi.D3DKMTWaitForSynchronizationObject] } }
__declspec(naked) void FakeD3DKMTWaitForVerticalBlankEvent() { _asm { jmp [dxgi.D3DKMTWaitForVerticalBlankEvent] } }

void LoadDxgi()
{
	// Load real dll
	dxgi.dll = LoadDll(dtype.dxgi);
	// Load dll functions
	if (dxgi.dll)
	{
		dxgi.ApplyCompatResolutionQuirking = GetProcAddress(dxgi.dll, "ApplyCompatResolutionQuirking");
		dxgi.CompatString = GetProcAddress(dxgi.dll, "CompatString");
		dxgi.CompatValue = GetProcAddress(dxgi.dll, "CompatValue");
		dxgi.DXGIDumpJournal = GetProcAddress(dxgi.dll, "DXGIDumpJournal");
		dxgi.DXGIRevertToSxS = GetProcAddress(dxgi.dll, "DXGIRevertToSxS");
		dxgi.PIXBeginCapture = GetProcAddress(dxgi.dll, "PIXBeginCapture");
		dxgi.PIXEndCapture = GetProcAddress(dxgi.dll, "PIXEndCapture");
		dxgi.PIXGetCaptureState = GetProcAddress(dxgi.dll, "PIXGetCaptureState");
		dxgi.SetAppCompatStringPointer = GetProcAddress(dxgi.dll, "SetAppCompatStringPointer");
		dxgi.CreateDXGIFactory1 = GetProcAddress(dxgi.dll, "CreateDXGIFactory1");
		dxgi.CreateDXGIFactory2 = GetProcAddress(dxgi.dll, "CreateDXGIFactory2");
		dxgi.CreateDXGIFactory = GetProcAddress(dxgi.dll, "CreateDXGIFactory");
		dxgi.DXGID3D10CreateDevice = GetProcAddress(dxgi.dll, "DXGID3D10CreateDevice");
		dxgi.DXGID3D10CreateLayeredDevice = GetProcAddress(dxgi.dll, "DXGID3D10CreateLayeredDevice");
		dxgi.DXGID3D10ETWRundown = GetProcAddress(dxgi.dll, "DXGID3D10ETWRundown");
		dxgi.DXGID3D10GetLayeredDeviceSize = GetProcAddress(dxgi.dll, "DXGID3D10GetLayeredDeviceSize");
		dxgi.DXGID3D10RegisterLayers = GetProcAddress(dxgi.dll, "DXGID3D10RegisterLayers");
		dxgi.DXGIGetDebugInterface1 = GetProcAddress(dxgi.dll, "DXGIGetDebugInterface1");
		dxgi.DXGIReportAdapterConfiguration = GetProcAddress(dxgi.dll, "DXGIReportAdapterConfiguration");
		dxgi.D3DKMTCloseAdapter = GetProcAddress(dxgi.dll, "D3DKMTCloseAdapter");
		dxgi.D3DKMTDestroyAllocation = GetProcAddress(dxgi.dll, "D3DKMTDestroyAllocation");
		dxgi.D3DKMTDestroyContext = GetProcAddress(dxgi.dll, "D3DKMTDestroyContext");
		dxgi.D3DKMTDestroyDevice = GetProcAddress(dxgi.dll, "D3DKMTDestroyDevice");
		dxgi.D3DKMTDestroySynchronizationObject = GetProcAddress(dxgi.dll, "D3DKMTDestroySynchronizationObject");
		dxgi.D3DKMTQueryAdapterInfo = GetProcAddress(dxgi.dll, "D3DKMTQueryAdapterInfo");
		dxgi.D3DKMTSetDisplayPrivateDriverFormat = GetProcAddress(dxgi.dll, "D3DKMTSetDisplayPrivateDriverFormat");
		dxgi.D3DKMTSignalSynchronizationObject = GetProcAddress(dxgi.dll, "D3DKMTSignalSynchronizationObject");
		dxgi.D3DKMTUnlock = GetProcAddress(dxgi.dll, "D3DKMTUnlock");
		dxgi.OpenAdapter10 = GetProcAddress(dxgi.dll, "OpenAdapter10");
		dxgi.OpenAdapter10_2 = GetProcAddress(dxgi.dll, "OpenAdapter10_2");
		dxgi.D3DKMTCreateAllocation = GetProcAddress(dxgi.dll, "D3DKMTCreateAllocation");
		dxgi.D3DKMTCreateContext = GetProcAddress(dxgi.dll, "D3DKMTCreateContext");
		dxgi.D3DKMTCreateDevice = GetProcAddress(dxgi.dll, "D3DKMTCreateDevice");
		dxgi.D3DKMTCreateSynchronizationObject = GetProcAddress(dxgi.dll, "D3DKMTCreateSynchronizationObject");
		dxgi.D3DKMTEscape = GetProcAddress(dxgi.dll, "D3DKMTEscape");
		dxgi.D3DKMTGetContextSchedulingPriority = GetProcAddress(dxgi.dll, "D3DKMTGetContextSchedulingPriority");
		dxgi.D3DKMTGetDeviceState = GetProcAddress(dxgi.dll, "D3DKMTGetDeviceState");
		dxgi.D3DKMTGetDisplayModeList = GetProcAddress(dxgi.dll, "D3DKMTGetDisplayModeList");
		dxgi.D3DKMTGetMultisampleMethodList = GetProcAddress(dxgi.dll, "D3DKMTGetMultisampleMethodList");
		dxgi.D3DKMTGetRuntimeData = GetProcAddress(dxgi.dll, "D3DKMTGetRuntimeData");
		dxgi.D3DKMTGetSharedPrimaryHandle = GetProcAddress(dxgi.dll, "D3DKMTGetSharedPrimaryHandle");
		dxgi.D3DKMTLock = GetProcAddress(dxgi.dll, "D3DKMTLock");
		dxgi.D3DKMTOpenAdapterFromHdc = GetProcAddress(dxgi.dll, "D3DKMTOpenAdapterFromHdc");
		dxgi.D3DKMTOpenResource = GetProcAddress(dxgi.dll, "D3DKMTOpenResource");
		dxgi.D3DKMTPresent = GetProcAddress(dxgi.dll, "D3DKMTPresent");
		dxgi.D3DKMTQueryAllocationResidency = GetProcAddress(dxgi.dll, "D3DKMTQueryAllocationResidency");
		dxgi.D3DKMTQueryResourceInfo = GetProcAddress(dxgi.dll, "D3DKMTQueryResourceInfo");
		dxgi.D3DKMTRender = GetProcAddress(dxgi.dll, "D3DKMTRender");
		dxgi.D3DKMTSetAllocationPriority = GetProcAddress(dxgi.dll, "D3DKMTSetAllocationPriority");
		dxgi.D3DKMTSetContextSchedulingPriority = GetProcAddress(dxgi.dll, "D3DKMTSetContextSchedulingPriority");
		dxgi.D3DKMTSetDisplayMode = GetProcAddress(dxgi.dll, "D3DKMTSetDisplayMode");
		dxgi.D3DKMTSetGammaRamp = GetProcAddress(dxgi.dll, "D3DKMTSetGammaRamp");
		dxgi.D3DKMTSetVidPnSourceOwner = GetProcAddress(dxgi.dll, "D3DKMTSetVidPnSourceOwner");
		dxgi.D3DKMTWaitForSynchronizationObject = GetProcAddress(dxgi.dll, "D3DKMTWaitForSynchronizationObject");
		dxgi.D3DKMTWaitForVerticalBlankEvent = GetProcAddress(dxgi.dll, "D3DKMTWaitForVerticalBlankEvent");
	}
}