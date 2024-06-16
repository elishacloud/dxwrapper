#pragma once

class CDirectInput8
{
private:
	DWORD threadId = 0;
	HANDLE hThread = 0;

public:
	CDirectInput8()
	{
		hThread = CreateThread(NULL, 0, ThreadProc, NULL, 0, &threadId);
		if (hThread == NULL)
		{
			MessageBoxA(NULL, "CreateThread() failed", "dinput8.dll", MB_OK);
			return;
		}
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
			MessageBoxA(NULL, "RegisterClassExA() failed", "dinput8.dll", MB_OK);
			return 0;
		}

		HWND hWnd = CreateWindowExA(WS_EX_CLIENTEDGE, "CDirectInput8", "dinput8.dll", WS_OVERLAPPEDWINDOW, 1920, 10, 400, 400, NULL, NULL, hModule_dll, NULL);
		if (hWnd == NULL)
		{
			MessageBoxA(NULL, "CreateWindowExA() failed", "dinput8.dll", MB_OK);
			return 0;
		}

		ShowWindow(hWnd, SW_HIDE);
		UpdateWindow(hWnd);

		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0) > 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
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
			diGlobalsInstance->LogA("GetRawInputData does not return correct size!", __FILE__, __LINE__);
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
			RAWINPUTDEVICE Rid[4] = { 0 };
			int ridLength = 2;

			//
			Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
			Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
			Rid[0].dwFlags = 0;
			Rid[0].hwndTarget = hWnd;

			Rid[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
			Rid[1].usUsage = HID_USAGE_GENERIC_KEYBOARD;
			Rid[1].dwFlags = 0;
			Rid[1].hwndTarget = hWnd;

			if (diGlobalsInstance->enableGamepadSupport)
			{
				ridLength = 4;

				Rid[2].usUsagePage = HID_USAGE_PAGE_GENERIC;
				Rid[2].usUsage = HID_USAGE_GENERIC_JOYSTICK;
				Rid[2].dwFlags = RIDEV_DEVNOTIFY;
				Rid[2].hwndTarget = hWnd;

				Rid[3].usUsagePage = HID_USAGE_PAGE_GENERIC;
				Rid[3].usUsage = HID_USAGE_GENERIC_GAMEPAD;
				Rid[3].dwFlags = RIDEV_DEVNOTIFY;
				Rid[3].hwndTarget = hWnd;
			}

			if (RegisterRawInputDevices(Rid, ridLength, sizeof(Rid[0])) == FALSE)
			{
				MessageBoxA(NULL, "RegisterRawInputDevices() failed!", "dinput8.dll", MB_OK | MB_ICONEXCLAMATION);
				//registration failed. Call GetLastError for the cause of the error
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