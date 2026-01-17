/**
* Copyright (C) 2025 Elisha Riedlinger
*
* This software is  provided 'as-is', without any express  or implied  warranty. In no event will the
* authors be held liable for any damages arising from the use of this software.
* Permission  is granted  to anyone  to use  this software  for  any  purpose,  including  commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*   1. The origin of this software must not be misrepresented; you must not claim that you  wrote the
*      original  software. If you use this  software  in a product, an  acknowledgment in the product
*      documentation would be appreciated but is not required.
*   2. Altered source versions must  be plainly  marked as such, and  must not be  misrepresented  as
*      being the original software.
*   3. This notice may not be removed or altered from any source distribution.
*/

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <algorithm>
#include "WndProc.h"
#include "GDI.h"
#include "ddraw\ddraw.h"
#include "d3d9\d3d9External.h"
#include "Utils\Utils.h"
#include "Settings\Settings.h"
#include "Logging\Logging.h"

namespace WndProc
{
	struct WNDPROCSTRUCT;

	LRESULT CALLBACK Handler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, WNDPROCSTRUCT* AppWndProcInstance);
	WNDPROC GetWndProc(HWND hWnd);
	LONG SetWndProc(HWND hWnd, WNDPROC ProcAddress);
	LRESULT CallWndProc(WNDPROC lpPrevWndFunc, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	bool IsExecutableAddress(void* address);

	bool SwitchingResolution = false;

	std::atomic<bool> IsKeyboardActive = false;
	void SetKeyboardLayoutFocus(HWND hWnd, bool IsActivating);

	struct WNDPROCSTRUCT
	{
	private:
		BYTE FunctCode[38] = {
			/* LRESULT CALLBACK MyWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
			*  {
			*      WndProc(hwnd, msg, wParam, lParam, this);
			*  } */
			0x55,								// PUSH EBP
			0x8B, 0xEC,							// MOV EBP,ESP
			0x51,								// PUSH ECX
			0x68, 0x00,0x00,0x00,0x00,			// PUSH DWORD 0x00000000[this]
			0xFF,0x75, 0x14,					// PUSH DWORD PTR SS:[EBP+14]
			0xFF,0x75, 0x10,					// PUSH DWORD PTR SS:[EBP+10]
			0xFF,0x75, 0x0C,					// PUSH DWORD PTR SS:[EBP+0C]
			0xFF,0x75, 0x08,					// PUSH DWORD PTR SS:[EBP+08]
			0xE8, 0x00,0x00,0x00,0x00,			// CALL dxwrapper.WndProc
			0x89,0x45, 0xFC,					// MOV DWORD PTR SS:[EBP-4],EAX
			0x8B,0x45, 0xFC,					// MOV EAX,DWORD PTR SS:[EBP-4]
			0x8B,0xE5,							// MOV ESP,EBP
			0x5D,								// POP EBP
			0xC2, 0x10,0x00						// RETN
		};
		LONG* pFunctVar = (LONG*)&FunctCode[5];
		int* pFunctCall = (int*)&FunctCode[22];
		HWND hWnd = nullptr;
		WNDPROC MyWndProc = nullptr;
		WNDPROC AppWndProc = nullptr;
		DATASTRUCT DataStruct;	// Use initialization from struct
		bool Active = true;
		bool Exiting = false;
	public:
		WNDPROCSTRUCT(HWND p_hWnd, WNDPROC p_AppWndProc) : hWnd(p_hWnd), AppWndProc(p_AppWndProc)
		{
			DWORD oldProtect = 0;
			if (VirtualProtect(FunctCode, sizeof(FunctCode), PAGE_EXECUTE_READWRITE, &oldProtect))
			{
				// CFG registration
				Utils::MarkAsValidCallTarget(FunctCode, sizeof(FunctCode), 0);

				// Patch code
				*pFunctVar = (LONG)this;
				*pFunctCall = (int)&Handler - ((int)pFunctCall + 4);

				// Set function pointer
				MyWndProc = reinterpret_cast<WNDPROC>((LONG)FunctCode);
			}
		}
		~WNDPROCSTRUCT()
		{
			Exiting = true;
			Active = false;
			if (Config.Exiting)
			{
				AppWndProc = (IsWindowUnicode(hWnd) ? DefWindowProcW : DefWindowProcA);
				(IsWindowUnicode(hWnd) ?
					SetWindowLongW(hWnd, GWL_WNDPROC, (LONG)DefWindowProcW) :
					SetWindowLongA(hWnd, GWL_WNDPROC, (LONG)DefWindowProcA));
				return;
			}
			// Restore WndProc
			if (IsWindow(hWnd) && AppWndProc)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Deleting WndProc instance! " << hWnd);
				SetWndProc(hWnd, AppWndProc);
			}
		}
		HWND GetHWnd() const { return hWnd; }
		WNDPROC GetMyWndProc() const { return MyWndProc; }
		WNDPROC GetAppWndProc() const { return AppWndProc; }
		DATASTRUCT* GetDataStruct() { return &DataStruct; }
		bool IsActive() const { return Active; }
		void SetInactive() { Active = false; }
		bool IsExiting() const { return Exiting; }
	};

	std::vector<std::shared_ptr<WNDPROCSTRUCT>> WndProcList;
}

bool WndProc::IsExecutableAddress(void* address)
{
	if (!address)
	{
		return false;
	}

	MEMORY_BASIC_INFORMATION mbi = {};
	SIZE_T result = VirtualQuery(address, &mbi, sizeof(mbi));
	if (result == 0)
	{
		return false;
	}

	return (mbi.State == MEM_COMMIT) &&
		(mbi.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY));
}

WNDPROC WndProc::CheckWndProc(HWND hWnd, LONG dwNewLong)
{
	for (auto& entry : WndProcList)
	{
		if (entry->IsActive() && entry->GetHWnd() == hWnd && !(entry->IsExiting() && (LONG)entry->GetAppWndProc() == dwNewLong))
		{
			return entry->GetMyWndProc();
		}
	}
	return nullptr;
}

WNDPROC WndProc::GetWndProc(HWND hWnd)
{
	return reinterpret_cast<WNDPROC>(IsWindowUnicode(hWnd) ?
		GetWindowLongW(hWnd, GWL_WNDPROC) :
		GetWindowLongA(hWnd, GWL_WNDPROC));
}

LONG WndProc::SetWndProc(HWND hWnd, WNDPROC ProcAddress)
{
	return (IsWindowUnicode(hWnd) ?
		SetWindowLongW(hWnd, GWL_WNDPROC, (LONG)ProcAddress) :
		SetWindowLongA(hWnd, GWL_WNDPROC, (LONG)ProcAddress));
}

LRESULT WndProc::CallWndProc(WNDPROC lpPrevWndFunc, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	return (IsWindowUnicode(hWnd) ?
		(lpPrevWndFunc ?
			CallWindowProcW(lpPrevWndFunc, hWnd, Msg, wParam, lParam) :
			DefWindowProcW(hWnd, Msg, wParam, lParam)) :
		(lpPrevWndFunc ?
			CallWindowProcA(lpPrevWndFunc, hWnd, Msg, wParam, lParam) :
			DefWindowProcA(hWnd, Msg, wParam, lParam)));
}

bool WndProc::ShouldHook(HWND hWnd)
{
	if (!IsWindow(hWnd))
		return false;

	// Must be top-level (child windows never receive WM_ACTIVATE)
	if (GetParent(hWnd) != NULL)
		return false;

	// Message-only windows never activate
	if (hWnd == HWND_MESSAGE)
		return false;

	return true;
}

WndProc::DATASTRUCT* WndProc::AddWndProc(HWND hWnd)
{
	// Validate window handle
	if (!IsWindow(hWnd))
	{
		return nullptr;
	}

	// Remove inactive elements
	WndProcList.erase(
		std::remove_if(WndProcList.begin(), WndProcList.end(),
			[](const std::shared_ptr<WNDPROCSTRUCT>& wndProc) {
				return !wndProc->IsActive() && !IsWindow(wndProc->GetHWnd());
			}),
		WndProcList.end());

	// Check if window is already hooked
	for (auto& entry : WndProcList)
	{
		if (entry->IsActive() && entry->GetHWnd() == hWnd)
		{
			return entry->GetDataStruct();
		}
	}

	// Get WndProc from hWnd
	WNDPROC NewAppWndProc = GetWndProc(hWnd);

	// Create new struct
	WNDPROCSTRUCT* raw = static_cast<WNDPROCSTRUCT*>(
		VirtualAlloc(nullptr, sizeof(WNDPROCSTRUCT), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE)
		);

	if (!raw)
	{
		return nullptr;
	}

	// Construct the object in-place
	new (raw) WNDPROCSTRUCT(hWnd, NewAppWndProc);

	auto deleter = [](WNDPROCSTRUCT* p)
		{
			if (!p) return;

			// Explicit destructor call
			p->~WNDPROCSTRUCT();

			// Free executable memory
			VirtualFree(p, 0, MEM_RELEASE);
		};

	std::shared_ptr<WNDPROCSTRUCT> NewEntry(raw, deleter);

	// Get new WndProc
	WNDPROC NewWndProc = NewEntry->GetMyWndProc();
	if (!NewWndProc)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: could not get function target!");
		return nullptr;
	}

	// Set new window pointer and store struct address
	LOG_LIMIT(100, __FUNCTION__ << " Creating WndProc instance! " << hWnd);
	SetWndProc(hWnd, NewWndProc);
	WndProcList.push_back(NewEntry);

	// Handle keyboard layout
	if (Config.ForceKeyboardLayout)
	{
		if (hWnd == GetActiveWindow() || hWnd == GetFocus())
		{
			PostMessage(hWnd, WM_APP_SET_KEYBOARD_LAYOUT, (WPARAM)NewWndProc, WM_MAKE_KEY(hWnd, NewWndProc));
		}
	}

	// Return
	return NewEntry->GetDataStruct();
}

void WndProc::RemoveWndProc(HWND hWnd)
{
	// Remove instances from the vector
	auto newEnd = std::remove_if(WndProcList.begin(), WndProcList.end(), [hWnd](const std::shared_ptr<WNDPROCSTRUCT>& AppWndProcInstance) -> bool
		{
			return (AppWndProcInstance->GetHWnd() == hWnd);
		});

	// Erase removed instances from the vector
	WndProcList.erase(newEnd, WndProcList.end());
}

WndProc::DATASTRUCT* WndProc::GetWndProctStruct(HWND hWnd)
{
	for (auto& entry : WndProcList)
	{
		if (entry->IsActive() && entry->GetHWnd() == hWnd)
		{
			return entry->GetDataStruct();
		}
	}
	return nullptr;
}

DWORD WndProc::MakeKey(DWORD Val1, DWORD Val2)
{
	DWORD Result = 0;
	for (DWORD Val : { Val1, Val2 } )
	{
		for (int x = 1; x < 8; x++)
		{
			Val = Val ^ (Val << 20);
			Val = Val ^ (Val >> 12);
			Val = (Val << 15) + (Val >> 17);
		}
		Result += Val;
	}
	return Result % 8 ? Result ^ 0xAAAAAAAA : Result ^ 0x55555555;
}

void WndProc::SetKeyboardLayoutFocus(HWND hWnd, bool IsActivating)
{
	// On Activation
	if (IsActivating)
	{
		PostMessage(hWnd, WM_APP_SET_KEYBOARD_LAYOUT, (WPARAM)hWnd, WM_MAKE_KEY(hWnd, hWnd));
	}
	// On Deactivation
	else if (IsKeyboardActive)
	{
		IsKeyboardActive = false;
		KeyboardLayout::UnSetForcedKeyboardLayout();
	}
}

LRESULT CALLBACK WndProc::Handler(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, WNDPROCSTRUCT* AppWndProcInstance)
{
	if (Msg != WM_PAINT)
	{
		Logging::LogDebug() << __FUNCTION__ << " " << hWnd << " " << Logging::hex(Msg) << " " << wParam << " " << lParam << " IsIconic: " << IsIconic(hWnd);
	}

	if (!AppWndProcInstance || !hWnd)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: invalid pointer!");
		return NULL;
	}

	const WNDPROC pWndProc = AppWndProcInstance->GetAppWndProc();
	const HWND hWndInstance = AppWndProcInstance->GetHWnd();
	DATASTRUCT* pDataStruct = AppWndProcInstance->GetDataStruct();

	if (hWnd != hWndInstance)
	{
		return CallWndProc(pWndProc, hWnd, Msg, wParam, lParam);
	}

	const bool IsForcingWindowedMode = (Config.EnableWindowMode && pDataStruct->IsExclusiveMode);

	switch (Msg)
	{
	case WM_APP_CREATE_D3D9_DEVICE:
		// Handle Direct3D9 device creation
		if (WM_MAKE_KEY(hWnd, wParam) == lParam)
		{
			if (m_IDirectDrawX::CheckDirectDrawXInterface((void*)wParam))
			{
				((m_IDirectDrawX*)wParam)->CreateD9Device(__FUNCTION__);
			}
			return NULL;
		}
		break;

	case WM_APP_SET_KEYBOARD_LAYOUT:
		if (WM_MAKE_KEY(hWnd, wParam) == lParam)
		{
			if (Config.ForceKeyboardLayout)
			{
				IsKeyboardActive = true;
				KeyboardLayout::SetForcedKeyboardLayout();
			}
			return NULL;
		}
		break;

	case WM_ACTIVATEAPP:
		// Handle keyboard layout
		if (Config.ForceKeyboardLayout && hWnd == hWndInstance)
		{
			SetKeyboardLayoutFocus(hWnd, wParam != FALSE);
		}
		// Some games don't properly handle app activate in exclusive mode
		if (pDataStruct->IsDirectDraw)
		{
			static WPARAM IsActive = 0xFFFF;
			Logging::Log() << __FUNCTION__ << " WM_ACTIVATEAPP IsExclusiveMode: " << pDataStruct->IsExclusiveMode << " IsCreatingDevice: " << pDataStruct->IsCreatingDevice << " " << IsActive << " -> " << wParam;
			const bool IsDuplicateMessage = (IsActive == wParam);
			if (IsDuplicateMessage || IsForcingWindowedMode)
			{
				LOG_LIMIT(3, __FUNCTION__ << " Warning: filtering " << (IsDuplicateMessage ? "duplicate " : "") << "WM_ACTIVATEAPP: " << wParam);
				return NULL;
			}
			IsActive = wParam;
		}
		break;

	case WM_ACTIVATE:
		// Handle keyboard layout
		if (Config.ForceKeyboardLayout && hWnd == hWndInstance)
		{
			SetKeyboardLayoutFocus(hWnd, LOWORD(wParam) != WA_INACTIVE);
		}
		// Filter duplicate messages when using DirectDraw
		if (pDataStruct->IsDirectDraw)
		{
			if (pDataStruct->IsActive == LOWORD(wParam))
			{
				LOG_LIMIT(3, __FUNCTION__ << " Warning: filtering duplicate WM_ACTIVATE: " << LOWORD(wParam));
				return CallWndProc(nullptr, hWnd, Msg, wParam, lParam);
			}
			pDataStruct->IsActive = LOWORD(wParam);
		}

		// Special handling for iconic state to prevent issues with some games
		if (pDataStruct->IsDirectDraw && IsIconic(hWnd))
		{
			// Tell Windows & game to fully restore
			if (LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE)
			{
				CallWndProc(pWndProc, hWnd, Msg, WA_ACTIVE, NULL);
				CallWndProc(nullptr, hWnd, WM_SYSCOMMAND, SC_RESTORE, NULL);
				SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOACTIVATE);
				SetForegroundWindow(hWnd);
				return NULL;
			}
			// Some games require filtering this when iconic, other games require this message to see when the window is activated
			if (pDataStruct->DirectXVersion <= 4)
			{
				return CallWndProc(nullptr, hWnd, Msg, wParam, lParam);
			}
		}
		break;

	case WM_NCACTIVATE:
		// Filter some messages while forcing windowed mode
		if (pDataStruct->IsDirectDraw && IsForcingWindowedMode)
		{
			return CallWndProc(nullptr, hWnd, Msg, wParam, lParam);
		}
		break;

	case WM_SETFOCUS:
		// Handle keyboard layout
		if (Config.ForceKeyboardLayout && hWnd == hWndInstance)
		{
			SetKeyboardLayoutFocus(hWnd, true);
		}
		break;

	case WM_KILLFOCUS:
		// Handle keyboard layout
		if (Config.ForceKeyboardLayout && hWnd == hWndInstance)
		{
			SetKeyboardLayoutFocus(hWnd, false);
		}
		break;

	case WM_STYLECHANGING:
	case WM_STYLECHANGED:
	case WM_ENTERSIZEMOVE:
	case WM_EXITSIZEMOVE:
	case WM_SIZING:
	case WM_SIZE:
	case WM_WINDOWPOSCHANGING:
		// Filter some messages while forcing windowed mode
		if (pDataStruct->IsCreatingDevice && IsForcingWindowedMode)
		{
			return CallWndProc(nullptr, hWnd, Msg, wParam, lParam);
		}
		break;

	case WM_WINDOWPOSCHANGED:
		// Handle exclusive mode cases where the window is resized to be different than the display size
		if (pDataStruct->IsDirectDraw && pDataStruct->IsExclusiveMode)
		{
			m_IDirectDrawX::CheckWindowPosChange(hWnd, (WINDOWPOS*)lParam);
		}

		// Filter some messages while forcing windowed mode
		if (pDataStruct->IsCreatingDevice && IsForcingWindowedMode)
		{
			return CallWndProc(nullptr, hWnd, Msg, wParam, lParam);
		}
		break;

	case WM_PAINT:
		// Some games hang when attempting to paint while iconic
		if (pDataStruct->IsDirectDraw && IsIconic(hWnd))
		{
			return CallWndProc(nullptr, hWnd, Msg, wParam, lParam);
		}
		break;

	case WM_SYNCPAINT:
		// Send WM_SYNCPAINT to DefWindowProc
		return CallWndProc(nullptr, hWnd, Msg, wParam, lParam);

	case WM_DISPLAYCHANGE:
		// Handle cases where monitor gets disconnected during resolution change
		if (pDataStruct->IsDirectDraw)
		{
			SwitchingResolution = true;
		}
		break;

	case WM_SYSCOMMAND:
		// Set instance as inactive when window closes
		if (wParam == SC_CLOSE && hWnd == hWndInstance)
		{
			AppWndProcInstance->SetInactive();
		}
		break;

	case WM_CLOSE:
	case WM_DESTROY:
	case WM_NCDESTROY:
		// Handle keyboard layout
		if (Config.ForceKeyboardLayout && hWnd == hWndInstance)
		{
			SetKeyboardLayoutFocus(hWnd, false);
		}
		// Set instance as inactive when window closes
		if (hWnd == hWndInstance)
		{
			AppWndProcInstance->SetInactive();
		}
		break;
	}

	// Handle debug overlay
#ifdef ENABLE_DEBUGOVERLAY
	if (Config.EnableImgui)
	{
		ImGuiWndProc(hWnd, Msg, wParam, lParam);
	}
#endif

	return CallWndProc(pWndProc, hWnd, Msg, wParam, lParam);
}
