#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <atomic>

#define WM_APP_CREATE_D3D9_DEVICE      (WM_APP + 0xFFF - 0x123)
#define WM_APP_END_CREATING_DEVICE     (WM_APP + 0xFFF - 0x124)
#define WM_APP_SET_KEYBOARD_LAYOUT     (WM_APP + 0xFFF - 0x125)
#define WM_APP_DISABLE_KEYBOARD_LAYOUT (WM_APP + 0xFFF - 0x126)

#define WM_MAKE_KEY(Val1, Val2) \
	(LPARAM)WndProc::MakeKey((DWORD)Val1, (DWORD)Val2)

namespace WndProc
{
	struct DATASTRUCT {
		std::atomic<DWORD> DirectXVersion = 0;
		std::atomic<bool> IsDirectDraw = false;
		std::atomic<bool> IsDirect3D9 = false;
		std::atomic<bool> IsExclusiveMode = false;
		std::atomic<bool> NoWindowChanges = false;
		std::atomic<bool> InSizeMove = false;
		std::atomic<WORD> IsWindowActive = 0xFFFF;
		std::atomic<WORD> IsWindowIconic = 0xFFFF;
		std::atomic<int> ActiveDeviceCounter = 0;
		std::atomic<int> PendingDeviceCounter = 0;
		std::atomic<LONG> ClipWidth = 0;
		std::atomic<LONG> ClipHeight = 0;
	};

	DWORD MakeKey(DWORD Val1, DWORD Val2);

	struct ScopedSetDeviceCreationFlag
	{
	private:
		const HWND hWnd;
		const bool enable;
		DATASTRUCT* pDataStruct;
	public:
		// Constructor sets the flag to true
		ScopedSetDeviceCreationFlag(DATASTRUCT* pDataStruct, HWND hWnd, bool activate = true) : pDataStruct(pDataStruct), hWnd(hWnd), enable(activate && pDataStruct != nullptr)
		{
			if (enable)
			{
				pDataStruct->ActiveDeviceCounter++;
			}
		}
		// Destructor sets the flag back to false
		~ScopedSetDeviceCreationFlag()
		{
			if (enable)
			{
				pDataStruct->PendingDeviceCounter++;
				BOOL result = PostMessage(hWnd, WM_APP_END_CREATING_DEVICE, (WPARAM)hWnd, WM_MAKE_KEY(hWnd, hWnd));
				pDataStruct->ActiveDeviceCounter--;

				// If PostMessage fails than decrement pending device counter back down
				if (result == FALSE)
				{
					pDataStruct->PendingDeviceCounter--;
				}
			}
		}

		ScopedSetDeviceCreationFlag(const ScopedSetDeviceCreationFlag&) = delete;
		ScopedSetDeviceCreationFlag& operator=(const ScopedSetDeviceCreationFlag&) = delete;
	};

	extern bool SwitchingResolution;

	bool ShouldHook(HWND hWnd);
	DATASTRUCT* AddWndProc(HWND hWnd);
	void RemoveWndProc(HWND hWnd);
	DATASTRUCT* GetWndProctStruct(HWND hWnd);
	WNDPROC CheckWndProc(HWND hWnd, LONG dwNewLong);
	void DisableForcedKeyboardLayout();
}
