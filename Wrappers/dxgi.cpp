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

#define module dxgi

#define VISIT_PROCS(visit) \
	visit(ApplyCompatResolutionQuirking) \
	visit(CompatString) \
	visit(CompatValue) \
	visit(DXGIDumpJournal) \
	visit(DXGIRevertToSxS) \
	visit(PIXBeginCapture) \
	visit(PIXEndCapture) \
	visit(PIXGetCaptureState) \
	visit(SetAppCompatStringPointer) \
	visit(CreateDXGIFactory1) \
	visit(CreateDXGIFactory2) \
	visit(CreateDXGIFactory) \
	visit(DXGID3D10CreateDevice) \
	visit(DXGID3D10CreateLayeredDevice) \
	visit(DXGID3D10ETWRundown) \
	visit(DXGID3D10GetLayeredDeviceSize) \
	visit(DXGID3D10RegisterLayers) \
	visit(DXGIGetDebugInterface1) \
	visit(DXGIReportAdapterConfiguration) \
	visit(D3DKMTCloseAdapter) \
	visit(D3DKMTDestroyAllocation) \
	visit(D3DKMTDestroyContext) \
	visit(D3DKMTDestroyDevice) \
	visit(D3DKMTDestroySynchronizationObject) \
	visit(D3DKMTQueryAdapterInfo) \
	visit(D3DKMTSetDisplayPrivateDriverFormat) \
	visit(D3DKMTSignalSynchronizationObject) \
	visit(D3DKMTUnlock) \
	visit(OpenAdapter10) \
	visit(OpenAdapter10_2) \
	visit(D3DKMTCreateAllocation) \
	visit(D3DKMTCreateContext) \
	visit(D3DKMTCreateDevice) \
	visit(D3DKMTCreateSynchronizationObject) \
	visit(D3DKMTEscape) \
	visit(D3DKMTGetContextSchedulingPriority) \
	visit(D3DKMTGetDeviceState) \
	visit(D3DKMTGetDisplayModeList) \
	visit(D3DKMTGetMultisampleMethodList) \
	visit(D3DKMTGetRuntimeData) \
	visit(D3DKMTGetSharedPrimaryHandle) \
	visit(D3DKMTLock) \
	visit(D3DKMTOpenAdapterFromHdc) \
	visit(D3DKMTOpenResource) \
	visit(D3DKMTPresent) \
	visit(D3DKMTQueryAllocationResidency) \
	visit(D3DKMTQueryResourceInfo) \
	visit(D3DKMTRender) \
	visit(D3DKMTSetAllocationPriority) \
	visit(D3DKMTSetContextSchedulingPriority) \
	visit(D3DKMTSetDisplayMode) \
	visit(D3DKMTSetGammaRamp) \
	visit(D3DKMTSetVidPnSourceOwner) \
	visit(D3DKMTWaitForSynchronizationObject) \
	visit(D3DKMTWaitForVerticalBlankEvent) \

struct dxgi_dll
{
	HMODULE dll = nullptr;
	VISIT_PROCS(ADD_FARPROC_MEMBER);
} dxgi;

VISIT_PROCS(CREATE_PROC_STUB)

void LoadDxgi()
{
	// Load real dll
	dxgi.dll = LoadDll(dtype.dxgi);

	// Load dll functions
	if (dxgi.dll)
	{
		VISIT_PROCS(LOAD_ORIGINAL_PROC);
	}
}