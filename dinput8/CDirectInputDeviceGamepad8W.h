#pragma once

class CDirectInputDeviceGamepad8W : public CDirectInputDeviceGamepad8
{
private:
	DIDEVICEINSTANCEW* gamepadDeviceInfo = nullptr;

public:
	CDirectInputDeviceGamepad8W() : CDirectInputDeviceGamepad8()
	{
		gamepadDeviceInfo = new DIDEVICEINSTANCEW();
		ZeroMemory(gamepadDeviceInfo, sizeof(DIDEVICEINSTANCEA));
		gamepadDeviceInfo->dwSize = sizeof(DIDEVICEINSTANCEA);
		gamepadDeviceInfo->guidInstance = GUID_Xbox360Controller;
		gamepadDeviceInfo->guidProduct = GUID_Xbox360Controller;
		gamepadDeviceInfo->dwDevType = DIDEVTYPE_HID | DI8DEVTYPE_GAMEPAD | (DI8DEVTYPEGAMEPAD_STANDARD << 8);
		gamepadDeviceInfo->wUsage = HID_USAGE_GENERIC_GAMEPAD;
		gamepadDeviceInfo->wUsagePage = HID_USAGE_PAGE_GENERIC;
		StringCbCopyW(gamepadDeviceInfo->tszInstanceName, 260, L"Controller (Gamepad XBox360)");
		StringCbCopyW(gamepadDeviceInfo->tszProductName, 260, L"Controller (Gamepad XBox360)");

		this->dwDevType = gamepadDeviceInfo->dwDevType;
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

	HRESULT STDMETHODCALLTYPE GetDeviceInfo(LPDIDEVICEINSTANCEW pdidi)
	{
		diGlobalsInstance->LogA("GamepadDevice->GetDeviceInfo()", __FILE__, __LINE__);
		memcpy(pdidi, gamepadDeviceInfo, sizeof(DIDEVICEINSTANCEW));

		return DI_OK;
	}
};