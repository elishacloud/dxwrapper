#pragma once

class CDirectInputDeviceKeyboard8A : public CDirectInputDeviceKeyboard8
{
private:
	DIDEVICEINSTANCEA* keyboardDeviceInfo = {};

public:
	CDirectInputDeviceKeyboard8A() : CDirectInputDeviceKeyboard8()
	{
		keyboardDeviceInfo = new DIDEVICEINSTANCEA();
		ZeroMemory(keyboardDeviceInfo, sizeof(DIDEVICEINSTANCEA));
		keyboardDeviceInfo->dwSize = sizeof(DIDEVICEINSTANCEA);
		keyboardDeviceInfo->guidInstance = GUID_SysKeyboard;
		keyboardDeviceInfo->guidProduct = GUID_SysKeyboard;
		keyboardDeviceInfo->dwDevType = DI8DEVTYPE_KEYBOARD | (DIDEVTYPEKEYBOARD_PCENH << 8);
		StringCbCopyA(keyboardDeviceInfo->tszInstanceName, 260, "Keyboard");
		StringCbCopyA(keyboardDeviceInfo->tszProductName, 260, "Keyboard");

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

	HRESULT STDMETHODCALLTYPE GetObjectInfo(LPDIDEVICEOBJECTINSTANCEA pdidoi, DWORD dwObj, DWORD dwHow)
	{
		diGlobalsInstance->LogA("KeyboardDevice->GetObjectInfo(dwObj: %x, dwHow: %x)", __FILE__, __LINE__, dwObj, dwHow);

		ZeroMemory(pdidoi, sizeof(DIDEVICEOBJECTINSTANCEW));

		// dwHow: 1, dwObj: 200 

		if (dwHow == DIPH_BYOFFSET)
		{
			pdidoi->dwSize = sizeof(DIDEVICEOBJECTINSTANCEW);
			pdidoi->guidType = GUID_Key;
			pdidoi->dwOfs = dwObj;
			pdidoi->dwType = DIDFT_BUTTON;
			diGlobalsInstance->GetKeyNameA(dwObj, pdidoi->tszName);
			return DI_OK;
		}

		return DIERR_UNSUPPORTED;
	}

	HRESULT STDMETHODCALLTYPE GetDeviceInfo(LPDIDEVICEINSTANCEA pdidi)
	{
		diGlobalsInstance->LogA("KeyboardDevice->GetDeviceInfo()", __FILE__, __LINE__);
		memcpy(pdidi, keyboardDeviceInfo, sizeof(DIDEVICEINSTANCEA));

		return DI_OK;
	}
};