#pragma once

class CDirectInput8A : public CDirectInput8
{
private:
	CDirectInputDeviceMouse8A* mouseDevice = nullptr;
	CDirectInputDeviceKeyboard8A* keyboardDevice = nullptr;
	CDirectInputDeviceGamepad8A* gamepadDevice = nullptr;

	DIDEVICEINSTANCEA mouseDeviceInfo = {};
	DIDEVICEINSTANCEA keyboardDeviceInfo = {};
	DIDEVICEINSTANCEA gamepadDeviceInfo = {};

public:
	CDirectInput8A() : CDirectInput8()
	{
		mouseDevice = new CDirectInputDeviceMouse8A();
		keyboardDevice = new CDirectInputDeviceKeyboard8A();
		gamepadDevice = new CDirectInputDeviceGamepad8A();
	}

	HRESULT STDMETHODCALLTYPE CreateDevice(GUID* rguid, LPDIRECTINPUTDEVICE8A* lplpDirectInputDevice, LPUNKNOWN pUnkOuter)
	{
		UNREFERENCED_PARAMETER(pUnkOuter);

		if (IsEqualIID(GUID_SysMouse, *rguid))
		{
			diGlobalsInstance->LogA("CreateDevice() for GUID_SysMouse", __FILE__, __LINE__);

			*lplpDirectInputDevice = (LPDIRECTINPUTDEVICE8A)mouseDevice;
			return DI_OK;
		}
		else if (IsEqualIID(GUID_SysKeyboard, *rguid))
		{
			diGlobalsInstance->LogA("CreateDevice() for GUID_SysKeyboard", __FILE__, __LINE__);

			*lplpDirectInputDevice = (LPDIRECTINPUTDEVICE8A)keyboardDevice;
			return DI_OK;
		}
		else if ((diGlobalsInstance->enableGamepadSupport) && (IsEqualIID(GUID_Xbox360Controller, *rguid)))
		{
			diGlobalsInstance->LogA("CreateDevice() for GUID_Xbox360Controller", __FILE__, __LINE__);

			*lplpDirectInputDevice = (LPDIRECTINPUTDEVICE8A)gamepadDevice;
			return DI_OK;
		}

		diGlobalsInstance->LogA("CreateDevice() for rguid = %x-%x-%x-%x", __FILE__, __LINE__, rguid->Data1, rguid->Data2, rguid->Data3, rguid->Data4);
		return E_ABORT;
	}

	HRESULT STDMETHODCALLTYPE EnumDevices(DWORD dwDevType, LPDIENUMDEVICESCALLBACKA lpCallback, LPVOID pvRef, DWORD dwFlags)
	{
		UNREFERENCED_PARAMETER(dwFlags);

		//diGlobalsInstance->LogA("EnumDevices() for dwDevType=%x, dwFlags=%x", __FILE__, __LINE__, dwDevType, dwFlags);

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