#pragma once

class CDirectInput8Globals
{
private:
	DWORD threadId = 0;
	HANDLE hThread = 0;
	DWORD dikMapping[256] = {};
	const wchar_t* dikNames[256] = {};
	CRITICAL_SECTION critSect = {};

public:
	bool enableGamepadSupport = false;

	// Sequence number for keyboard actions
	DWORD dwSequence = 0;

	// Key-States received via WM_INPUT / Raw-Input
	BYTE keyStates[256] = {};

	// Key-States actually sent to the game
	BYTE gameKeyStates[256] = {};

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

		InitializeKeyMappings();
		InitializeKeyNames();
	}

	void InitializeKeyMappings()
	{
		ZeroMemory(dikMapping, sizeof(dikMapping));
		dikMapping[VK_LBUTTON] = 0xFF;
		dikMapping[VK_RBUTTON] = 0xFF;
		dikMapping[VK_MBUTTON] = 0xFF;
		dikMapping[VK_OEM_1] = 0xFF;
		dikMapping[VK_OEM_2] = 0xFF;

		dikMapping[VK_SHIFT] = DIK_LSHIFT;
		dikMapping[VK_CONTROL] = DIK_LCONTROL;
		dikMapping[VK_MENU] = DIK_LMENU;

		dikMapping[VK_OEM_PLUS] = DIK_ADD;
		dikMapping[VK_OEM_COMMA] = DIK_COMMA;
		dikMapping[VK_OEM_MINUS] = DIK_MINUS;
		dikMapping[VK_OEM_PERIOD] = DIK_PERIOD;

		dikMapping[VK_LMENU] = DIK_LMENU;
		dikMapping[VK_RMENU] = DIK_RMENU;
		dikMapping[VK_BACK] = DIK_BACK;
		dikMapping[VK_TAB] = DIK_TAB;
		dikMapping[VK_RETURN] = DIK_RETURN;
		dikMapping[VK_LSHIFT] = DIK_LSHIFT;
		dikMapping[VK_RSHIFT] = DIK_RSHIFT;
		dikMapping[VK_LCONTROL] = DIK_LCONTROL;
		dikMapping[VK_RCONTROL] = DIK_RCONTROL;
		dikMapping[VK_PAUSE] = DIK_PAUSE;
		dikMapping[VK_CAPITAL] = DIK_CAPITAL;
		dikMapping[VK_ESCAPE] = DIK_ESCAPE;
		dikMapping[VK_SPACE] = DIK_SPACE;
		dikMapping[VK_PRIOR] = DIK_PRIOR;
		dikMapping[VK_NEXT] = DIK_NEXT;
		dikMapping[VK_END] = DIK_END;
		dikMapping[VK_HOME] = DIK_HOME;
		dikMapping[VK_LEFT] = DIK_LEFT;
		dikMapping[VK_UP] = DIK_UP;
		dikMapping[VK_RIGHT] = DIK_RIGHT;
		dikMapping[VK_DOWN] = DIK_DOWN;
		dikMapping[VK_INSERT] = DIK_INSERT;
		dikMapping[VK_DELETE] = DIK_DELETE;
		dikMapping[0x30] = DIK_0;
		dikMapping[0x31] = DIK_1;
		dikMapping[0x32] = DIK_2;
		dikMapping[0x33] = DIK_3;
		dikMapping[0x34] = DIK_4;
		dikMapping[0x35] = DIK_5;
		dikMapping[0x36] = DIK_6;
		dikMapping[0x37] = DIK_7;
		dikMapping[0x38] = DIK_8;
		dikMapping[0x39] = DIK_9;

		dikMapping[0x41] = DIK_A;
		dikMapping[0x42] = DIK_B;
		dikMapping[0x43] = DIK_C;
		dikMapping[0x44] = DIK_D;
		dikMapping[0x45] = DIK_E;
		dikMapping[0x46] = DIK_F;
		dikMapping[0x47] = DIK_G;
		dikMapping[0x48] = DIK_H;
		dikMapping[0x49] = DIK_I;
		dikMapping[0x4A] = DIK_J;
		dikMapping[0x4B] = DIK_K;
		dikMapping[0x4C] = DIK_L;
		dikMapping[0x4D] = DIK_M;
		dikMapping[0x4E] = DIK_N;
		dikMapping[0x4F] = DIK_O;
		dikMapping[0x50] = DIK_P;
		dikMapping[0x51] = DIK_Q;
		dikMapping[0x52] = DIK_R;
		dikMapping[0x53] = DIK_S;
		dikMapping[0x54] = DIK_T;
		dikMapping[0x55] = DIK_U;
		dikMapping[0x56] = DIK_V;
		dikMapping[0x57] = DIK_W;
		dikMapping[0x58] = DIK_X;
		dikMapping[0x59] = DIK_Y;
		dikMapping[0x5A] = DIK_Z;

		dikMapping[VK_LWIN] = DIK_LWIN;
		dikMapping[VK_RWIN] = DIK_RWIN;

		dikMapping[VK_NUMPAD0] = DIK_NUMPAD0;
		dikMapping[VK_NUMPAD1] = DIK_NUMPAD1;
		dikMapping[VK_NUMPAD2] = DIK_NUMPAD2;
		dikMapping[VK_NUMPAD3] = DIK_NUMPAD3;
		dikMapping[VK_NUMPAD4] = DIK_NUMPAD4;
		dikMapping[VK_NUMPAD5] = DIK_NUMPAD5;
		dikMapping[VK_NUMPAD6] = DIK_NUMPAD6;
		dikMapping[VK_NUMPAD7] = DIK_NUMPAD7;
		dikMapping[VK_NUMPAD8] = DIK_NUMPAD8;
		dikMapping[VK_NUMPAD9] = DIK_NUMPAD9;

		dikMapping[VK_MULTIPLY] = DIK_MULTIPLY;
		dikMapping[VK_ADD] = DIK_ADD;
		dikMapping[VK_SUBTRACT] = DIK_SUBTRACT;
		dikMapping[VK_DECIMAL] = DIK_DECIMAL;
		dikMapping[VK_DIVIDE] = DIK_DIVIDE;

		dikMapping[VK_F1] = DIK_F1;
		dikMapping[VK_F2] = DIK_F2;
		dikMapping[VK_F3] = DIK_F3;
		dikMapping[VK_F4] = DIK_F4;
		dikMapping[VK_F5] = DIK_F5;
		dikMapping[VK_F6] = DIK_F6;
		dikMapping[VK_F7] = DIK_F7;
		dikMapping[VK_F8] = DIK_F8;
		dikMapping[VK_F9] = DIK_F9;
		dikMapping[VK_F10] = DIK_F10;
		dikMapping[VK_F11] = DIK_F11;
		dikMapping[VK_F12] = DIK_F12;

		dikMapping[VK_NUMLOCK] = DIK_NUMLOCK;
	}

	void InitializeKeyNames()
	{
		ZeroMemory(dikNames, sizeof(dikNames));

		dikNames[DIK_ADD] = L"+";
		dikNames[DIK_COMMA] = L",";
		dikNames[DIK_MINUS] = L"-";
		dikNames[DIK_PERIOD] = L".";

		dikNames[DIK_LMENU] = L"Left-Alt";
		dikNames[DIK_RMENU] = L"Right-Alt";
		dikNames[DIK_BACK] = L"Back";
		dikNames[DIK_TAB] = L"Tab";
		dikNames[DIK_RETURN] = L"Return";
		dikNames[DIK_LSHIFT] = L"Left-Shift";
		dikNames[DIK_RSHIFT] = L"Right-Shift";
		dikNames[DIK_LCONTROL] = L"Left-Control";
		dikNames[DIK_RCONTROL] = L"Right-Control";
		dikNames[DIK_PAUSE] = L"Pause";
		dikNames[DIK_CAPITAL] = L"Caps-Lock";
		dikNames[DIK_ESCAPE] = L"Escape";
		dikNames[DIK_SPACE] = L"Space";
		dikNames[DIK_PRIOR] = L"Prior";
		dikNames[DIK_NEXT] = L"Next";
		dikNames[DIK_END] = L"End";
		dikNames[DIK_HOME] = L"Home";
		dikNames[DIK_LEFT] = L"Left";
		dikNames[DIK_UP] = L"Up";
		dikNames[DIK_RIGHT] = L"Right";
		dikNames[DIK_DOWN] = L"Down";
		dikNames[DIK_INSERT] = L"Insert";
		dikNames[DIK_DELETE] = L"Delete";
		dikNames[DIK_0] = L"0";
		dikNames[DIK_1] = L"1";
		dikNames[DIK_2] = L"2";
		dikNames[DIK_3] = L"3";
		dikNames[DIK_4] = L"4";
		dikNames[DIK_5] = L"5";
		dikNames[DIK_6] = L"6";
		dikNames[DIK_7] = L"7";
		dikNames[DIK_8] = L"8";
		dikNames[DIK_9] = L"9";

		dikNames[DIK_A] = L"A";
		dikNames[DIK_B] = L"B";
		dikNames[DIK_C] = L"C";
		dikNames[DIK_D] = L"D";
		dikNames[DIK_E] = L"E";
		dikNames[DIK_F] = L"F";
		dikNames[DIK_G] = L"G";
		dikNames[DIK_H] = L"H";
		dikNames[DIK_I] = L"I";
		dikNames[DIK_J] = L"J";
		dikNames[DIK_K] = L"K";
		dikNames[DIK_L] = L"L";
		dikNames[DIK_M] = L"M";
		dikNames[DIK_N] = L"N";
		dikNames[DIK_O] = L"O";
		dikNames[DIK_P] = L"P";
		dikNames[DIK_Q] = L"Q";
		dikNames[DIK_R] = L"R";
		dikNames[DIK_S] = L"S";
		dikNames[DIK_T] = L"T";
		dikNames[DIK_U] = L"U";
		dikNames[DIK_V] = L"V";
		dikNames[DIK_W] = L"W";
		dikNames[DIK_X] = L"X";
		dikNames[DIK_Y] = L"Y";
		dikNames[DIK_Z] = L"Z";

		dikNames[DIK_LWIN] = L"Left-Win";
		dikNames[DIK_RWIN] = L"Right-Win";

		dikNames[DIK_NUMPAD0] = L"Numpad-0";
		dikNames[DIK_NUMPAD1] = L"Numpad-1";
		dikNames[DIK_NUMPAD2] = L"Numpad-2";
		dikNames[DIK_NUMPAD3] = L"Numpad-3";
		dikNames[DIK_NUMPAD4] = L"Numpad-4";
		dikNames[DIK_NUMPAD5] = L"Numpad-5";
		dikNames[DIK_NUMPAD6] = L"Numpad-6";
		dikNames[DIK_NUMPAD7] = L"Numpad-7";
		dikNames[DIK_NUMPAD8] = L"Numpad-8";
		dikNames[DIK_NUMPAD9] = L"Numpad-9";

		dikNames[DIK_MULTIPLY] = L"*";
		dikNames[DIK_ADD] = L"+";
		dikNames[DIK_SUBTRACT] = L"-";
		dikNames[DIK_DECIMAL] = L".";
		dikNames[DIK_DIVIDE] = L"/";

		dikNames[DIK_F1] = L"F1";
		dikNames[DIK_F2] = L"F2";
		dikNames[DIK_F3] = L"F3";
		dikNames[DIK_F4] = L"F4";
		dikNames[DIK_F5] = L"F5";
		dikNames[DIK_F6] = L"F6";
		dikNames[DIK_F7] = L"F7";
		dikNames[DIK_F8] = L"F8";
		dikNames[DIK_F9] = L"F9";
		dikNames[DIK_F10] = L"F10";
		dikNames[DIK_F11] = L"F11";
		dikNames[DIK_F12] = L"F12";

		dikNames[DIK_NUMLOCK] = L"Num-Lock";
	}

	void GetKeyNameW(DWORD dik, wchar_t* keyName)
	{
		const wchar_t* keyNameMapped = dikNames[dik];

		if (keyNameMapped)
		{
			lstrcpyW(keyName, keyNameMapped);
		}
		else
		{
			wsprintfW(keyName, L"Unknown key 0x%x", dik);
		}
	}

	void GetKeyNameA(DWORD dik, char* keyName)
	{
		wchar_t keyNameW[4096];

		GetKeyNameW(dik, keyNameW);
		ToAnsi(keyNameW, keyName);
	}

	void ToAnsi(wchar_t* inputString, char* outputString)
	{
		WideCharToMultiByte(CP_ACP, 0, inputString, -1, outputString, 4096, NULL, NULL);
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
				//registration failed. Call GetLastError for the cause of the error
				Logging::Log() << __FUNCTION__ << " Error: RegisterRawInputDevices() failed!";
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