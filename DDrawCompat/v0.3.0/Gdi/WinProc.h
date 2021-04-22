#pragma once

#include <Gdi/Gdi.h>

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
