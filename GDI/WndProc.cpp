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

#undef DefWindowProc

namespace WndProc
{
	struct WNDPROCSTRUCT;

	LRESULT CALLBACK Handler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, WNDPROCSTRUCT* AppWndProcInstance);
	WNDPROC GetWndProc(HWND hWnd);
	LONG SetWndProc(HWND hWnd, WNDPROC ProcAddress);
	LRESULT CallWndProc(WNDPROC lpPrevWndFunc, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	LRESULT DefWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	bool IsExecutableAddress(void* address);

	bool SwitchingResolution = false;

	void HandleWindowFocus(HWND hWnd, DATASTRUCT* pDataStruct, bool IsActivating);
	void HandleClipMouseCursor(HWND hWnd, DATASTRUCT* pDataStruct, bool IsActivating);

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
		std::atomic<bool> Active = true;
		std::atomic<bool> ReadyToDelete = false;
		std::atomic<bool> Exiting = false;
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
			if (IsWindow(hWnd))
			{
				if (Config.Exiting)
				{
					SetToDefault();
				}
				// Restore WndProc
				else if (AppWndProc)
				{
					LOG_LIMIT(100, __FUNCTION__ << " Deleting WndProc instance! " << hWnd);
					SetWndProc(hWnd, AppWndProc);
				}
			}
		}
		HWND GetHWnd() const { return hWnd; }
		WNDPROC GetMyWndProc() const { return MyWndProc; }
		WNDPROC GetAppWndProc() const { return AppWndProc; }
		DATASTRUCT* GetDataStruct() { return &DataStruct; }
		void SetToDefault()
		{
			if (IsWindowUnicode(hWnd))
			{
				AppWndProc = DefWindowProcW;
				SetWindowLongW(hWnd, GWL_WNDPROC, (LONG)DefWindowProcW);
			}
			else
			{
				AppWndProc = DefWindowProcA;
				SetWindowLongA(hWnd, GWL_WNDPROC, (LONG)DefWindowProcA);
			}
		}
		inline bool IsActive() const { return Active; }
		inline void SetInactive() { Active = false; }
		inline bool IsReadyToDelete() const { return ReadyToDelete; }
		inline void MarkReadyToDelete() { ReadyToDelete = true; }
		inline bool IsExiting() const { return Exiting; }
	};

	std::vector<std::shared_ptr<WNDPROCSTRUCT>> WndProcList;

	inline static bool CheckWndProc(std::shared_ptr<WNDPROCSTRUCT> pDataStruct, HWND hWnd)
	{
		return (pDataStruct->GetHWnd() == hWnd && pDataStruct->IsActive() && !pDataStruct->IsExiting());
	}
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

LRESULT WndProc::DefWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	return (IsWindowUnicode(hWnd) ?
		DefWindowProcW(hWnd, Msg, wParam, lParam) :
		DefWindowProcA(hWnd, Msg, wParam, lParam));
}

bool WndProc::ShouldHook(HWND hWnd)
{
	if (!IsWindow(hWnd))
	{
		return false;
	}

	const LONG_PTR style = GetWindowLongPtr(hWnd, GWL_STYLE);

	// Child windows can NEVER be foreground
	if (style & WS_CHILD)
	{
		return false;
	}

	// Message-only windows can NEVER activate
	if (GetAncestor(hWnd, GA_PARENT) == HWND_MESSAGE)
	{
		return false;
	}

	// Must be true top-level
	if (GetAncestor(hWnd, GA_ROOT) != hWnd)
	{
		return false;
	}

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
	RemoveInactiveWndProcs();

	// Check if window is already hooked
	for (const auto& entry : WndProcList)
	{
		if (CheckWndProc(entry, hWnd))
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
	if (Config.ForceKeyboardLayout && hWnd == GetForegroundWindow())
	{
		PostMessage(hWnd, WM_APP_SET_KEYBOARD_LAYOUT, (WPARAM)NewWndProc, WM_MAKE_KEY(hWnd, NewWndProc));
	}

	// Return
	return NewEntry->GetDataStruct();
}

void WndProc::RemoveInactiveWndProcs()
{
	// Remove inactive elements
	for (auto it = WndProcList.begin(); it != WndProcList.end();)
	{
		if (CheckWndProc(*it, (*it)->GetHWnd()))
		{
			// Valid record
			it++;
		}
		else
		{
			// Invalid record
			if ((*it)->IsReadyToDelete() == false)
			{
				// Don't delete on first pass instead mark as ready to delete
				(*it)->MarkReadyToDelete();
				it++;
			}
			else
			{
				// Delete record
				it = WndProcList.erase(it);
			}
		}
	}
}

WndProc::DATASTRUCT* WndProc::GetWndProctStruct(HWND hWnd)
{
	for (const auto& entry : WndProcList)
	{
		if (CheckWndProc(entry, hWnd))
		{
			return entry->GetDataStruct();
		}
	}
	return nullptr;
}

DWORD WndProc::MakeKey(DWORD Val1, DWORD Val2)
{
	// Single-step mixer
	auto Mix = [](DWORD v) -> DWORD {
		v ^= v >> 16;
		v *= 0x7feb352d;
		v ^= v >> 15;
		v *= 0x846ca68b;
		v ^= v >> 16;
		return v;
		};

	// Combine two inputs in a simple, asymmetric way
	DWORD Result = Mix(Val1) + 0x9E3779B9;   // add golden ratio constant
	Result ^= Mix(Val2) + (Result << 6) + (Result >> 2);

	// Final avalanche
	return Mix(Result);
}

inline void WndProc::HandleWindowFocus(HWND hWnd, DATASTRUCT* pDataStruct, bool IsActivating)
{
	// Handle keyboard layout
	if (Config.ForceKeyboardLayout)
	{
		SetKeyboardLayoutFocus(hWnd, IsActivating);
	}

	// Handle window clipping
	HandleClipMouseCursor(hWnd, pDataStruct, IsActivating);
}

inline void WndProc::HandleClipMouseCursor(HWND hWnd, DATASTRUCT* pDataStruct, bool IsActivating)
{
	if (Config.EnableCursorClip && pDataStruct->IsDirect3D9)
	{
		if (IsActivating && pDataStruct->InSizeMove == false)
		{
			Utils::ClipMouseCursor(hWnd, pDataStruct->ClipWidth, pDataStruct->ClipHeight);
		}
		else
		{
			Utils::UnClipMouseCursor();
		}
	}
}

inline void WndProc::SetKeyboardLayoutFocus(HWND hWnd, bool IsActivating)
{
	// On Activation
	if (IsActivating)
	{
		PostMessage(hWnd, WM_APP_SET_KEYBOARD_LAYOUT, (WPARAM)hWnd, WM_MAKE_KEY(hWnd, hWnd));
	}
	// On Deactivation
	else if (IsKeyboardActive && hWnd == GetForegroundWindow())
	{
		IsKeyboardActive = false;
		KeyboardLayout::UnSetForcedKeyboardLayout();
	}
}

void WndProc::DisableForcedKeyboardLayout()
{
	HWND hWnd = GetForegroundWindow();
	if (GetWndProctStruct(hWnd))
	{
		PostMessage(hWnd, WM_APP_DISABLE_KEYBOARD_LAYOUT, (WPARAM)hWnd, WM_MAKE_KEY(hWnd, hWnd));
	}

	KeyboardLayout::DisableForcedKeyboardLayout();
}

LRESULT CALLBACK WndProc::Handler(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, WNDPROCSTRUCT* AppWndProcInstance)
{
	if (Msg != WM_PAINT)
	{
		//Logging::Log() << __FUNCTION__ << " " << hWnd << " " << Logging::hex(Msg) << " " << wParam << " " << lParam << " IsIconic: " << IsIconic(hWnd);
	}

	if (!AppWndProcInstance || !hWnd)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: invalid pointer!");
		return 0;
	}

	const WNDPROC pWndProc = AppWndProcInstance->GetAppWndProc();
	const HWND hWndInstance = AppWndProcInstance->GetHWnd();
	DATASTRUCT* pDataStruct = AppWndProcInstance->GetDataStruct();

	if (AppWndProcInstance->IsExiting() || !AppWndProcInstance->IsActive())
	{
		// WndProc is inactive or exiting use default WndProc instead
		return DefWndProc(hWnd, Msg, wParam, lParam);
	}
	else if (hWnd != hWndInstance)
	{
		// Messages don't apply to this window
		return CallWndProc(pWndProc, hWnd, Msg, wParam, lParam);
	}

	switch (Msg)
	{
	case WM_APP_CREATE_D3D9_DEVICE:
		if (WM_MAKE_KEY(hWnd, wParam) == lParam)
		{
			if (m_IDirectDrawX::CheckDirectDrawXInterface((void*)wParam))
			{
				((m_IDirectDrawX*)wParam)->CreateD9Device(__FUNCTION__);
			}
			return 0;
		}
		break;

	case WM_APP_RESET_D3D9_DEVICE:
		if (WM_MAKE_KEY(hWnd, wParam) == lParam)
		{
			LOG_LIMIT(3, __FUNCTION__ << " Handling device window activation. Iconic: " << IsIconic(hWnd));

			// Restore window
			if (IsIconic(hWnd))
			{
				LOG_LIMIT(3, __FUNCTION__ << " Restoring iconic window: " << hWnd);
				ShowWindow(hWnd, SW_RESTORE);
			}

			// Reset device once restored
			if (!IsIconic(hWnd))
			{
				LOG_LIMIT(3, __FUNCTION__ << " Attempting to reset device.");

				if (m_IDirectDrawX::TriggerDeviceReset(hWnd) == DDERR_SURFACELOST)
				{
					LOG_LIMIT(3, __FUNCTION__ << " Reset failed because device is still lost!");

					ShowWindow(hWnd, SW_MINIMIZE);
					ShowWindow(hWnd, SW_RESTORE);
				}
			}

			return 0;
		}
		break;

	case WM_APP_SET_KEYBOARD_LAYOUT:
		if (WM_MAKE_KEY(hWnd, wParam) == lParam)
		{
			if (Config.ForceKeyboardLayout && hWnd == GetForegroundWindow() && AppWndProcInstance->IsActive())
			{
				IsKeyboardActive = true;
				KeyboardLayout::SetForcedKeyboardLayout();
			}
			return 0;
		}
		break;

	case WM_APP_DISABLE_KEYBOARD_LAYOUT:
		if (WM_MAKE_KEY(hWnd, wParam) == lParam)
		{
			if (Config.ForceKeyboardLayout && hWnd == GetForegroundWindow())
			{
				KeyboardLayout::DisableForcedKeyboardLayout();
			}
			return 0;
		}
		break;

	case WM_ACTIVATEAPP:
		// Handle window focus change
		HandleWindowFocus(hWnd, pDataStruct, wParam != FALSE);

		// Some games don't properly handle app activate in exclusive mode
		if (pDataStruct->IsDirectDraw)
		{
			static WPARAM IsActiveApp = pDataStruct->IsExclusiveMode && pDataStruct->DirectXVersion > 4 ? TRUE : 0xFFFF;

			const bool IsDuplicateMessage = (IsActiveApp == wParam);

			if (IsDuplicateMessage || Config.DdrawFilterActivateApp)
			{
				LOG_LIMIT(3, __FUNCTION__ << " Warning: filtering " << (IsDuplicateMessage ? "duplicate " : "") << "WM_ACTIVATEAPP: " << wParam);
				return 0;
			}
			IsActiveApp = wParam;
		}
		// Filter messages for loss of focus or minimize
		if (Config.HideWindowFocusChanges && wParam == FALSE)
		{
			return DefWndProc(hWnd, Msg, wParam, lParam);
		}
		break;

	case WM_ACTIVATE:
		// Handle window focus change
		HandleWindowFocus(hWnd, pDataStruct, LOWORD(wParam) != WA_INACTIVE);

		// Filter duplicate messages when using DirectDraw
		if (pDataStruct->IsDirectDraw)
		{
			const BOOL IsWindowIconic = IsIconic(hWnd);
			const bool IsDuplicateMessage = (pDataStruct->IsWindowActive == wParam);
			const bool IsIconicMatches = (pDataStruct->IsWindowIconic == IsWindowIconic);

			if (IsDuplicateMessage && IsIconicMatches)
			{
				LOG_LIMIT(3, __FUNCTION__ << " Warning: filtering duplicate WM_ACTIVATE: " << LOWORD(wParam) << " IsIconic: " << IsWindowIconic);
				return DefWndProc(hWnd, Msg, wParam, lParam);
			}
			pDataStruct->IsWindowActive = wParam;
			pDataStruct->IsWindowIconic = IsWindowIconic;
		}
		// Special handling for iconic state to prevent issues with some games
		if (pDataStruct->IsDirectDraw && IsIconic(hWnd))
		{
			const UINT activateState = LOWORD(wParam);
			const bool isActivating = (activateState == WA_ACTIVE || activateState == WA_CLICKACTIVE);
			const bool filterMessage = (pDataStruct->DirectXVersion <= 4);

			if (isActivating)
			{
				LOG_LIMIT(3, __FUNCTION__ << " Sending message to activate device window.");
				PostMessage(hWnd, WM_APP_RESET_D3D9_DEVICE, (WPARAM)hWnd, WM_MAKE_KEY(hWnd, hWnd));
			}

			if (filterMessage)
			{
				LOG_LIMIT(3, __FUNCTION__ << " Warning: filtering WM_ACTIVATE when iconic. Active: " << activateState);
				return DefWndProc(hWnd, Msg, wParam, lParam);
			}
			else
			{
				return CallWndProc(pWndProc, hWnd, Msg, wParam, lParam);;
			}
		}
		// Filter messages for loss of focus or minimize
		if (Config.HideWindowFocusChanges && LOWORD(wParam) == WA_INACTIVE)
		{
			return DefWndProc(hWnd, Msg, wParam, lParam);
		}
		break;

	case WM_NCACTIVATE:
		// Handle window focus change
		HandleWindowFocus(hWnd, pDataStruct, LOWORD(wParam) != FALSE);

		// Filter messages for loss of focus or minimize
		if (Config.HideWindowFocusChanges && wParam == FALSE)
		{
			return DefWndProc(hWnd, Msg, wParam, lParam);
		}
		// Filter some messages while forcing windowed mode
		if (pDataStruct->IsDirectDraw && (Config.EnableWindowMode && pDataStruct->IsExclusiveMode))
		{
			LOG_LIMIT(3, __FUNCTION__ << " Warning: filtering WM_NCACTIVATE when forcing windowed mode. " <<
				hWnd << " " << Logging::hex(Msg) << " " << wParam << " " << lParam << " IsIconic: " << IsIconic(hWnd));
			return DefWndProc(hWnd, Msg, wParam, lParam);
		}
		break;

	case WM_SETFOCUS:
		// Handle window focus change
		HandleWindowFocus(hWnd, pDataStruct, true);
		break;

	case WM_KILLFOCUS:
		// Handle window focus change
		HandleWindowFocus(hWnd, pDataStruct, false);

		// Filter messages for loss of focus or minimize
		if (Config.HideWindowFocusChanges && wParam == FALSE)
		{
			return DefWndProc(hWnd, Msg, wParam, lParam);
		}
		break;

	case WM_SHOWWINDOW:
		// Handle window focus change
		HandleWindowFocus(hWnd, pDataStruct, wParam != FALSE);

		// Filter messages for loss of focus or minimize
		if (Config.HideWindowFocusChanges && wParam == FALSE)
		{
			return DefWndProc(hWnd, Msg, wParam, lParam);
		}
		break;

	case WM_MOVE:
		// Hide minimized window location from game
		if (pDataStruct->IsDirectDraw)
		{
			const int x = (int)(short)LOWORD(lParam);
			const int y = (int)(short)HIWORD(lParam);

			if (x == -32000 && y == -32000)
			{
				return DefWndProc(hWnd, Msg, wParam, lParam);
			}
		}
		break;

	case WM_SIZE:
		// Handle window focus change
		HandleWindowFocus(hWnd, pDataStruct, wParam != SIZE_MINIMIZED);

		// Filter messages for loss of focus or minimize
		if (Config.HideWindowFocusChanges && wParam == SIZE_MINIMIZED)
		{
			return DefWndProc(hWnd, Msg, wParam, lParam);
		}
		// Filter size change when creating device
		// Some games (e.g. Police Quest SWAT 2, Splintercell Chaos Theory) don't handle size messages well
		if (pDataStruct->IsCreatingDevice)
		{
			LOG_LIMIT(3, __FUNCTION__ << " Warning: filtering some messages when creating device. " <<
				hWnd << " " << Logging::hex(Msg) << " " << wParam << " " << lParam << " IsIconic: " << IsIconic(hWnd));
			return DefWndProc(hWnd, Msg, wParam, lParam);
		}
		break;

	case WM_ENTERSIZEMOVE:
	case WM_EXITSIZEMOVE:
		// Handle window focus change
		pDataStruct->InSizeMove = (Msg == WM_ENTERSIZEMOVE);
		HandleClipMouseCursor(hWnd, pDataStruct, true);

		[[fallthrough]];
	case WM_STYLECHANGING:
	case WM_STYLECHANGED:
	case WM_SIZING:
	case WM_MOVING:
		// Filter some messages when creating device
		if (pDataStruct->IsCreatingDevice)
		{
			LOG_LIMIT(3, __FUNCTION__ << " Warning: filtering some messages when creating device. " <<
				hWnd << " " << Logging::hex(Msg) << " " << wParam << " " << lParam << " IsIconic: " << IsIconic(hWnd));
			return DefWndProc(hWnd, Msg, wParam, lParam);
		}
		break;

	case WM_WINDOWPOSCHANGED:
		if (lParam)
		{
			WINDOWPOS* WinPos = reinterpret_cast<WINDOWPOS*>(lParam);

			// Handle window focus change
			if (WinPos->flags & SWP_HIDEWINDOW)
			{
				HandleWindowFocus(hWnd, pDataStruct, false);
			}
			else if (WinPos->flags & SWP_SHOWWINDOW)
			{
				HandleWindowFocus(hWnd, pDataStruct, true);
			}
			else if (Config.EnableCursorClip && pDataStruct->IsDirect3D9 &&
				(hWnd == GetFocus() || hWnd == GetActiveWindow() || hWnd == GetForegroundWindow()))
			{
				HandleClipMouseCursor(hWnd, pDataStruct, true);
			}
		}
		[[fallthrough]];
	case WM_WINDOWPOSCHANGING:
		if (lParam)
		{
			WINDOWPOS* WinPos = reinterpret_cast<WINDOWPOS*>(lParam);

			// Disallow negative top, some games (e.g. The Summoner [-20]) will overwrite the y coordinate to a negative number
			if (WinPos->y < 0 && !GetMenu(hWnd))
			{
				WinPos->y = 0;
			}
			// Check if need to modify the size
			if (!(WinPos->flags & SWP_NOSIZE))
			{
				// Handle exclusive mode cases where the window is resized to be different than the display size (e.g. Call to Power 2)
				if (pDataStruct->IsDirectDraw && pDataStruct->IsExclusiveMode)
				{
					m_IDirectDrawX::CheckFixWindowPos(hWnd, WinPos);
				}

				RECT rc = {};
				GetWindowRect(hWnd, &rc);

				const LONG CurCX = rc.right - rc.left;
				const LONG CurCY = rc.bottom - rc.top;

				// Don't resize if no changes in window size (e.g. Twisted Metal 2)
				if (WinPos->cx == CurCX && WinPos->cy == CurCY)
				{
					WinPos->flags |= SWP_NOSIZE;
				}
			}
			// Filter messages for loss of focus or minimize
			if (Config.HideWindowFocusChanges && (WinPos->flags & SWP_HIDEWINDOW))
			{
				return DefWndProc(hWnd, Msg, wParam, lParam);
			}
			// Hide minimized window location from game
			if (pDataStruct->IsDirectDraw && WinPos->x == -32000 && WinPos->y == -32000)
			{
				return DefWndProc(hWnd, Msg, wParam, lParam);
			}
		}
		break;

	case WM_PAINT:
		// Some games hang when attempting to paint while iconic
		if (pDataStruct->IsDirectDraw && IsIconic(hWnd))
		{
			return DefWndProc(hWnd, Msg, wParam, lParam);
		}
		break;

	case WM_SYNCPAINT:
		// Send WM_SYNCPAINT to DefWindowProc
		return DefWndProc(hWnd, Msg, wParam, lParam);

	case WM_ERASEBKGND:
		// Filter background clear when creating device or in exclusive mode
		if (pDataStruct->IsCreatingDevice || pDataStruct->IsExclusiveMode)
		{
			return TRUE;
		}
		break;

	case WM_DISPLAYCHANGE:
		// Handle cases where monitor gets disconnected during resolution change
		if (pDataStruct->IsDirectDraw)
		{
			SwitchingResolution = true;
		}
		break;

	case WM_SYSCOMMAND:
		// Handle window focus change
		if ((wParam & 0xFFF0) == SC_MINIMIZE || (wParam & 0xFFF0) == SC_MAXIMIZE)
		{
			HandleWindowFocus(hWnd, pDataStruct, (wParam & 0xFFF0) == SC_MAXIMIZE);
		}
		// Filter messages for loss of focus or minimize
		if (Config.HideWindowFocusChanges && (wParam & 0xFFF0) == SC_MINIMIZE)
		{
			return DefWndProc(hWnd, Msg, wParam, lParam);
		}
		break;

	case WM_NCDESTROY:
		// Handle window focus change
		HandleWindowFocus(hWnd, pDataStruct, false);

		// Release d3d9 interface
		if (pDataStruct->IsDirectDraw)
		{
			m_IDirectDrawX::TriggerDeviceRelease(hWnd);
		}

		// Set instance to default before window closes
		AppWndProcInstance->SetToDefault();

		// Final WndProc call
		LRESULT lr =  CallWndProc(pWndProc, hWnd, Msg, wParam, lParam);

		// Set instance as inactive when window closes
		AppWndProcInstance->SetInactive();

		return lr;
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
