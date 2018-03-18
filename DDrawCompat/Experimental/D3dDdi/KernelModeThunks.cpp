#define CINTERFACE

#include <map>

#include <d3d.h>
#include <d3dumddi.h>
#include <../km/d3dkmthk.h>

#include "DDrawCompat\DDrawLog.h"
#include "Common/Hook.h"
#include "D3dDdi/Hooks.h"
#include "D3dDdi/KernelModeThunks.h"
#include "DDraw/Surfaces/PrimarySurface.h"

namespace
{
	struct ContextInfo
	{
		D3DKMT_HANDLE device;

		ContextInfo() : device(0) {}
	};

	struct DeviceInfo
	{
		D3DKMT_HANDLE adapter;
		D3DDDI_VIDEO_PRESENT_SOURCE_ID vidPnSourceId;

		DeviceInfo() : adapter(0), vidPnSourceId(D3DDDI_ID_UNINITIALIZED) {}
	};

	std::map<D3DKMT_HANDLE, ContextInfo> g_contexts;
	std::map<D3DKMT_HANDLE, DeviceInfo> g_devices;

	decltype(D3DKMTCreateContextVirtual)* g_origD3dKmtCreateContextVirtual = nullptr;
	decltype(D3DKMTSetVidPnSourceOwner1)* g_origD3dKmtSetVidPnSourceOwner1 = nullptr;

	D3DDDI_FLIPINTERVAL_TYPE g_overrideFlipInterval = D3DDDI_FLIPINTERVAL_NOOVERRIDE;
	UINT g_presentCount = 0;

	NTSTATUS APIENTRY createDevice(D3DKMT_CREATEDEVICE* pData)
	{
		Compat::LogEnter("D3DKMTCreateDevice", pData);
		NTSTATUS result = CALL_ORIG_FUNC(D3DKMTCreateDevice)(pData);
		if (SUCCEEDED(result))
		{
			g_devices[pData->hDevice].adapter = pData->hAdapter;

			D3DKMT_SETQUEUEDLIMIT limit = {};
			limit.hDevice = pData->hDevice;
			limit.Type = D3DKMT_SET_QUEUEDLIMIT_PRESENT;
			limit.QueuedPresentLimit = 1;
			CALL_ORIG_FUNC(D3DKMTSetQueuedLimit)(&limit);
		}
		Compat::LogLeave("D3DKMTCreateDevice", pData) << result;
		return result;
	}

	NTSTATUS APIENTRY createContext(D3DKMT_CREATECONTEXT* pData)
	{
		Compat::LogEnter("D3DKMTCreateContext", pData);
		NTSTATUS result = CALL_ORIG_FUNC(D3DKMTCreateContext)(pData);
		if (SUCCEEDED(result))
		{
			g_contexts[pData->hContext].device = pData->hDevice;
		}
		Compat::LogLeave("D3DKMTCreateContext", pData) << result;
		return result;
	}

	NTSTATUS APIENTRY createContextVirtual(D3DKMT_CREATECONTEXTVIRTUAL* pData)
	{
		Compat::LogEnter("D3DKMTCreateContextVirtual", pData);
		NTSTATUS result = g_origD3dKmtCreateContextVirtual(pData);
		if (SUCCEEDED(result))
		{
			g_contexts[pData->hContext].device = pData->hDevice;
		}
		Compat::LogLeave("D3DKMTCreateContextVirtual", pData) << result;
		return result;
	}

	NTSTATUS APIENTRY destroyContext(const D3DKMT_DESTROYCONTEXT* pData)
	{
		Compat::LogEnter("D3DKMTDestroyContext", pData);
		NTSTATUS result = CALL_ORIG_FUNC(D3DKMTDestroyContext)(pData);
		if (SUCCEEDED(result))
		{
			g_contexts.erase(pData->hContext);
		}
		Compat::LogLeave("D3DKMTDestroyContext", pData) << result;
		return result;
	}

	NTSTATUS APIENTRY destroyDevice(const D3DKMT_DESTROYDEVICE* pData)
	{
		Compat::LogEnter("D3DKMTDestroyDevice", pData);
		NTSTATUS result = CALL_ORIG_FUNC(D3DKMTDestroyDevice)(pData);
		if (SUCCEEDED(result))
		{
			g_devices.erase(pData->hDevice);
		}
		Compat::LogLeave("D3DKMTDestroyDevice", pData) << result;
		return result;
	}

	bool isPresentReady(D3DKMT_HANDLE device, D3DDDI_VIDEO_PRESENT_SOURCE_ID vidPnSourceId)
	{
		D3DKMT_GETDEVICESTATE deviceState = {};
		deviceState.hDevice = device;
		deviceState.StateType = D3DKMT_DEVICESTATE_PRESENT;
		deviceState.PresentState.VidPnSourceId = vidPnSourceId;
		NTSTATUS stateResult = D3DKMTGetDeviceState(&deviceState);
		return FAILED(stateResult) ||
			g_presentCount == deviceState.PresentState.PresentStats.PresentCount ||
			0 == deviceState.PresentState.PresentStats.PresentCount;
	}

	NTSTATUS APIENTRY present(D3DKMT_PRESENT* pData)
	{
		Compat::LogEnter("D3DKMTPresent", pData);

		if (pData->Flags.Flip && D3DDDI_FLIPINTERVAL_NOOVERRIDE != g_overrideFlipInterval)
		{
			pData->FlipInterval = g_overrideFlipInterval;
		}

		++g_presentCount;
		pData->Flags.PresentCountValid = 1;
		pData->PresentCount = g_presentCount;

		NTSTATUS result = CALL_ORIG_FUNC(D3DKMTPresent)(pData);
		if (SUCCEEDED(result) &&
			1 == DDraw::PrimarySurface::getDesc().dwBackBufferCount &&
			pData->Flags.Flip &&
			D3DDDI_FLIPINTERVAL_IMMEDIATE != pData->FlipInterval &&
			D3DDDI_FLIPINTERVAL_NOOVERRIDE == g_overrideFlipInterval)
		{
			auto contextIt = g_contexts.find(pData->hContext);
			auto deviceIt = (contextIt != g_contexts.end())
				? g_devices.find(contextIt->second.device)
				: g_devices.find(pData->hDevice);
			if (deviceIt != g_devices.end())
			{
				D3DKMT_WAITFORVERTICALBLANKEVENT vbEvent = {};
				vbEvent.hAdapter = deviceIt->second.adapter;
				vbEvent.hDevice = deviceIt->first;
				vbEvent.VidPnSourceId = deviceIt->second.vidPnSourceId;

				while (!isPresentReady(deviceIt->first, deviceIt->second.vidPnSourceId))
				{
					if (FAILED(D3DKMTWaitForVerticalBlankEvent(&vbEvent)))
					{
						Sleep(1);
					}
				}
			}
		}

		Compat::LogLeave("D3DKMTPresent", pData) << result;
		return result;
	}

	NTSTATUS APIENTRY queryAdapterInfo(const D3DKMT_QUERYADAPTERINFO* pData)
	{
		Compat::LogEnter("D3DKMTQueryAdapterInfo", pData);
		NTSTATUS result = CALL_ORIG_FUNC(D3DKMTQueryAdapterInfo)(pData);
		if (SUCCEEDED(result) && KMTQAITYPE_UMDRIVERNAME == pData->Type)
		{
			auto info = static_cast<D3DKMT_UMDFILENAMEINFO*>(pData->pPrivateDriverData);
			D3dDdi::onUmdFileNameQueried(info->UmdFileName);
		}
		Compat::LogLeave("D3DKMTQueryAdapterInfo", pData) << result;
		return result;
	}

	NTSTATUS APIENTRY setQueuedLimit(const D3DKMT_SETQUEUEDLIMIT* pData)
	{
		Compat::LogEnter("D3DKMTSetQueuedLimit", pData);
		if (D3DKMT_SET_QUEUEDLIMIT_PRESENT == pData->Type)
		{
			const UINT origLimit = pData->QueuedPresentLimit;
			const_cast<D3DKMT_SETQUEUEDLIMIT*>(pData)->QueuedPresentLimit = 1;
			NTSTATUS result = CALL_ORIG_FUNC(D3DKMTSetQueuedLimit)(pData);
			const_cast<D3DKMT_SETQUEUEDLIMIT*>(pData)->QueuedPresentLimit = origLimit;
			Compat::LogLeave("D3DKMTSetQueuedLimit", pData) << result;
			return result;
		}
		NTSTATUS result = CALL_ORIG_FUNC(D3DKMTSetQueuedLimit)(pData);
		Compat::LogLeave("D3DKMTSetQueuedLimit", pData) << result;
		return result;
	}

	void processSetVidPnSourceOwner(const D3DKMT_SETVIDPNSOURCEOWNER* pData)
	{
		auto& vidPnSourceId = g_devices[pData->hDevice].vidPnSourceId;
		for (UINT i = 0; i < pData->VidPnSourceCount; ++i)
		{
			if (D3DKMT_VIDPNSOURCEOWNER_UNOWNED != pData->pType[i])
			{
				vidPnSourceId = pData->pVidPnSourceId[i];
				return;
			}
		}
		vidPnSourceId = D3DDDI_ID_UNINITIALIZED;
	}

	NTSTATUS APIENTRY setVidPnSourceOwner(const D3DKMT_SETVIDPNSOURCEOWNER* pData)
	{
		Compat::LogEnter("D3DKMTSetVidPnSourceOwner", pData);
		NTSTATUS result = CALL_ORIG_FUNC(D3DKMTSetVidPnSourceOwner)(pData);
		if (SUCCEEDED(result))
		{
			processSetVidPnSourceOwner(pData);
		}
		Compat::LogLeave("D3DKMTSetVidPnSourceOwner", pData) << result;
		return result;
	}

	NTSTATUS APIENTRY setVidPnSourceOwner1(const D3DKMT_SETVIDPNSOURCEOWNER1* pData)
	{
		Compat::LogEnter("D3DKMTSetVidPnSourceOwner1", pData);
		NTSTATUS result = g_origD3dKmtSetVidPnSourceOwner1(pData);
		if (SUCCEEDED(result))
		{
			processSetVidPnSourceOwner(&pData->Version0);
		}
		Compat::LogLeave("D3DKMTSetVidPnSourceOwner1", pData) << result;
		return result;
	}
}

namespace D3dDdi
{
	namespace KernelModeThunks
	{
		bool isPresentReady()
		{
			for (auto it : g_devices)
			{
				if (D3DDDI_ID_UNINITIALIZED != it.second.vidPnSourceId)
				{
					return ::isPresentReady(it.first, it.second.vidPnSourceId);
				}
			}
			return true;
		}

		void installHooks()
		{
			HOOK_FUNCTION(gdi32, D3DKMTCreateContext, createContext);
			HOOK_FUNCTION(gdi32, D3DKMTCreateDevice, createDevice);
			HOOK_FUNCTION(gdi32, D3DKMTDestroyContext, destroyContext);
			HOOK_FUNCTION(gdi32, D3DKMTDestroyDevice, destroyDevice);
			HOOK_FUNCTION(gdi32, D3DKMTQueryAdapterInfo, queryAdapterInfo);
			HOOK_FUNCTION(gdi32, D3DKMTPresent, present);
			HOOK_FUNCTION(gdi32, D3DKMTSetQueuedLimit, setQueuedLimit);
			HOOK_FUNCTION(gdi32, D3DKMTSetVidPnSourceOwner, setVidPnSourceOwner);

			// Functions not available in Windows Vista
			Compat::hookFunction("gdi32", "D3DKMTCreateContextVirtual",
				reinterpret_cast<void*&>(g_origD3dKmtCreateContextVirtual), createContextVirtual);
			Compat::hookFunction("gdi32", "D3DKMTSetVidPnSourceOwner1",
				reinterpret_cast<void*&>(g_origD3dKmtSetVidPnSourceOwner1), setVidPnSourceOwner1);
		}

		void overrideFlipInterval(D3DDDI_FLIPINTERVAL_TYPE flipInterval)
		{
			g_overrideFlipInterval = flipInterval;
		}

		void releaseVidPnSources()
		{
			for (auto it : g_devices)
			{
				if (D3DDDI_ID_UNINITIALIZED != it.second.vidPnSourceId)
				{
					D3DKMT_SETVIDPNSOURCEOWNER vidPnSourceOwner = {};
					vidPnSourceOwner.hDevice = it.first;
					D3DKMTSetVidPnSourceOwner(&vidPnSourceOwner);
				}
			}
		}
	}
}
