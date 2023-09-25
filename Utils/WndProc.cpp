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
		LONG_PTR SetWndProc(HWND hWnd, WNDPROC ProcAddress);

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
		public:
			HWND hWnd = nullptr;
			WNDPROC MyWndProc = 0;
			WNDPROC AppWndProc = 0;
			WNDPROCSTRUCT()
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
		};

		std::vector<std::shared_ptr<WNDPROCSTRUCT>> WndProcList;
	}
}

using namespace Utils;

WNDPROC WndProc::GetWndProc(HWND hWnd)
{
	return reinterpret_cast<WNDPROC>(IsWindowUnicode(hWnd) ?
		GetWindowLongPtrW(hWnd, GWL_WNDPROC) :
		GetWindowLongPtrA(hWnd, GWL_WNDPROC));
}

LONG_PTR WndProc::SetWndProc(HWND hWnd, WNDPROC ProcAddress)
{
	return (IsWindowUnicode(hWnd) ?
		SetWindowLongPtrW(hWnd, GWL_WNDPROC, (LONG)ProcAddress) :
		SetWindowLongPtrA(hWnd, GWL_WNDPROC, (LONG)ProcAddress));
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
		if (entry->hWnd == hWnd)
		{
			return false;
		}
	}

	// Create new struct
	auto NewEntry = std::make_shared<WNDPROCSTRUCT>();
	NewEntry->hWnd = hWnd;

	// Get existing WndProc and give it to struct
	NewEntry->AppWndProc = GetWndProc(hWnd);
	if (!NewEntry->AppWndProc)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: could not get wndproc window pointer!");
		return false;
	}

	// Get new WndProc
	WNDPROC NewWndProc = NewEntry->MyWndProc;
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
			return (AppWndProcInstance->hWnd == hWnd);
		});

	// Erase removed instances from the vector
	WndProcList.erase(newEnd, WndProcList.end());
}

LRESULT CALLBACK WndProc::Handler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, WNDPROCSTRUCT* AppWndProcInstance)
{
	const WNDPROC pWndProc = (AppWndProcInstance) ? AppWndProcInstance->AppWndProc : DefWindowProc;
	const HWND hWndInstance = (AppWndProcInstance) ? AppWndProcInstance->hWnd : nullptr;

	Logging::LogDebug() << __FUNCTION__ << " " << hWnd << " " << Logging::hex(msg);

	if (!AppWndProcInstance)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: invalid pointer!");
	}

	// Clean up instance when window closes
	if ((msg == WM_CLOSE || msg == WM_DESTROY || msg == WM_NCDESTROY || (msg == WM_SYSCOMMAND && wParam == SC_CLOSE)) && hWnd == hWndInstance)
	{
		RemoveWndProc(hWnd);
	}

	return pWndProc(hWnd, msg, wParam, lParam);
}
