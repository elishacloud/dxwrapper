#pragma once

class CDirectInputDeviceMouse8
{
private:
	DIDATAFORMAT dataFormat = {};
	bool exclusiveMode = false;
	bool isAquired = false;
	HWND hWndForegroundWindow = nullptr;
	DIDEVICEINSTANCEA mouseDeviceInfoA = {};
	DIDEVICEINSTANCEW mouseDeviceInfoW = {};

public:
	DWORD dwDevType = 0;

	CDirectInputDeviceMouse8()
	{
		Logging::Log() << __FUNCTION__ << " Using Raw mouse input for DirectInput8!";

		if (!diGlobalsInstance)
		{
			diGlobalsInstance = new CDirectInput8Globals();
		}

		mouseDeviceInfoA.dwSize = sizeof(DIDEVICEINSTANCEA);
		mouseDeviceInfoA.guidInstance = GUID_SysMouse;
		mouseDeviceInfoA.guidProduct = GUID_SysMouse;
		mouseDeviceInfoA.dwDevType = DI8DEVTYPE_MOUSE | (DI8DEVTYPEMOUSE_UNKNOWN << 8);
		strcpy_s(mouseDeviceInfoA.tszInstanceName, sizeof(mouseDeviceInfoA.tszInstanceName), "Mouse");
		strcpy_s(mouseDeviceInfoA.tszProductName, sizeof(mouseDeviceInfoA.tszProductName), "Mouse");

		mouseDeviceInfoW.dwSize = sizeof(DIDEVICEINSTANCEW);
		mouseDeviceInfoW.guidInstance = GUID_SysMouse;
		mouseDeviceInfoW.guidProduct = GUID_SysMouse;
		mouseDeviceInfoW.dwDevType = DI8DEVTYPE_MOUSE | (DI8DEVTYPEMOUSE_UNKNOWN << 8);
		wcscpy_s(mouseDeviceInfoW.tszInstanceName, sizeof(mouseDeviceInfoW.tszInstanceName) / sizeof(WCHAR), L"Mouse");
		wcscpy_s(mouseDeviceInfoW.tszProductName, sizeof(mouseDeviceInfoW.tszProductName) / sizeof(WCHAR), L"Mouse");

		this->dwDevType = mouseDeviceInfoA.dwDevType;
	}
	~CDirectInputDeviceMouse8()
	{
		diGlobalsInstance->mouseEventHandle = nullptr;
	}

	HRESULT STDMETHODCALLTYPE GetCapabilities(LPDIDEVCAPS lpDIDevCaps)
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

	void AcquireInternal()
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

	HRESULT STDMETHODCALLTYPE Acquire()
	{
		bool isAlreadyAquired = this->isAquired;

		this->hWndForegroundWindow = GetForegroundWindow();

		this->AcquireInternal();
		this->isAquired = true;

		return isAlreadyAquired ? S_FALSE : DI_OK;
	}

	HRESULT STDMETHODCALLTYPE Unacquire()
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

	HRESULT STDMETHODCALLTYPE GetDeviceState(DWORD cbData, LPVOID lpvData)
	{
		if (!this->isAquired)
		{
			return DIERR_INPUTLOST;
		}

		this->AcquireInternal();

		diGlobalsInstance->Lock();

		// Copy current state to lpvData
		memcpy(lpvData, &diGlobalsInstance->mouseStateDeviceData, min(cbData, sizeof(DIMOUSESTATE)));

		// Set axis movement to zero:
		// (=Mark last movement data as fetched)
		diGlobalsInstance->mouseStateDeviceData.lX = 0;
		diGlobalsInstance->mouseStateDeviceData.lY = 0;
		diGlobalsInstance->mouseStateDeviceData.lZ = 0;

		diGlobalsInstance->Unlock();

		return DI_OK;
	}

	HRESULT STDMETHODCALLTYPE GetDeviceData(DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags)
	{
		if (!this->isAquired)
		{
			return DIERR_INPUTLOST;
		}

		if (!pdwInOut || (rgdod && cbObjectData != sizeof(DIDEVICEOBJECTDATA) && cbObjectData != sizeof(DIDEVICEOBJECTDATA_DX3)))
		{
			return DIERR_INVALIDPARAM;
		}

		bool isPeek = ((dwFlags & DIGDD_PEEK) > 0);

		if (rgdod == nullptr && isPeek)
		{
			*pdwInOut = min(6, *pdwInOut);	// Just hard code to 6 as the current number of possible buffered events
			return DI_OK;
		}

		DWORD dwOut = 0;

		// Determine timestamp:
		__int64 fTime = 0;
		GetSystemTimeAsFileTime((FILETIME*)&fTime);
		fTime = fTime / 1000;

		this->AcquireInternal();

		diGlobalsInstance->Lock();

		// Checking for overflow
		if (rgdod == nullptr && *pdwInOut == 0)
		{
			if (diGlobalsInstance->mouseStateDeviceData.lX != 0 ||
				diGlobalsInstance->mouseStateDeviceData.lY != 0 ||
				diGlobalsInstance->mouseStateDeviceData.lZ != 0 ||
				diGlobalsInstance->mouseStateDeviceData.rgbButtons[0] != diGlobalsInstance->mouseStateDeviceDataGame.rgbButtons[0] ||
				diGlobalsInstance->mouseStateDeviceData.rgbButtons[1] != diGlobalsInstance->mouseStateDeviceDataGame.rgbButtons[1] ||
				diGlobalsInstance->mouseStateDeviceData.rgbButtons[2] != diGlobalsInstance->mouseStateDeviceDataGame.rgbButtons[2])
			{
				return DI_BUFFEROVERFLOW;
			}
		}
		// Flush buffer
		else if (rgdod == nullptr && !isPeek)
		{
			if (dwOut < *pdwInOut && diGlobalsInstance->mouseStateDeviceData.lX != 0)
			{
				diGlobalsInstance->dwSequence++;
				diGlobalsInstance->mouseStateDeviceData.lX = 0;
				dwOut++;
			}
			else if (dwOut < *pdwInOut && diGlobalsInstance->mouseStateDeviceData.lY != 0)
			{
				diGlobalsInstance->dwSequence++;
				diGlobalsInstance->mouseStateDeviceData.lY = 0;
				dwOut++;
			}
			else if (dwOut < *pdwInOut && diGlobalsInstance->mouseStateDeviceData.lZ != 0)
			{
				diGlobalsInstance->dwSequence++;
				diGlobalsInstance->mouseStateDeviceData.lZ = 0;
				dwOut++;
			}
			else if (dwOut < *pdwInOut && diGlobalsInstance->mouseStateDeviceData.rgbButtons[0] != diGlobalsInstance->mouseStateDeviceDataGame.rgbButtons[0])
			{
				diGlobalsInstance->dwSequence++;
				diGlobalsInstance->mouseStateDeviceDataGame.rgbButtons[0] = diGlobalsInstance->mouseStateDeviceData.rgbButtons[0];
				dwOut++;
			}
			else if (dwOut < *pdwInOut && diGlobalsInstance->mouseStateDeviceData.rgbButtons[1] != diGlobalsInstance->mouseStateDeviceDataGame.rgbButtons[1])
			{
				diGlobalsInstance->dwSequence++;
				diGlobalsInstance->mouseStateDeviceDataGame.rgbButtons[1] = diGlobalsInstance->mouseStateDeviceData.rgbButtons[1];
				dwOut++;
			}
			else if (dwOut < *pdwInOut && diGlobalsInstance->mouseStateDeviceData.rgbButtons[2] != diGlobalsInstance->mouseStateDeviceDataGame.rgbButtons[2])
			{
				diGlobalsInstance->dwSequence++;
				diGlobalsInstance->mouseStateDeviceDataGame.rgbButtons[2] = diGlobalsInstance->mouseStateDeviceData.rgbButtons[2];
				dwOut++;
			}
		}
		// Full device object data
		else if (rgdod)
		{
			memset(rgdod, 0, *pdwInOut * cbObjectData);

			LPDIDEVICEOBJECTDATA p_rgdod = rgdod;
			for (DWORD i = 0; i < *pdwInOut; i++)
			{
				// Sending DIMOFS_X
				if (diGlobalsInstance->mouseStateDeviceData.lX != 0)
				{
					p_rgdod->dwData = diGlobalsInstance->mouseStateDeviceData.lX;
					p_rgdod->dwOfs = DIMOFS_X;
					p_rgdod->dwSequence = diGlobalsInstance->dwSequence;
					p_rgdod->dwTimeStamp = (DWORD)fTime;
					if (cbObjectData == sizeof(DIDEVICEOBJECTDATA))
					{
						p_rgdod->uAppData = NULL;
					}

					diGlobalsInstance->dwSequence++;
					diGlobalsInstance->mouseStateDeviceData.lX = 0;
					dwOut++;
				}
				// Sending DIMOFS_Y
				else if (diGlobalsInstance->mouseStateDeviceData.lY != 0)
				{
					p_rgdod->dwData = diGlobalsInstance->mouseStateDeviceData.lY;
					p_rgdod->dwOfs = DIMOFS_Y;
					p_rgdod->dwSequence = diGlobalsInstance->dwSequence;
					p_rgdod->dwTimeStamp = (DWORD)fTime;
					if (cbObjectData == sizeof(DIDEVICEOBJECTDATA))
					{
						p_rgdod->uAppData = NULL;
					}

					diGlobalsInstance->dwSequence++;
					diGlobalsInstance->mouseStateDeviceData.lY = 0;
					dwOut++;
				}
				// Sending DIMOFS_Z
				else if (diGlobalsInstance->mouseStateDeviceData.lZ != 0)
				{
					p_rgdod->dwData = diGlobalsInstance->mouseStateDeviceData.lZ;
					p_rgdod->dwOfs = DIMOFS_Z;
					p_rgdod->dwSequence = diGlobalsInstance->dwSequence;
					p_rgdod->dwTimeStamp = (DWORD)fTime;
					if (cbObjectData == sizeof(DIDEVICEOBJECTDATA))
					{
						p_rgdod->uAppData = NULL;
					}

					diGlobalsInstance->dwSequence++;
					diGlobalsInstance->mouseStateDeviceData.lZ = 0;
					dwOut++;
				}
				// Sending DIMOFS_BUTTON0
				else if (diGlobalsInstance->mouseStateDeviceData.rgbButtons[0] != diGlobalsInstance->mouseStateDeviceDataGame.rgbButtons[0])
				{
					p_rgdod->dwData = diGlobalsInstance->mouseStateDeviceData.rgbButtons[0];
					p_rgdod->dwOfs = DIMOFS_BUTTON0;
					p_rgdod->dwSequence = diGlobalsInstance->dwSequence;
					p_rgdod->dwTimeStamp = (DWORD)fTime;
					if (cbObjectData == sizeof(DIDEVICEOBJECTDATA))
					{
						p_rgdod->uAppData = NULL;
					}

					diGlobalsInstance->dwSequence++;
					diGlobalsInstance->mouseStateDeviceDataGame.rgbButtons[0] = diGlobalsInstance->mouseStateDeviceData.rgbButtons[0];
					dwOut++;
				}
				// Sending DIMOFS_BUTTON1
				else if (diGlobalsInstance->mouseStateDeviceData.rgbButtons[1] != diGlobalsInstance->mouseStateDeviceDataGame.rgbButtons[1])
				{
					p_rgdod->dwData = diGlobalsInstance->mouseStateDeviceData.rgbButtons[1];
					p_rgdod->dwOfs = DIMOFS_BUTTON1;
					p_rgdod->dwSequence = diGlobalsInstance->dwSequence;
					p_rgdod->dwTimeStamp = (DWORD)fTime;
					if (cbObjectData == sizeof(DIDEVICEOBJECTDATA))
					{
						p_rgdod->uAppData = NULL;
					}

					diGlobalsInstance->dwSequence++;
					diGlobalsInstance->mouseStateDeviceDataGame.rgbButtons[1] = diGlobalsInstance->mouseStateDeviceData.rgbButtons[1];
					dwOut++;
				}
				// Sending DIMOFS_BUTTON2
				else if (diGlobalsInstance->mouseStateDeviceData.rgbButtons[2] != diGlobalsInstance->mouseStateDeviceDataGame.rgbButtons[2])
				{
					p_rgdod->dwData = diGlobalsInstance->mouseStateDeviceData.rgbButtons[2];
					p_rgdod->dwOfs = DIMOFS_BUTTON2;
					p_rgdod->dwSequence = diGlobalsInstance->dwSequence;
					p_rgdod->dwTimeStamp = (DWORD)fTime;
					if (cbObjectData == sizeof(DIDEVICEOBJECTDATA))
					{
						p_rgdod->uAppData = NULL;
					}

					diGlobalsInstance->dwSequence++;
					diGlobalsInstance->mouseStateDeviceDataGame.rgbButtons[2] = diGlobalsInstance->mouseStateDeviceData.rgbButtons[2];
					dwOut++;
				}
				// No more data to sent
				else
				{
					break;
				}

				p_rgdod = (LPDIDEVICEOBJECTDATA)((DWORD)p_rgdod + cbObjectData);
			}
		}

		diGlobalsInstance->Unlock();

		*pdwInOut = dwOut;

		return DI_OK;	// Always just return OK
	}

	HRESULT STDMETHODCALLTYPE SetDataFormat(LPCDIDATAFORMAT lpdf)
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

	HRESULT STDMETHODCALLTYPE SetEventNotification(HANDLE hEvent)
	{

		diGlobalsInstance->Lock();

		diGlobalsInstance->mouseEventHandle = hEvent;

		diGlobalsInstance->Unlock();

		return DI_OK;
	}

	HRESULT STDMETHODCALLTYPE SetCooperativeLevel(HWND hwnd, DWORD dwFlags)
	{
		UNREFERENCED_PARAMETER(hwnd);

		this->exclusiveMode = ((dwFlags & DISCL_EXCLUSIVE) > 0);

		return DI_OK;
	}

	HRESULT STDMETHODCALLTYPE GetDeviceInfo(LPDIDEVICEINSTANCEA pdidi)
	{
		if (!pdidi || (pdidi->dwSize != sizeof(DIDEVICEINSTANCE_DX3A) && pdidi->dwSize != sizeof(DIDEVICEINSTANCEA)))
		{
			return DIERR_INVALIDPARAM;
		}

		memcpy(pdidi, &mouseDeviceInfoA, pdidi->dwSize);

		return DI_OK;
	}

	HRESULT STDMETHODCALLTYPE GetDeviceInfo(LPDIDEVICEINSTANCEW pdidi)
	{
		if (!pdidi || (pdidi->dwSize != sizeof(DIDEVICEINSTANCE_DX3W) && pdidi->dwSize != sizeof(DIDEVICEINSTANCEW)))
		{
			return DIERR_INVALIDPARAM;
		}

		memcpy(pdidi, &mouseDeviceInfoW, pdidi->dwSize);

		return DI_OK;
	}
};