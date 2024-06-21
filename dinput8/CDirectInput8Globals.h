#pragma once

#include "hid.h"
#include "Dllmain\Dllmain.h"

class CDirectInput8Globals
{
private:
	DWORD threadId = 0;
	HANDLE hThread = 0;
	CRITICAL_SECTION critSect = {};

public:
	// Sequence number for keyboard actions
	DWORD dwSequence = 0;

	// Mouse-State for GetDeviceData() / GetDeviceState()
	DIMOUSESTATE* mouseStateDeviceData = new DIMOUSESTATE();
	DIMOUSESTATE* mouseStateDeviceDataGame = new DIMOUSESTATE();

	HANDLE mouseEventHandle = nullptr;

	CDirectInput8Globals()
	{
		InitializeCriticalSection(&critSect);

		hThread = CreateThread(NULL, 0, ThreadProc, NULL, 0, &threadId);
		if (hThread == NULL)
		{
			Logging::Log() << __FUNCTION__ << " Error: CreateThread() failed!";
			return;
		}

		dwSequence = 1;
	}

	void Lock()
	{
		EnterCriticalSection(&critSect);
	}

	void Unlock()
	{
		LeaveCriticalSection(&critSect);
	}

	void CheckRawInputDevices()
	{
		// TODO: Check for new rawinput devices
	}

	void HandleRawInput(RAWINPUT* raw)
	{
		Lock();
		{
			if (raw->header.dwType == RIM_TYPEMOUSE)
			{
				if ((raw->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Error: IsAbsoluteMouse - not handled yet!");
				}
				else
				{
					mouseStateDeviceData->lX += raw->data.mouse.lLastX;
					mouseStateDeviceData->lY += raw->data.mouse.lLastY;

					if (raw->data.mouse.usButtonFlags & RI_MOUSE_WHEEL)
					{
						short mouseWheelDelta = (short)raw->data.mouse.usButtonData;
						mouseStateDeviceData->lZ += mouseWheelDelta;
					}

					if (raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN)
					{
						mouseStateDeviceData->rgbButtons[0] = 0x80;
					}
					if (raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP)
					{
						mouseStateDeviceData->rgbButtons[0] = 0x00;
					}

					if (raw->data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN)
					{
						mouseStateDeviceData->rgbButtons[1] = 0x80;
					}
					if (raw->data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP)
					{
						mouseStateDeviceData->rgbButtons[1] = 0x00;
					}

					if (raw->data.mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN)
					{
						mouseStateDeviceData->rgbButtons[2] = 0x80;
					}
					if (raw->data.mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP)
					{
						mouseStateDeviceData->rgbButtons[2] = 0x00;
					}
				}

				if (this->mouseEventHandle)
				{
					ResetEvent(this->mouseEventHandle);
				}
			}
		}
		Unlock();
	}

	static DWORD WINAPI ThreadProc(LPVOID lpParameter)
	{
		UNREFERENCED_PARAMETER(lpParameter);

		WNDCLASSEXA wcex;
		ZeroMemory(&wcex, sizeof(WNDCLASSEXA));
		wcex.cbSize = sizeof(WNDCLASSEXA);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = MainWndProc;
		wcex.hInstance = hModule_dll;
		wcex.hIcon = LoadIconA(NULL, IDI_APPLICATION);
		wcex.hCursor = LoadCursorA(NULL, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		wcex.lpszClassName = "CDirectInput8";

		if (!RegisterClassExA(&wcex))
		{
			Logging::Log() << __FUNCTION__ << " Error: RegisterClassExA() failed!";
			return 0;
		}

		HWND hWnd = CreateWindowExA(WS_EX_CLIENTEDGE, "CDirectInput8", "dinput8.dll", WS_OVERLAPPEDWINDOW, 1920, 10, 400, 400, NULL, NULL, hModule_dll, NULL);
		if (hWnd == NULL)
		{
			Logging::Log() << __FUNCTION__ << " Error: CreateWindowExA() failed!";
			return 0;
		}

		ShowWindow(hWnd, SW_HIDE);
		UpdateWindow(hWnd);

		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0) > 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (diGlobalsInstance->mouseEventHandle)
			{
				SetEvent(diGlobalsInstance->mouseEventHandle);
			}
		}

		return msg.wParam;
	}

	static LRESULT HandleWMInput(LPARAM lParam)
	{
		UINT dwSize = 0;

		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
		LPBYTE lpb = new BYTE[dwSize];
		if (lpb == NULL)
		{
			return 0;
		}

		if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: GetRawInputData does not return correct size!");

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
			RAWINPUTDEVICE Rid[1] = {};

			Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
			Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
			Rid[0].dwFlags = 0; // Can use RIDEV_NOLEGACY, etc. if needed
			Rid[0].hwndTarget = hWnd;

			if (RegisterRawInputDevices(Rid, 1, sizeof(Rid[0])) == FALSE)
			{
				DWORD error = GetLastError();
				Logging::Log() << __FUNCTION__ << " Error: RegisterRawInputDevices() failed! Error: " << error << " " << hWnd;
			}
		}
		else if (Msg == WM_INPUT)
		{
			return HandleWMInput(lParam);
		}
		else if (Msg == WM_INPUT_DEVICE_CHANGE)
		{
			diGlobalsInstance->CheckRawInputDevices();
		}

		return DefWindowProcA(hWnd, Msg, wParam, lParam);
	}
};