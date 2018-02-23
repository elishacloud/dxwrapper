#include <algorithm>
#include <vector>

#include "Direct3d/DepthBuffer.h"

namespace
{
	std::string bitDepthsToString(DWORD bitDepths)
	{
		std::string result;
		if (bitDepths & DDBD_8) { result += ", 8"; }
		if (bitDepths & DDBD_16) { result += ", 16"; }
		if (bitDepths & DDBD_24) { result += ", 24"; }
		if (bitDepths & DDBD_32) { result += ", 32"; }
		result = '"' + result.substr(2) + '"';
		return result;
	}

	HRESULT CALLBACK enumZBufferFormatsCallback(LPDDPIXELFORMAT lpDDPixFmt, LPVOID lpContext)
	{
		Compat::LogEnter("CompatDepthBuffer::enumZBufferFormatsCallback", lpDDPixFmt, lpContext);
		if (DDPF_ZBUFFER == lpDDPixFmt->dwFlags &&
			(static_cast<uint64_t>(1) << lpDDPixFmt->dwZBufferBitDepth) - 1 == lpDDPixFmt->dwZBitMask)
		{
			DWORD& supportedBitDepths = *reinterpret_cast<DWORD*>(lpContext);
			switch (lpDDPixFmt->dwZBufferBitDepth)
			{
			case 8: supportedBitDepths |= DDBD_8; break;
			case 16: supportedBitDepths |= DDBD_16; break;
			case 24: supportedBitDepths |= DDBD_24; break;
			case 32: supportedBitDepths |= DDBD_32; break;
			}
		}
		Compat::LogLeave("CompatDepthBuffer::enumZBufferFormatsCallback",
			lpDDPixFmt, lpContext) << D3DENUMRET_OK;
		return D3DENUMRET_OK;
	}

	GUID getDeviceGuid(const D3DDEVICEDESC& /*desc*/)
	{
		return IID_IDirect3DHALDevice;
	}

	GUID getDeviceGuid(const D3DDEVICEDESC7& desc)
	{
		return desc.deviceGUID;
	}

	template <typename TDirect3d>
	DWORD getSupportedZBufferBitDepths(CompatPtr<TDirect3d> d3d, const GUID& deviceGuid)
	{
		DWORD supportedBitDepths = 0;
		if (d3d)
		{
			d3d->EnumZBufferFormats(d3d, deviceGuid, &enumZBufferFormatsCallback, &supportedBitDepths);
		}
		return supportedBitDepths;
	}

	bool isHardwareZBufferSupported(const D3DDEVICEDESC& desc)
	{
		return (desc.dwFlags & D3DDD_DEVICEZBUFFERBITDEPTH) && 0 != desc.dwDeviceZBufferBitDepth;
	}

	bool isHardwareZBufferSupported(const D3DDEVICEDESC7& desc)
	{
		return 0 != desc.dwDeviceZBufferBitDepth &&
			(IID_IDirect3DHALDevice == desc.deviceGUID || IID_IDirect3DTnLHalDevice == desc.deviceGUID);
	}

	void logSupportedZBufferBitDepthsChanged(
		CompatPtr<IDirectDraw7> dd, const GUID& d3dGuid, DWORD oldBitDepths, DWORD newBitDepths)
	{
		struct DeviceId
		{
			GUID directDrawGuid;
			GUID direct3dGuid;

			bool operator==(const DeviceId& rhs) const
			{
				return directDrawGuid == rhs.directDrawGuid && direct3dGuid == rhs.direct3dGuid;
			}
		};

		DDDEVICEIDENTIFIER2 deviceIdentifier = {};
		dd->GetDeviceIdentifier(dd, &deviceIdentifier, 0);
		const DeviceId deviceId = { deviceIdentifier.guidDeviceIdentifier, d3dGuid };

		static std::vector<DeviceId> loggedDevices;
		if (loggedDevices.end() != std::find(loggedDevices.begin(), loggedDevices.end(), deviceId))
		{
			return;
		}
		loggedDevices.push_back(deviceId);

		Compat::Log() << "Incorrect z-buffer bit depth capabilities detected for \"" <<
			deviceIdentifier.szDescription << "\" / " <<
			(IID_IDirect3DTnLHalDevice == d3dGuid ? "Direct3D T&L HAL" : "Direct3D HAL") << ',';
		Compat::Log() << "  changed supported z-buffer bit depths from " <<
			bitDepthsToString(oldBitDepths) << " to " << bitDepthsToString(newBitDepths);
	}
}

namespace Direct3d
{
	namespace DepthBuffer
	{
		template <typename TDirect3d, typename TD3dDeviceDesc>
		void fixSupportedZBufferBitDepths(
			CompatPtr<TDirect3d> d3d, TD3dDeviceDesc& desc)
		{
			if (isHardwareZBufferSupported(desc))
			{
				const DWORD supportedBitDepths = getSupportedZBufferBitDepths(d3d, getDeviceGuid(desc));
				if (0 != supportedBitDepths && supportedBitDepths != desc.dwDeviceZBufferBitDepth)
				{
					logSupportedZBufferBitDepthsChanged(
						d3d, getDeviceGuid(desc), desc.dwDeviceZBufferBitDepth, supportedBitDepths);
					desc.dwDeviceZBufferBitDepth = supportedBitDepths;
				}
			}
		}

		template void fixSupportedZBufferBitDepths(CompatPtr<IDirect3D3>, D3DDEVICEDESC&);
		template void fixSupportedZBufferBitDepths(CompatPtr<IDirect3D7>, D3DDEVICEDESC7&);
	}
}
