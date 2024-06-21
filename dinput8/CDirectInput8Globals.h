#pragma once

#include "hid.h"
#include "Dllmain\Dllmain.h"

class CDirectInput8Globals
{
private:
	DWORD threadId = 0;
	HANDLE hThread = 0;
	CRITICAL_SECTION critSect = {};
	bool terminateThread = false;

public:
	// Sequence number for actions
	DWORD dwSequence = 1;

	// Mouse-State for GetDeviceData() / GetDeviceState()
	DIMOUSESTATE mouseStateDeviceData = {};
	DIMOUSESTATE mouseStateDeviceDataGame = {};

	HANDLE mouseEventHandle = nullptr;

	CDirectInput8Globals()
	{
		InitializeCriticalSection(&critSect);

		if (!hidDllLoaded)
		{
			LoadHidLibrary();
		}

		hThread = CreateThread(nullptr, 0, ThreadProc, this, 0, &threadId);
		if (hThread == nullptr)
		{
			Logging::Log() << __FUNCTION__ << " Error: CreateThread() failed!";
			return;
		}
	}
	~CDirectInput8Globals()
	{
		// Signal the thread to terminate
		terminateThread = true;

		// Wait for the thread to terminate
		if (hThread != nullptr)
		{
			WaitForSingleObject(hThread, INFINITE);
			CloseHandle(hThread);
			hThread = nullptr;
		}

		// Close any other handles
		if (mouseEventHandle != nullptr)
		{
			CloseHandle(mouseEventHandle);
			mouseEventHandle = nullptr;
		}

		// Clean up critical section
		DeleteCriticalSection(&critSect);

		// Clear global instance
		diGlobalsInstance = nullptr;
	}

	void Lock()
	{
		EnterCriticalSection(&critSect);
	}

	void Unlock()
	{
		LeaveCriticalSection(&critSect);
	}

	void HandleRawInput(RAWINPUT* raw)
	{
		Lock();

		if (raw->header.dwType == RIM_TYPEMOUSE)
		{
			if ((raw->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: IsAbsoluteMouse - not handled yet!");
			}
			else
			{
				mouseStateDeviceData.lX += raw->data.mouse.lLastX;
				mouseStateDeviceData.lY += raw->data.mouse.lLastY;

				if (raw->data.mouse.usButtonFlags & RI_MOUSE_WHEEL)
				{
					short mouseWheelDelta = (short)raw->data.mouse.usButtonData;
					mouseStateDeviceData.lZ += mouseWheelDelta;
				}

				if (raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN)
				{
					mouseStateDeviceData.rgbButtons[0] = 0x80;
				}
				if (raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP)
				{
					mouseStateDeviceData.rgbButtons[0] = 0x00;
				}

				if (raw->data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN)
				{
					mouseStateDeviceData.rgbButtons[1] = 0x80;
				}
				if (raw->data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP)
				{
					mouseStateDeviceData.rgbButtons[1] = 0x00;
				}

				if (raw->data.mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN)
				{
					mouseStateDeviceData.rgbButtons[2] = 0x80;
				}
				if (raw->data.mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP)
				{
					mouseStateDeviceData.rgbButtons[2] = 0x00;
				}
			}

			if (this->mouseEventHandle)
			{
				ResetEvent(this->mouseEventHandle);
			}
		}

		Unlock();
	}

	static DWORD WINAPI ThreadProc(LPVOID lpParameter)
	{
		CDirectInput8Globals* pThis = reinterpret_cast<CDirectInput8Globals*>(lpParameter);

		WNDCLASSEXA wcex = {};
		wcex.cbSize = sizeof(WNDCLASSEXA);
		wcex.lpfnWndProc = MainWndProc;
		wcex.hInstance = hModule_dll;
		wcex.lpszClassName = "CDirectInput8";

		if (!RegisterClassExA(&wcex))
		{
			Logging::Log() << __FUNCTION__ << " Error: RegisterClassExA() failed!";
			pThis->hThread = 0;
			return 0;
		}

		HWND hWnd = CreateWindowExA(0, "CDirectInput8", "dxwrapper", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, hModule_dll, nullptr);
		if (hWnd == nullptr)
		{
			Logging::Log() << __FUNCTION__ << " Error: CreateWindowExA() failed!";
			pThis->hThread = 0;
			return 0;
		}

		// Log the window handle
		Logging::Log() << "Created window handle: " << hWnd;

		MSG msg = {};
		while (GetMessage(&msg, nullptr, 0, 0) > 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			// Check for termination signal
			if (pThis->terminateThread)
			{
				break;
			}

			if (diGlobalsInstance->mouseEventHandle)
			{
				SetEvent(diGlobalsInstance->mouseEventHandle);
			}
		}

		DestroyWindow(hWnd);
		UnregisterClassA("CDirectInput8", hModule_dll);

		pThis->hThread = 0;
		return msg.wParam;
	}

	static LRESULT HandleWMInput(LPARAM lParam)
	{
		UINT dwSize = 0;

		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER));
		LPBYTE lpb = new BYTE[dwSize];
		if (lpb == nullptr)
		{
			return 0;
		}

		if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
		{
			Logging::Log() << __FUNCTION__ << " Error: GetRawInputData does not return correct size!";

			delete[] lpb;
			return 0;
		}

		RAWINPUT* raw = (RAWINPUT*)lpb;

		diGlobalsInstance->HandleRawInput(raw);

		delete[] lpb;
		return 0;
	}

	static LRESULT CALLBACK MainWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
	{
		if (Msg == WM_CREATE)
		{
			RAWINPUTDEVICE Rid = {};

			Rid.usUsagePage = HID_USAGE_PAGE_GENERIC;
			Rid.usUsage = HID_USAGE_GENERIC_MOUSE;
			Rid.dwFlags = RIDEV_INPUTSINK | RIDEV_NOLEGACY;
			Rid.hwndTarget = hWnd;

			if (RegisterRawInputDevices(&Rid, 1, sizeof(Rid)) == FALSE)
			{
				DWORD error = GetLastError();
				Logging::Log() << __FUNCTION__ << " RegisterRawInputDevices() failed! Error: " << error << " " << hWnd;
			}
		}
		else if (Msg == WM_INPUT)
		{
			return HandleWMInput(lParam);
		}

		return DefWindowProcA(hWnd, Msg, wParam, lParam);
	}
};