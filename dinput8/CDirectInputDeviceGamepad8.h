#pragma once

class CDirectInputDeviceGamepad8
{
public:
	DIDATAFORMAT dataFormat = {};
	DWORD dwDevType = 0;
	bool isAcquired = false;

	CDirectInputDeviceGamepad8() {}

	HRESULT STDMETHODCALLTYPE Base_GetCapabilities(LPDIDEVCAPS lpDIDevCaps)
	{
		diGlobalsInstance->LogA("GamepadDevice->GetCapabilities()", __FILE__, __LINE__);

		lpDIDevCaps->dwFlags = DIDC_ATTACHED | DIDC_EMULATED;
		lpDIDevCaps->dwDevType = this->dwDevType;
		lpDIDevCaps->dwAxes = 5;
		lpDIDevCaps->dwButtons = 10;
		lpDIDevCaps->dwPOVs = 1;
		lpDIDevCaps->dwFFSamplePeriod = 0;
		lpDIDevCaps->dwFFMinTimeResolution = 0;
		lpDIDevCaps->dwFirmwareRevision = 0;
		lpDIDevCaps->dwHardwareRevision = 0;
		lpDIDevCaps->dwFFDriverVersion = 0;

		return DI_OK;
	}

	HRESULT STDMETHODCALLTYPE Base_Acquire()
	{
		diGlobalsInstance->LogA("GamepadDevice->Acquire()", __FILE__, __LINE__);

		this->isAcquired = true;

		return DI_OK;
	}

	HRESULT STDMETHODCALLTYPE Base_Unacquire()
	{
		diGlobalsInstance->LogA("GamepadDevice->Unacquire()", __FILE__, __LINE__);

		this->isAcquired = false;

		return DI_OK;
	}

	HRESULT STDMETHODCALLTYPE Base_GetDeviceState(DWORD cbData, LPVOID lpvData)
	{
		diGlobalsInstance->LogA("GamepadDevice->GetDeviceState()", __FILE__, __LINE__);

		if (!this->isAcquired)
		{
			return DIERR_INPUTLOST;
		}

		diGlobalsInstance->Lock();
		{
			ZeroMemory(lpvData, cbData);

			diGlobalsInstance->gamepadState->lX = 0;
			diGlobalsInstance->gamepadState->lY = 0;
			diGlobalsInstance->gamepadState->lZ = 0;
			diGlobalsInstance->gamepadState->lRx = 0;
			diGlobalsInstance->gamepadState->lRy = 0;
			diGlobalsInstance->gamepadState->lRz = 0;
			diGlobalsInstance->gamepadState->rglSlider[0] = 0;
			diGlobalsInstance->gamepadState->rglSlider[1] = 0;
			diGlobalsInstance->gamepadState->rgdwPOV[0] = (DWORD)-1;
			diGlobalsInstance->gamepadState->rgdwPOV[1] = (DWORD)-1;
			diGlobalsInstance->gamepadState->rgdwPOV[2] = (DWORD)-1;
			diGlobalsInstance->gamepadState->rgdwPOV[3] = (DWORD)-1;
			for (int i = 0; i < 32; i++)
			{
				diGlobalsInstance->gamepadState->rgbButtons[i] = 0;
			}

			if (cbData == sizeof(DIJOYSTATE))
			{
				memcpy(lpvData, diGlobalsInstance->gamepadState, sizeof(DIJOYSTATE));
			}
			else if (cbData == sizeof(DIJOYSTATE2))
			{
				memcpy(lpvData, diGlobalsInstance->gamepadState, sizeof(DIJOYSTATE2));
			}
			else {
				diGlobalsInstance->LogA("GamepadDevice->GetDeviceState(): Unsupported cbData: %i", __FILE__, __LINE__, cbData);
				return DIERR_UNSUPPORTED;
			}
		}
		diGlobalsInstance->Unlock();

		return DI_OK;
	}

	HRESULT STDMETHODCALLTYPE Base_GetDeviceData(DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags)
	{
		UNREFERENCED_PARAMETER(cbObjectData);
		UNREFERENCED_PARAMETER(rgdod);
		UNREFERENCED_PARAMETER(dwFlags);

		diGlobalsInstance->LogA("GamepadDevice->GetDeviceData()", __FILE__, __LINE__);

		if (pdwInOut)
		{
			*pdwInOut = 0;
		}

		return DI_OK;
	}

	HRESULT STDMETHODCALLTYPE Base_SetDataFormat(LPCDIDATAFORMAT lpdf)
	{
		diGlobalsInstance->LogA("GamepadDevice->SetDataFormat()", __FILE__, __LINE__);
		memcpy(&dataFormat, lpdf, sizeof(DIDATAFORMAT));
		return DI_OK;
	}
};