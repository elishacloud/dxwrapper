#pragma once

class CDirectInput8W : public CDirectInput8
{
private:
	CDirectInputDeviceMouse8W* mouseDevice = nullptr;
	CDirectInputDeviceKeyboard8W* keyboardDevice = nullptr;
	CDirectInputDeviceGamepad8W* gamepadDevice = nullptr;

	DIDEVICEINSTANCEW mouseDeviceInfo = {};
	DIDEVICEINSTANCEW keyboardDeviceInfo = {};
	DIDEVICEINSTANCEW gamepadDeviceInfo = {};

public:
	CDirectInput8W() : CDirectInput8()
	{
		mouseDevice = new CDirectInputDeviceMouse8W();
		keyboardDevice = new CDirectInputDeviceKeyboard8W();
		gamepadDevice = new CDirectInputDeviceGamepad8W();
	}

	/*** IDirectInput8W methods ***/
	HRESULT STDMETHODCALLTYPE CreateDevice(GUID* rguid, LPDIRECTINPUTDEVICE8W* lplpDirectInputDevice, LPUNKNOWN pUnkOuter)
	{
		UNREFERENCED_PARAMETER(pUnkOuter);

		if (IsEqualIID(GUID_SysMouse, *rguid))
		{
			diGlobalsInstance->LogA("CreateDevice() for GUID_SysMouse", __FILE__, __LINE__);

			*lplpDirectInputDevice = (LPDIRECTINPUTDEVICE8W)mouseDevice;
			return DI_OK;
		}
		else if (IsEqualIID(GUID_SysKeyboard, *rguid))
		{
			diGlobalsInstance->LogA("CreateDevice() for GUID_SysKeyboard", __FILE__, __LINE__);

			*lplpDirectInputDevice = (LPDIRECTINPUTDEVICE8W)keyboardDevice;
			return DI_OK;
		}
		else if ((diGlobalsInstance->enableGamepadSupport) && (IsEqualIID(GUID_Xbox360Controller, *rguid)))
		{
			diGlobalsInstance->LogA("CreateDevice() for GUID_Xbox360Controller", __FILE__, __LINE__);

			*lplpDirectInputDevice = (LPDIRECTINPUTDEVICE8W)gamepadDevice;
			return DI_OK;
		}

		diGlobalsInstance->LogA("CreateDevice() for rguid=%x-%x-%x-%x", __FILE__, __LINE__, rguid->Data1, rguid->Data2, rguid->Data3, rguid->Data4);

		return E_ABORT;
	}

	HRESULT STDMETHODCALLTYPE EnumDevices(DWORD dwDevType, LPDIENUMDEVICESCALLBACKW lpCallback, LPVOID pvRef, DWORD dwFlags)
	{
		diGlobalsInstance->LogA("EnumDevices() for dwDevType=0x%x, dwFlags=0x0", __FILE__, __LINE__, dwDevType, dwFlags);

		if ((dwDevType == DI8DEVCLASS_ALL) || (dwDevType == DI8DEVCLASS_POINTER) || (dwDevType == DI8DEVTYPE_MOUSE))
		{
			diGlobalsInstance->LogA("EnumDevices: Returning Mouse-Device", __FILE__, __LINE__);

			mouseDevice->GetDeviceInfo(&mouseDeviceInfo);
			if (lpCallback(&mouseDeviceInfo, pvRef) == DIENUM_STOP)
			{
				diGlobalsInstance->LogA("EnumDevices stopped (due to DIENUM_STOP)", __FILE__, __LINE__);
				return DI_OK;
			}
		}

		if ((dwDevType == DI8DEVCLASS_ALL) || (dwDevType == DI8DEVCLASS_KEYBOARD))
		{
			diGlobalsInstance->LogA("EnumDevices: Returning Keyboard-Device", __FILE__, __LINE__);

			keyboardDevice->GetDeviceInfo(&keyboardDeviceInfo);
			if (lpCallback(&keyboardDeviceInfo, pvRef) == DIENUM_STOP)
			{
				diGlobalsInstance->LogA("EnumDevices stopped (due to DIENUM_STOP)", __FILE__, __LINE__);
				return DI_OK;
			}
		}

		if (diGlobalsInstance->enableGamepadSupport)
		{
			if ((dwDevType == DI8DEVCLASS_ALL) || (dwDevType == DI8DEVCLASS_GAMECTRL))
			{
				diGlobalsInstance->LogA("EnumDevices: Returning Gamepad-Device", __FILE__, __LINE__);

				gamepadDevice->GetDeviceInfo(&gamepadDeviceInfo);
				if (lpCallback(&gamepadDeviceInfo, pvRef) == DIENUM_STOP)
				{
					diGlobalsInstance->LogA("EnumDevices stopped (due to DIENUM_STOP)", __FILE__, __LINE__);
					return DI_OK;
				}
			}
		}

		return DI_OK;
	}
};