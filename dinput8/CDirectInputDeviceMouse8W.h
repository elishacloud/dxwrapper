#pragma once

class CDirectInputDeviceMouse8W : public CDirectInputDeviceMouse8
{
private:
	DIDEVICEINSTANCEW* mouseDeviceInfo = nullptr;

public:
	CDirectInputDeviceMouse8W() : CDirectInputDeviceMouse8()
	{
		mouseDeviceInfo = new DIDEVICEINSTANCEW();
		ZeroMemory(mouseDeviceInfo, sizeof(DIDEVICEINSTANCEW));
		mouseDeviceInfo->dwSize = sizeof(DIDEVICEINSTANCEW);
		mouseDeviceInfo->guidInstance = GUID_SysMouse;
		mouseDeviceInfo->guidProduct = GUID_SysMouse;
		mouseDeviceInfo->dwDevType = DI8DEVTYPE_MOUSE | (DI8DEVTYPEMOUSE_UNKNOWN << 8);
		StringCbCopyW(mouseDeviceInfo->tszInstanceName, sizeof(mouseDeviceInfo->tszInstanceName), L"Mouse");
		StringCbCopyW(mouseDeviceInfo->tszProductName, sizeof(mouseDeviceInfo->tszProductName), L"Mouse");

		this->dwDevType = mouseDeviceInfo->dwDevType;
	}

	HRESULT STDMETHODCALLTYPE GetCapabilities(LPDIDEVCAPS lpDIDevCaps)
	{
		return Base_GetCapabilities(lpDIDevCaps);
	}

	HRESULT STDMETHODCALLTYPE Acquire()
	{
		return Base_Acquire();
	}

	HRESULT STDMETHODCALLTYPE Unacquire()
	{
		return Base_Unacquire();
	}

	HRESULT STDMETHODCALLTYPE GetDeviceState(DWORD cbData, LPVOID lpvData)
	{
		return Base_GetDeviceState(cbData, lpvData);
	}

	HRESULT STDMETHODCALLTYPE GetDeviceData(DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags)
	{
		return Base_GetDeviceData(cbObjectData, rgdod, pdwInOut, dwFlags);
	}

	HRESULT STDMETHODCALLTYPE SetDataFormat(LPCDIDATAFORMAT lpdf)
	{
		return Base_SetDataFormat(lpdf);
	}

	HRESULT STDMETHODCALLTYPE SetEventNotification(HANDLE hEvent)
	{
		return Base_SetEventNotification(hEvent);
	}

	HRESULT STDMETHODCALLTYPE SetCooperativeLevel(HWND hwnd, DWORD dwFlags)
	{
		return Base_SetCooperativeLevel(hwnd, dwFlags);
	}

	HRESULT STDMETHODCALLTYPE GetDeviceInfo(LPDIDEVICEINSTANCEW pdidi)
	{
		if (!pdidi || (pdidi->dwSize != sizeof(DIDEVICEINSTANCE_DX3) && pdidi->dwSize != sizeof(DIDEVICEINSTANCEA)))
		{
			return DIERR_INVALIDPARAM;
		}

		memcpy(pdidi, mouseDeviceInfo, pdidi->dwSize);

		return DI_OK;
	}
};