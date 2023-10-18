/**
* Copyright (C) 2023 Elisha Riedlinger
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
#include "d3d9\d3d9External.h"
#include "Utils.h"
#include "Settings\Settings.h"
#include "Logging\Logging.h"

namespace Utils
{
	namespace WndProc
	{
		struct WNDPROCSTRUCT;

		LRESULT CALLBACK Handler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, WNDPROCSTRUCT* AppWndProcInstance);
		WNDPROC GetWndProc(HWND hWnd);
		LONG SetWndProc(HWND hWnd, WNDPROC ProcAddress);
		LRESULT CallWndProc(WNDPROC lpPrevWndFunc, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

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
			DWORD oldProtect = 0;
			HWND hWnd = nullptr;
			WNDPROC MyWndProc = 0;
			WNDPROC AppWndProc = 0;
			bool Exiting = false;
		public:
			WNDPROCSTRUCT(HWND p_hWnd, WNDPROC p_AppWndProc) : hWnd(p_hWnd), AppWndProc(p_AppWndProc)
			{
				// Set memory protection to make it executable
				if (VirtualProtect(FunctCode, sizeof(FunctCode), PAGE_EXECUTE_READWRITE, &oldProtect))
				{
					*pFunctVar = (LONG)this;
					*pFunctCall = (int)&Handler - ((int)pFunctCall + 4);
					MyWndProc = reinterpret_cast<WNDPROC>((LONG)FunctCode);
				}
			}
			~WNDPROCSTRUCT()
			{
				Exiting = true;
				if (Config.Exiting)
				{
					return;
				}
				// Restore the memory protection
				if (MyWndProc)
				{
					DWORD tmpProtect = 0;
					VirtualProtect(FunctCode, sizeof(FunctCode), oldProtect, &tmpProtect);
				}
				// Restore WndProc
				if (hWnd && AppWndProc)
				{
					LOG_LIMIT(100, __FUNCTION__ << " Deleting WndProc instance! " << hWnd);
					SetWndProc(hWnd, AppWndProc);
				}
			}
			HWND GetHWnd() { return hWnd; }
			WNDPROC GetMyWndProc() { return MyWndProc; }
			WNDPROC GetAppWndProc() { return AppWndProc; }
			bool IsExiting() { return Exiting; }
		};

		std::vector<std::shared_ptr<WNDPROCSTRUCT>> WndProcList;
	}
}

using namespace Utils;

WNDPROC WndProc::CheckWndProc(HWND hWnd, LONG dwNewLong)
{
	for (auto& entry : WndProcList)
	{
		if (entry->GetHWnd() == hWnd && !(entry->IsExiting() && (LONG)entry->GetAppWndProc() == dwNewLong))
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

bool WndProc::AddWndProc(HWND hWnd)
{
	// Validate window handle
	if (!IsWindow(hWnd))
	{
		return false;
	}

	// Check if window is already hooked
	for (auto& entry : WndProcList)
	{
		if (entry->GetHWnd() == hWnd)
		{
			return true;
		}
	}

	// Check WndProc in struct
	WNDPROC NewAppWndProc = GetWndProc(hWnd);
	if (!NewAppWndProc)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: could not get wndproc window pointer!");
		return false;
	}

	// Create new struct
	auto NewEntry = std::make_shared<WNDPROCSTRUCT>(hWnd, NewAppWndProc);

	// Get new WndProc
	WNDPROC NewWndProc = NewEntry->GetMyWndProc();
	if (!NewWndProc)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: could not get function target!");
		return false;
	}

	// Set new window pointer and store struct address
	LOG_LIMIT(100, __FUNCTION__ << " Creating WndProc instance! " << hWnd);
	SetWndProc(hWnd, NewWndProc);
	WndProcList.push_back(NewEntry);
	return true;
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

LRESULT CALLBACK WndProc::Handler(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, WNDPROCSTRUCT* AppWndProcInstance)
{
	const WNDPROC pWndProc = (AppWndProcInstance) ? AppWndProcInstance->GetAppWndProc() : nullptr;
	const HWND hWndInstance = (AppWndProcInstance) ? AppWndProcInstance->GetHWnd() : nullptr;

	Logging::LogDebug() << __FUNCTION__ << " " << hWnd << " " << Logging::hex(Msg);

	if (!AppWndProcInstance)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: invalid pointer!");
	}

	// Handle debug overlay
#ifdef ENABLE_DEBUGOVERLAY
	ImGuiWndProc(hWnd, Msg, wParam, lParam);
#endif

	// Clean up instance when window closes
	if ((Msg == WM_CLOSE || Msg == WM_DESTROY || Msg == WM_NCDESTROY || (Msg == WM_SYSCOMMAND && wParam == SC_CLOSE)) && hWnd == hWndInstance)
	{
		RemoveWndProc(hWnd);
	}

	return CallWndProc(pWndProc, hWnd, Msg, wParam, lParam);
}
