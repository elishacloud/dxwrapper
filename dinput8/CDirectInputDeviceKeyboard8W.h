#pragma once

class CDirectInputDeviceKeyboard8W : public CDirectInputDeviceKeyboard8
{
private:
	DIDEVICEINSTANCEW* keyboardDeviceInfo = {};

public:
	CDirectInputDeviceKeyboard8W() : CDirectInputDeviceKeyboard8()
	{
		keyboardDeviceInfo = new DIDEVICEINSTANCEW();
		ZeroMemory(keyboardDeviceInfo, sizeof(DIDEVICEINSTANCEW));
		keyboardDeviceInfo->dwSize = sizeof(DIDEVICEINSTANCEW);
		keyboardDeviceInfo->guidInstance = GUID_SysKeyboard;
		keyboardDeviceInfo->guidProduct = GUID_SysKeyboard;
		keyboardDeviceInfo->dwDevType = DI8DEVTYPE_KEYBOARD | (DIDEVTYPEKEYBOARD_PCENH << 8);
		StringCbCopyW(keyboardDeviceInfo->tszInstanceName, 260, L"Keyboard");
		StringCbCopyW(keyboardDeviceInfo->tszProductName, 260, L"Keyboard");

		this->dwDevType = keyboardDeviceInfo->dwDevType;
	}

	HRESULT STDMETHODCALLTYPE GetCapabilities(LPDIDEVCAPS lpDIDevCaps)
	{
		return Base_GetCapabilities(lpDIDevCaps);
	}

	HRESULT STDMETHODCALLTYPE GetProperty(GUID* rguidProp, LPDIPROPHEADER pdiph)
	{

		return Base_GetProperty(rguidProp, pdiph);
	}

	HRESULT STDMETHODCALLTYPE SetProperty(GUID* rguidProp, LPCDIPROPHEADER pdiph)
	{
		return Base_SetProperty(rguidProp, pdiph);
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

	HRESULT STDMETHODCALLTYPE GetForceFeedbackState(LPDWORD pdwOut)
	{
		return Base_GetForceFeedbackState(pdwOut);
	}

	HRESULT STDMETHODCALLTYPE GetObjectInfo(LPDIDEVICEOBJECTINSTANCEW pdidoi, DWORD dwObj, DWORD dwHow)
	{
		diGlobalsInstance->LogA("KeyboardDevice->GetObjectInfo()", __FILE__, __LINE__);

		ZeroMemory(pdidoi, sizeof(DIDEVICEOBJECTINSTANCEW));

		char tmp[4096];
		wsprintfA(tmp, "dwHow: %i, dwObj: %i \r\n", dwHow, dwObj);
		OutputDebugStringA(tmp);

		// dwHow: 1, dwObj: 200 

		if (dwHow == DIPH_BYOFFSET)
		{
			pdidoi->dwSize = sizeof(DIDEVICEOBJECTINSTANCEW);
			pdidoi->guidType = GUID_Key;
			pdidoi->dwOfs = dwObj;
			pdidoi->dwType = DIDFT_BUTTON;
			diGlobalsInstance->GetKeyNameW(dwObj, pdidoi->tszName);
			return DI_OK;
		}

		return DIERR_UNSUPPORTED;
	}

	HRESULT STDMETHODCALLTYPE GetDeviceInfo(LPDIDEVICEINSTANCEW pdidi)
	{
		diGlobalsInstance->LogA("KeyboardDevice->GetDeviceInfo()", __FILE__, __LINE__);
		memcpy(pdidi, keyboardDeviceInfo, sizeof(DIDEVICEINSTANCEW));

		return DI_OK;
	}
};