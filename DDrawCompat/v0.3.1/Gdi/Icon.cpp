#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <DDrawCompat/v0.3.1/Common/Hook.h>
#include <DDrawCompat/DDrawLog.h>
#include <DDrawCompat/v0.3.1/Gdi/DcFunctions.h>
#include <DDrawCompat/v0.3.1/Gdi/Icon.h>

namespace
{
	template <auto func>
	const char* g_funcName = nullptr;

	template <auto origFunc, typename... Params>
	bool isDibRedirectionEnabled(Params...)
	{
		return false;
	}

	template <>
	bool isDibRedirectionEnabled<&CopyImage>(HANDLE, UINT type, int, int, UINT)
	{
		return IMAGE_CURSOR != type && IMAGE_ICON != type;
	}

	template <>
	bool isDibRedirectionEnabled<&LoadImageA>(HINSTANCE, LPCSTR, UINT type, int, int, UINT)
	{
		return IMAGE_CURSOR != type && IMAGE_ICON != type;
	}

	template <>
	bool isDibRedirectionEnabled<&LoadImageW>(HINSTANCE, LPCWSTR, UINT type, int, int, UINT)
	{
		return IMAGE_CURSOR != type && IMAGE_ICON != type;
	}

	template <auto origFunc, typename Result, typename... Params>
	Result WINAPI iconFunc(Params... params)
	{
		LOG_FUNC(g_funcName<origFunc>, params...);

		if (isDibRedirectionEnabled<origFunc>(params...))
		{
			return LOG_RESULT(Compat31::g_origFuncPtr<origFunc>(params...));
		}

		Gdi::DcFunctions::disableDibRedirection(true);
		Result result = Compat31::g_origFuncPtr<origFunc>(params...);
		Gdi::DcFunctions::disableDibRedirection(false);
		return LOG_RESULT(result);
	}

	template <auto origFunc>
	void hookIconFunc(const char* moduleName, const char* funcName)
	{
#ifdef DEBUGLOGS
		g_funcName<origFunc> = funcName;
#endif

		Compat31::hookFunction<origFunc>(moduleName, funcName, &iconFunc<origFunc>);
	}

	template <typename WndClass, typename WndClassEx>
	ATOM registerClass(const WndClass* lpWndClass, ATOM(WINAPI* origRegisterClassEx)(const WndClassEx*))
	{
		if (!lpWndClass)
		{
			return origRegisterClassEx(nullptr);
		}

		WndClassEx wc = {};
		wc.cbSize = sizeof(wc);
		memcpy(&wc.style, lpWndClass, sizeof(*lpWndClass));
		wc.hIconSm = wc.hIcon;
		return origRegisterClassEx(&wc);
	}

	template <typename WndClassEx>
	ATOM registerClassEx(const WndClassEx* lpwcx, ATOM(WINAPI* origRegisterClassEx)(const WndClassEx*))
	{
		if (!lpwcx)
		{
			return origRegisterClassEx(nullptr);
		}

		WndClassEx wc = *lpwcx;
		if (!wc.hIconSm)
		{
			wc.hIconSm = wc.hIcon;
		}
		return origRegisterClassEx(&wc);
	}

	ATOM WINAPI registerClassA(const WNDCLASSA* lpWndClass)
	{
		LOG_FUNC("RegisterClassA", lpWndClass);
		return LOG_RESULT(registerClass(lpWndClass, CALL_ORIG_FUNC(RegisterClassExA)));
	}

	ATOM WINAPI registerClassW(const WNDCLASSW* lpWndClass)
	{
		LOG_FUNC("RegisterClassW", lpWndClass);
		return LOG_RESULT(registerClass(lpWndClass, CALL_ORIG_FUNC(RegisterClassExW)));
	}

	ATOM WINAPI registerClassExA(const WNDCLASSEXA* lpwcx)
	{
		LOG_FUNC("RegisterClassExA", lpwcx);
		return LOG_RESULT(registerClassEx(lpwcx, CALL_ORIG_FUNC(RegisterClassExA)));
	}

	ATOM WINAPI registerClassExW(const WNDCLASSEXW* lpwcx)
	{
		LOG_FUNC("RegisterClassExW", lpwcx);
		return LOG_RESULT(registerClassEx(lpwcx, CALL_ORIG_FUNC(RegisterClassExW)));
	}
}

#define HOOK_ICON_FUNCTION(module, func) hookIconFunc<&func>(#module, #func)

namespace Gdi
{
	namespace Icon
	{
		void installHooks()
		{
			HOOK_ICON_FUNCTION(user32, CopyIcon);
			HOOK_ICON_FUNCTION(user32, CopyImage);
			HOOK_ICON_FUNCTION(user32, CreateCursor);
			HOOK_ICON_FUNCTION(user32, CreateIcon);
			HOOK_ICON_FUNCTION(user32, CreateIconFromResource);
			HOOK_ICON_FUNCTION(user32, CreateIconFromResourceEx);
			HOOK_ICON_FUNCTION(user32, CreateIconIndirect);
			HOOK_ICON_FUNCTION(user32, LoadCursorA);
			HOOK_ICON_FUNCTION(user32, LoadCursorW);
			HOOK_ICON_FUNCTION(user32, LoadCursorFromFileA);
			HOOK_ICON_FUNCTION(user32, LoadCursorFromFileW);
			HOOK_ICON_FUNCTION(user32, LoadIconA);
			HOOK_ICON_FUNCTION(user32, LoadIconW);
			HOOK_ICON_FUNCTION(user32, LoadImageA);
			HOOK_ICON_FUNCTION(user32, LoadImageW);
			HOOK_ICON_FUNCTION(user32, PrivateExtractIconsA);
			HOOK_ICON_FUNCTION(user32, PrivateExtractIconsW);

			HOOK_FUNCTION(user32, RegisterClassA, registerClassA);
			HOOK_FUNCTION(user32, RegisterClassW, registerClassW);
			HOOK_FUNCTION(user32, RegisterClassExA, registerClassExA);
			HOOK_FUNCTION(user32, RegisterClassExW, registerClassExW);
		}
	}
}
