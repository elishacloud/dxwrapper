#pragma once

class CDirectInputDeviceMouse8A : public CDirectInputDeviceMouse8
{
private:
	DIDEVICEINSTANCEA* mouseDeviceInfo = nullptr;

public:
	CDirectInputDeviceMouse8A() : CDirectInputDeviceMouse8()
	{
		mouseDeviceInfo = new DIDEVICEINSTANCEA();
		ZeroMemory(mouseDeviceInfo, sizeof(DIDEVICEINSTANCEA));
		mouseDeviceInfo->dwSize = sizeof(DIDEVICEINSTANCEA);
		mouseDeviceInfo->guidInstance = GUID_SysMouse;
		mouseDeviceInfo->guidProduct = GUID_SysMouse;
		mouseDeviceInfo->dwDevType = DI8DEVTYPE_MOUSE | (DI8DEVTYPEMOUSE_UNKNOWN << 8);
		StringCbCopyA(mouseDeviceInfo->tszInstanceName, sizeof(mouseDeviceInfo->tszInstanceName), "Mouse");
		StringCbCopyA(mouseDeviceInfo->tszProductName, sizeof(mouseDeviceInfo->tszProductName), "Mouse");

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

	HRESULT STDMETHODCALLTYPE GetDeviceInfo(LPDIDEVICEINSTANCEA pdidi)
	{
		if (!pdidi || (pdidi->dwSize != sizeof(DIDEVICEINSTANCE_DX3A) && pdidi->dwSize != sizeof(DIDEVICEINSTANCEA)))
		{
			return DIERR_INVALIDPARAM;
		}

		memcpy(pdidi, mouseDeviceInfo, pdidi->dwSize);

		return DI_OK;
	}
};