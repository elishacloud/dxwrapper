#pragma once

class CDirectInputDeviceMouse8
{
private:
	DIDATAFORMAT dataFormat = {};
	bool exclusiveMode = false;
	bool isAquired = false;
	HWND hWndForegroundWindow = nullptr;

public:
	CDirectInputDeviceMouse8()
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ") for raw mouse input!");

		if (!diGlobalsInstance)
		{
			diGlobalsInstance = new CDirectInput8Globals();
		}

		if (diGlobalsInstance)
		{
			diGlobalsInstance->AddRef();
		}
	}
	~CDirectInputDeviceMouse8()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");

		if (diGlobalsInstance)
		{
			diGlobalsInstance->Release();
		}
	}

	bool CheckInterface()
	{
		return (diGlobalsInstance && diGlobalsInstance->CheckInterface());
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
		if (!diGlobalsInstance)
		{
			return DIERR_GENERIC;
		}

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
		if (!diGlobalsInstance)
		{
			return DIERR_GENERIC;
		}

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
			// Don't return overflow as just using mouse state
		}
		// Flush buffer
		else if (rgdod == nullptr && !isPeek)
		{
			// Don't flush buffer as just using mouse state
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
		if (!diGlobalsInstance)
		{
			return DIERR_GENERIC;
		}

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
};