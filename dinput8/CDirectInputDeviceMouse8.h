#pragma once

class CDirectInputDeviceMouse8
{
private:
	DIDATAFORMAT dataFormat = {};
	bool exclusiveMode = false;
	bool isAquired = false;
	HWND hWndForegroundWindow = nullptr;

public:
	DWORD dwDevType = 0;

	CDirectInputDeviceMouse8()
	{
		if (!hidDllLoaded)
		{
			LoadHidLibrary();
		}
		if (!diGlobalsInstance)
		{
			diGlobalsInstance = new CDirectInput8Globals();
		}
		Logging::Log() << __FUNCTION__ << " Using Raw mouse input for DirectInput8!";
	}
	~CDirectInputDeviceMouse8()
	{
		diGlobalsInstance->mouseEventHandle = nullptr;
	}

	HRESULT STDMETHODCALLTYPE Base_GetCapabilities(LPDIDEVCAPS lpDIDevCaps)
	{
		if (!lpDIDevCaps || (lpDIDevCaps->dwSize != sizeof(DIDEVCAPS_DX3) && lpDIDevCaps->dwSize != sizeof(DIDEVCAPS)))
		{
			return DIERR_INVALIDPARAM;
		}

		lpDIDevCaps->dwFlags = DIDC_ATTACHED | DIDC_EMULATED;
		lpDIDevCaps->dwDevType = this->dwDevType;
		lpDIDevCaps->dwAxes = 3;
		lpDIDevCaps->dwButtons = 3;
		lpDIDevCaps->dwPOVs = 0;
		if (lpDIDevCaps->dwSize == sizeof(DIDEVCAPS))
		{
			lpDIDevCaps->dwFFSamplePeriod = 0;
			lpDIDevCaps->dwFFMinTimeResolution = 0;
			lpDIDevCaps->dwFirmwareRevision = 0;
			lpDIDevCaps->dwHardwareRevision = 0;
			lpDIDevCaps->dwFFDriverVersion = 0;
		}

		return DI_OK;
	}

	HRESULT STDMETHODCALLTYPE Base_Acquire()
	{
		bool isAlreadyAquired = this->isAquired;

		this->hWndForegroundWindow = GetForegroundWindow();

		this->Base_AcquireInternal();
		this->isAquired = true;

		return isAlreadyAquired ? S_FALSE : DI_OK;
	}

	void Base_AcquireInternal()
	{
		if (!this->isAquired)
		{
			return;
		}

		// Initial mouse capture
		// (Or capture lost due to 'ALT+TAB'-ing)
		HWND hWndCapture = GetCapture();
		if (hWndCapture != this->hWndForegroundWindow)
		{
			SetCapture(this->hWndForegroundWindow);

			if (this->exclusiveMode)
			{
				ShowCursor(false);
			}
		}
	}

	HRESULT STDMETHODCALLTYPE Base_Unacquire()
	{
		bool isAlreadyAquired = this->isAquired;

		ReleaseCapture();

		if (this->exclusiveMode)
		{
			ShowCursor(true);
		}

		this->isAquired = false;

		return !isAlreadyAquired ? DI_NOEFFECT : DI_OK;
	}

	HRESULT STDMETHODCALLTYPE Base_GetDeviceState(DWORD cbData, LPVOID lpvData)
	{
		if (!this->isAquired)
		{
			return DIERR_INPUTLOST;
		}

		this->Base_AcquireInternal();

		diGlobalsInstance->Lock();
		{
			// Copy current state to lpvData
			memcpy(lpvData, diGlobalsInstance->mouseStateDeviceData, min(cbData, sizeof(DIMOUSESTATE)));

			// Set axis movement to zero:
			// (=Mark last movement data as fetched)
			diGlobalsInstance->mouseStateDeviceData->lX = 0;
			diGlobalsInstance->mouseStateDeviceData->lY = 0;
			diGlobalsInstance->mouseStateDeviceData->lZ = 0;
		}
		diGlobalsInstance->Unlock();

		return DI_OK;
	}

	HRESULT STDMETHODCALLTYPE Base_GetDeviceData(DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags)
	{
		if (!this->isAquired)
		{
			return DIERR_INPUTLOST;
		}

		if (rgdod && (cbObjectData != sizeof(DIDEVICEOBJECTDATA) && cbObjectData != sizeof(DIDEVICEOBJECTDATA_DX3)))
		{
			return DIERR_INVALIDPARAM;
		}

		int dwOut = 0;

		bool isPeek = ((dwFlags & DIGDD_PEEK) > 0);

		// ToDo: add support for peek flag
		UNREFERENCED_PARAMETER(isPeek);

		// Determine timestamp:
		__int64 fTime = 0;
		GetSystemTimeAsFileTime((FILETIME*)&fTime);
		fTime = fTime / 1000;

		this->Base_AcquireInternal();

		diGlobalsInstance->Lock();
		{
			if ((*pdwInOut == 0) && (rgdod == NULL))
			{
				// Checking for overflow
			}
			else if ((*pdwInOut == INFINITE) && (rgdod == NULL))
			{
				// Flush buffer
				ZeroMemory(diGlobalsInstance->keyStates, sizeof(diGlobalsInstance->keyStates));
				ZeroMemory(diGlobalsInstance->gameKeyStates, sizeof(diGlobalsInstance->gameKeyStates));
			}
			else if (rgdod)
			{
				memset(rgdod, 0, *pdwInOut * cbObjectData);

				if (true)
				{
					LPDIDEVICEOBJECTDATA p_rgdod = rgdod;
					for (DWORD i = 0; i < *pdwInOut; i++)
					{
						if (diGlobalsInstance->mouseStateDeviceData->lX != 0)
						{
							// Sending DIMOFS_X
							p_rgdod->dwData = diGlobalsInstance->mouseStateDeviceData->lX;
							p_rgdod->dwOfs = DIMOFS_X;
							p_rgdod->dwSequence = diGlobalsInstance->dwSequence;
							p_rgdod->dwTimeStamp = (DWORD)fTime;
							if (cbObjectData == sizeof(DIDEVICEOBJECTDATA))
							{
								p_rgdod->uAppData = NULL;
							}

							diGlobalsInstance->dwSequence++;
							diGlobalsInstance->mouseStateDeviceData->lX = 0;
							dwOut++;
						}
						else if (diGlobalsInstance->mouseStateDeviceData->lY != 0)
						{
							// Sending DIMOFS_Y
							p_rgdod->dwData = diGlobalsInstance->mouseStateDeviceData->lY;
							p_rgdod->dwOfs = DIMOFS_Y;
							p_rgdod->dwSequence = diGlobalsInstance->dwSequence;
							p_rgdod->dwTimeStamp = (DWORD)fTime;
							if (cbObjectData == sizeof(DIDEVICEOBJECTDATA))
							{
								p_rgdod->uAppData = NULL;
							}

							diGlobalsInstance->dwSequence++;
							diGlobalsInstance->mouseStateDeviceData->lY = 0;
							dwOut++;
						}
						else if (diGlobalsInstance->mouseStateDeviceData->lZ != 0)
						{
							// Sending DIMOFS_Z
							p_rgdod->dwData = diGlobalsInstance->mouseStateDeviceData->lZ;
							p_rgdod->dwOfs = DIMOFS_Z;
							p_rgdod->dwSequence = diGlobalsInstance->dwSequence;
							p_rgdod->dwTimeStamp = (DWORD)fTime;
							if (cbObjectData == sizeof(DIDEVICEOBJECTDATA))
							{
								p_rgdod->uAppData = NULL;
							}

							diGlobalsInstance->dwSequence++;
							diGlobalsInstance->mouseStateDeviceData->lZ = 0;
							dwOut++;
						}
						else if (diGlobalsInstance->mouseStateDeviceData->rgbButtons[0] != diGlobalsInstance->mouseStateDeviceDataGame->rgbButtons[0])
						{
							// Sending DIMOFS_BUTTON0
							p_rgdod->dwData = diGlobalsInstance->mouseStateDeviceData->rgbButtons[0];
							p_rgdod->dwOfs = DIMOFS_BUTTON0;
							p_rgdod->dwSequence = diGlobalsInstance->dwSequence;
							p_rgdod->dwTimeStamp = (DWORD)fTime;
							if (cbObjectData == sizeof(DIDEVICEOBJECTDATA))
							{
								p_rgdod->uAppData = NULL;
							}

							diGlobalsInstance->dwSequence++;
							diGlobalsInstance->mouseStateDeviceDataGame->rgbButtons[0] = diGlobalsInstance->mouseStateDeviceData->rgbButtons[0];
							dwOut++;
						}
						else if (diGlobalsInstance->mouseStateDeviceData->rgbButtons[1] != diGlobalsInstance->mouseStateDeviceDataGame->rgbButtons[1])
						{
							// Sending DIMOFS_BUTTON1
							p_rgdod->dwData = diGlobalsInstance->mouseStateDeviceData->rgbButtons[1];
							p_rgdod->dwOfs = DIMOFS_BUTTON1;
							p_rgdod->dwSequence = diGlobalsInstance->dwSequence;
							p_rgdod->dwTimeStamp = (DWORD)fTime;
							if (cbObjectData == sizeof(DIDEVICEOBJECTDATA))
							{
								p_rgdod->uAppData = NULL;
							}

							diGlobalsInstance->dwSequence++;
							diGlobalsInstance->mouseStateDeviceDataGame->rgbButtons[1] = diGlobalsInstance->mouseStateDeviceData->rgbButtons[1];
							dwOut++;
						}
						else if (diGlobalsInstance->mouseStateDeviceData->rgbButtons[2] != diGlobalsInstance->mouseStateDeviceDataGame->rgbButtons[2])
						{
							// Sending DIMOFS_BUTTON2
							p_rgdod->dwData = diGlobalsInstance->mouseStateDeviceData->rgbButtons[2];
							p_rgdod->dwOfs = DIMOFS_BUTTON2;
							p_rgdod->dwSequence = diGlobalsInstance->dwSequence;
							p_rgdod->dwTimeStamp = (DWORD)fTime;
							if (cbObjectData == sizeof(DIDEVICEOBJECTDATA))
							{
								p_rgdod->uAppData = NULL;
							}

							diGlobalsInstance->dwSequence++;
							diGlobalsInstance->mouseStateDeviceDataGame->rgbButtons[2] = diGlobalsInstance->mouseStateDeviceData->rgbButtons[2];
							dwOut++;
						}
						p_rgdod = (LPDIDEVICEOBJECTDATA)((DWORD)p_rgdod + cbObjectData);
					}
				}
			}
		}
		diGlobalsInstance->Unlock();

		if (pdwInOut)
		{
			*pdwInOut = dwOut;
		}

		return DI_OK;
	}

	HRESULT STDMETHODCALLTYPE Base_SetDataFormat(LPCDIDATAFORMAT lpdf)
	{
		if (!lpdf || lpdf->dwSize != sizeof(DIDATAFORMAT))
		{
			return DIERR_INVALIDPARAM;
		}

		if (this->isAquired)
		{
			return DIERR_ACQUIRED;
		}

		memcpy(&dataFormat, lpdf, sizeof(DIDATAFORMAT));

		return DI_OK;
	}

	HRESULT STDMETHODCALLTYPE Base_SetEventNotification(HANDLE hEvent)
	{

		diGlobalsInstance->Lock();
		{
			diGlobalsInstance->mouseEventHandle = hEvent;
		}
		diGlobalsInstance->Unlock();

		return DI_OK;
	}

	HRESULT STDMETHODCALLTYPE Base_SetCooperativeLevel(HWND hwnd, DWORD dwFlags)
	{
		UNREFERENCED_PARAMETER(hwnd);

		this->exclusiveMode = ((dwFlags & DISCL_EXCLUSIVE) > 0);

		return DI_OK;
	}
};