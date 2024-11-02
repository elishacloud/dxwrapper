#pragma once

#include <DDrawCompat/v0.3.2/Gdi/Gdi.h>

namespace Gdi
{
	namespace WinProc
	{
		void dllThreadDetach();
		void installHooks();
		void onCreateWindow(HWND hwnd);
		void watchWindowPosChanges(WindowPosChangeNotifyFunc notifyFunc);
	}
}
