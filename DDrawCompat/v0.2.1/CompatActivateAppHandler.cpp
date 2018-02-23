#include "CompatActivateAppHandler.h"
#include "CompatDirectDraw.h"
#include "CompatDirectDrawSurface.h"
#include "CompatGdi.h"
#include "CompatPrimarySurface.h"
#include "DDrawLog.h"
#include "RealPrimarySurface.h"

namespace Compat21
{
	extern HWND g_mainWindow;

	namespace
	{
		HWND g_fullScreenCooperativeWindow = nullptr;
		DWORD g_fullScreenCooperativeFlags = 0;
		HHOOK g_callWndProcHook = nullptr;

		void handleActivateApp(bool isActivated);

		LRESULT CALLBACK callWndProc(int nCode, WPARAM wParam, LPARAM lParam)
		{
			auto ret = reinterpret_cast<CWPSTRUCT*>(lParam);
			Logging::LogEnter("callWndProc", nCode, wParam, ret);

			if (HC_ACTION == nCode && WM_ACTIVATEAPP == ret->message)
			{
				const bool isActivated = TRUE == ret->wParam;
				handleActivateApp(isActivated);
			}

			LRESULT result = CallNextHookEx(nullptr, nCode, wParam, lParam);
			Logging::LogLeave("callWndProc", nCode, wParam, ret) << result;
			return result;
		}

		void handleActivateApp(bool isActivated)
		{
			Logging::LogEnter("handleActivateApp", isActivated);

			static bool isActive = true;
			if (isActivated != isActive && RealPrimarySurface::isFullScreen())
			{
				IUnknown* ddIntf = nullptr;
				CompatDirectDrawSurface<IDirectDrawSurface7>::s_origVtable.GetDDInterface(
					RealPrimarySurface::getSurface(), reinterpret_cast<void**>(&ddIntf));
				IDirectDraw7* dd = nullptr;
				ddIntf->lpVtbl->QueryInterface(ddIntf, IID_IDirectDraw7, reinterpret_cast<void**>(&dd));

				if (isActivated)
				{
					CompatDirectDraw<IDirectDraw7>::s_origVtable.SetCooperativeLevel(
						dd, g_fullScreenCooperativeWindow, g_fullScreenCooperativeFlags);
					if (CompatPrimarySurface::isDisplayModeChanged)
					{
						const CompatPrimarySurface::DisplayMode& dm = CompatPrimarySurface::displayMode;
						CompatDirectDraw<IDirectDraw7>::s_origVtable.SetDisplayMode(
							dd, dm.width, dm.height, 32, dm.refreshRate, 0);
					}
					CompatGdi::enableEmulation();
				}
				else
				{
					CompatGdi::disableEmulation();
					if (CompatPrimarySurface::isDisplayModeChanged)
					{
						CompatDirectDraw<IDirectDraw7>::s_origVtable.RestoreDisplayMode(dd);
					}
					CompatDirectDraw<IDirectDraw7>::s_origVtable.SetCooperativeLevel(
						dd, g_fullScreenCooperativeWindow, DDSCL_NORMAL);
					ShowWindow(g_fullScreenCooperativeWindow, SW_MINIMIZE);
				}

				dd->lpVtbl->Release(dd);
				ddIntf->lpVtbl->Release(ddIntf);
			}

			isActive = isActivated;
			Logging::LogLeave("handleActivateApp", isActivated);
		}
	}

	namespace CompatActivateAppHandler
	{
		void installHooks()
		{
			const DWORD threadId = GetCurrentThreadId();
			g_callWndProcHook = SetWindowsHookEx(WH_CALLWNDPROC, callWndProc, nullptr, threadId);
		}

		void setFullScreenCooperativeLevel(HWND hwnd, DWORD flags)
		{
			g_fullScreenCooperativeWindow = hwnd;
			g_fullScreenCooperativeFlags = flags;
		}

		void uninstallHooks()
		{
			UnhookWindowsHookEx(g_callWndProcHook);
		}
	}
}