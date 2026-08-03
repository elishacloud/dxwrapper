#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <atomic>

#define WM_APP_CREATE_D3D9_DEVICE      (WM_APP + 0xFFF - 0x1)
#define WM_APP_RESET_D3D9_DEVICE       (WM_APP + 0xFFF - 0x2)
#define WM_APP_SET_KEYBOARD_LAYOUT     (WM_APP + 0xFFF - 0x3)
#define WM_APP_DISABLE_KEYBOARD_LAYOUT (WM_APP + 0xFFF - 0x4)

#define WM_MAKE_KEY(Val1, Val2) \
	(LPARAM)WndProc::MakeKey((DWORD)Val1, (DWORD)Val2)

namespace WndProc
{
	struct DATASTRUCT {
		std::atomic<DWORD> DirectXVersion = 0;
		std::atomic<bool> IsDirectDraw = false;
		std::atomic<bool> IsDirect3D9 = false;
		std::atomic<bool> IsCreatingDevice = false;
		std::atomic<bool> IsExclusiveMode = false;
		std::atomic<bool> NoWindowChanges = false;
		std::atomic<bool> InSizeMove = false;
		std::atomic<WPARAM> IsWindowActive = UINT32_MAX;
		std::atomic<BOOL> IsWindowIconic = UINT32_MAX;
		std::atomic<int> DeviceCounter = 0;
		std::atomic<LONG> ClipWidth = 0;
		std::atomic<LONG> ClipHeight = 0;
	};

	DWORD MakeKey(DWORD Val1, DWORD Val2);

	struct ScopedSetDeviceCreationFlag
	{
	private:
		inline static int Counter = 0;

		const bool enable;
		DATASTRUCT* pDataStruct;
	public:
		// Constructor sets the flag to true
		ScopedSetDeviceCreationFlag(DATASTRUCT* pDataStruct, bool activate = true) : pDataStruct(pDataStruct), enable(activate && pDataStruct != nullptr)
		{
			if (enable)
			{
				if (++Counter == 1)
				{
					pDataStruct->IsCreatingDevice = true;
				}
			}
		}
		// Destructor sets the flag back to false
		~ScopedSetDeviceCreationFlag()
		{
			if (enable)
			{
				if (--Counter == 0)
				{
					pDataStruct->IsCreatingDevice = false;
				}
			}
		}

		ScopedSetDeviceCreationFlag(const ScopedSetDeviceCreationFlag&) = delete;
		ScopedSetDeviceCreationFlag& operator=(const ScopedSetDeviceCreationFlag&) = delete;
	};

	extern bool SwitchingResolution;

	bool ShouldHook(HWND hWnd);
	DATASTRUCT* AddWndProc(HWND hWnd);
	void RemoveInactiveWndProcs();
	DATASTRUCT* GetWndProctStruct(HWND hWnd);
	void DisableForcedKeyboardLayout();
}
