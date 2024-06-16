#pragma once

class CDirectInputDeviceGamepad8A : public CDirectInputDeviceGamepad8
{
private:
	DIDEVICEINSTANCEA* gamepadDeviceInfo = nullptr;

public:
	CDirectInputDeviceGamepad8A() : CDirectInputDeviceGamepad8()
	{
		gamepadDeviceInfo = new DIDEVICEINSTANCEA();
		ZeroMemory(gamepadDeviceInfo, sizeof(DIDEVICEINSTANCEA));
		gamepadDeviceInfo->dwSize = sizeof(DIDEVICEINSTANCEA);
		gamepadDeviceInfo->guidInstance = GUID_Xbox360Controller;
		gamepadDeviceInfo->guidProduct = GUID_Xbox360Controller;
		gamepadDeviceInfo->dwDevType = DIDEVTYPE_HID | DI8DEVTYPE_GAMEPAD | (DI8DEVTYPEGAMEPAD_STANDARD << 8);
		gamepadDeviceInfo->wUsage = HID_USAGE_GENERIC_GAMEPAD;
		gamepadDeviceInfo->wUsagePage = HID_USAGE_PAGE_GENERIC;
		StringCbCopyA(gamepadDeviceInfo->tszInstanceName, 260, "Controller (Gamepad XBox360)");
		StringCbCopyA(gamepadDeviceInfo->tszProductName, 260, "Controller (Gamepad XBox360)");

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

	HRESULT STDMETHODCALLTYPE EnumObjects(LPDIENUMDEVICEOBJECTSCALLBACKA lpCallback, LPVOID pvRef, DWORD dwFlags)
	{

		diGlobalsInstance->LogA("GamepadDevice->EnumObjects(), dwFlags: %x", __FILE__, __LINE__, dwFlags);

		if ((dwFlags & DIDFT_ABSAXIS) > 0)
		{
			diGlobalsInstance->LogA(" ->DIDFT_ABSAXIS", __FILE__, __LINE__);
		}
		if ((dwFlags & DIDFT_ALIAS) > 0)
		{
			diGlobalsInstance->LogA(" ->DIDFT_ALIAS", __FILE__, __LINE__);
		}
		if ((dwFlags & DIDFT_ALL) > 0)
		{
			diGlobalsInstance->LogA(" ->DIDFT_ALL", __FILE__, __LINE__);
		}
		if ((dwFlags & DIDFT_AXIS) > 0)
		{
			diGlobalsInstance->LogA(" ->DIDFT_AXIS", __FILE__, __LINE__);
		}
		if ((dwFlags & DIDFT_BUTTON) > 0)
		{
			diGlobalsInstance->LogA(" ->DIDFT_BUTTON", __FILE__, __LINE__);
		}
		if ((dwFlags & DIDFT_COLLECTION) > 0)
		{
			diGlobalsInstance->LogA(" ->DIDFT_COLLECTION", __FILE__, __LINE__);
		}
		if ((dwFlags & DIDFT_FFACTUATOR) > 0)
		{
			diGlobalsInstance->LogA(" ->DIDFT_FFACTUATOR", __FILE__, __LINE__);
		}
		if ((dwFlags & DIDFT_FFEFFECTTRIGGER) > 0)
		{
			diGlobalsInstance->LogA(" ->DIDFT_FFEFFECTTRIGGER", __FILE__, __LINE__);
		}
		if ((dwFlags & DIDFT_NOCOLLECTION) > 0)
		{
			diGlobalsInstance->LogA(" ->DIDFT_NOCOLLECTION", __FILE__, __LINE__);
		}
		if ((dwFlags & DIDFT_NODATA) > 0)
		{
			diGlobalsInstance->LogA(" ->DIDFT_NODATA", __FILE__, __LINE__);
		}
		if ((dwFlags & DIDFT_OUTPUT) > 0)
		{
			diGlobalsInstance->LogA(" ->DIDFT_OUTPUT", __FILE__, __LINE__);
		}
		if ((dwFlags & DIDFT_POV) > 0)
		{
			diGlobalsInstance->LogA(" ->DIDFT_POV", __FILE__, __LINE__);
		}
		if ((dwFlags & DIDFT_PSHBUTTON) > 0)
		{
			diGlobalsInstance->LogA(" ->DIDFT_PSHBUTTON", __FILE__, __LINE__);
		}
		if ((dwFlags & DIDFT_RELAXIS) > 0)
		{
			diGlobalsInstance->LogA(" ->DIDFT_RELAXIS", __FILE__, __LINE__);
		}
		if ((dwFlags & DIDFT_TGLBUTTON) > 0)
		{
			diGlobalsInstance->LogA(" ->DIDFT_TGLBUTTON", __FILE__, __LINE__);
		}
		if ((dwFlags & DIDFT_VENDORDEFINED) > 0)
		{
			diGlobalsInstance->LogA(" ->DIDFT_VENDORDEFINED", __FILE__, __LINE__);
		}

		DIDEVICEOBJECTINSTANCEA* someObject = new DIDEVICEOBJECTINSTANCEA();
		ZeroMemory(someObject, sizeof(DIDEVICEOBJECTINSTANCEA));
		someObject->dwSize = sizeof(DIDEVICEOBJECTINSTANCEA);
		someObject->guidType = GUID_XAxis;
		someObject->dwOfs = 0;
		someObject->dwType = DIDFT_ABSAXIS;
		someObject->dwFlags = DIDOI_GUIDISUSAGE;
		lstrcpyA(someObject->tszName, "X-Axis");
		someObject->dwFFMaxForce = 0;
		someObject->dwFFForceResolution = 0;
		someObject->wCollectionNumber = 0;
		someObject->wDesignatorIndex = 0;
		someObject->wUsagePage = 0;
		someObject->wUsage = 0;
		someObject->dwDimension = 0;
		someObject->wExponent = 0;
		someObject->wReportId = 0;

		lpCallback(someObject, pvRef);

		return DI_OK;
	}

	HRESULT STDMETHODCALLTYPE GetDeviceInfo(LPDIDEVICEINSTANCEA pdidi)
	{
		diGlobalsInstance->LogA("GamepadDevice->GetDeviceInfo()", __FILE__, __LINE__);
		memcpy(pdidi, gamepadDeviceInfo, sizeof(DIDEVICEINSTANCEA));

		return DI_OK;
	}
};