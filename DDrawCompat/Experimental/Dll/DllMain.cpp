#define WIN32_LEAN_AND_MEAN

#include <string>

#include <Windows.h>
//********** Begin Edit *************
#include "winmm.h"
//********** End Edit ***************
#include <Uxtheme.h>

#include "Common/Hook.h"
#include "DDrawCompat\DDrawLog.h"
#include "Common/Time.h"
#include "D3dDdi/Hooks.h"
#include "DDraw/DirectDraw.h"
#include "DDraw/Hooks.h"
#include "Direct3d/Hooks.h"
#include "Dll/Procs.h"
#include "Gdi/Gdi.h"
#include "Win32/DisplayMode.h"
#include "Win32/FontSmoothing.h"
#include "Win32/MsgHooks.h"
#include "Win32/Registry.h"
//********** Begin Edit *************
#include "DllMain.h"
#define DllMain DllMain_DDrawCompat
//********** End Edit ***************

namespace CompatExperimental
{
	struct IDirectInput;

	HRESULT WINAPI SetAppCompatData(DWORD, DWORD);

	namespace
	{
		HMODULE g_origDDrawModule = nullptr;
		HMODULE g_origDInputModule = nullptr;

		void installHooks()
		{
			static bool isAlreadyInstalled = false;
			if (!isAlreadyInstalled)
			{
				Win32::DisplayMode::disableDwm8And16BitMitigation();
				Compat::Log() << "Installing registry hooks";
				Win32::Registry::installHooks();
				Compat::Log() << "Installing Direct3D driver hooks";
				D3dDdi::installHooks();
				Compat::Log() << "Installing DirectDraw hooks";
				DDraw::installHooks();
				Compat::Log() << "Installing Direct3D hooks";
				Direct3d::installHooks();
				//********** Begin Edit *************
				if (!Config.DDrawCompatDisableGDIHook)
				{
					Compat::Log() << "Installing GDI hooks";
					Gdi::installHooks();
				}
				//********** End Edit ***************
				Compat::Log() << "Installing display mode hooks";
				Win32::DisplayMode::installHooks(g_origDDrawModule);
				Compat::Log() << "Finished installing hooks";
				isAlreadyInstalled = true;
			}
		}

		bool loadLibrary(const std::string& systemDirectory, const std::string& dllName, HMODULE& module)
		{
			const std::string systemDllPath = systemDirectory + '\\' + dllName;

			module = LoadLibrary(systemDllPath.c_str());
			if (!module)
			{
				Compat::Log() << "Failed to load system " << dllName << " from " << systemDllPath;
				return false;
			}

			return true;
		}

		void printEnvironmentVariable(const char* var)
		{
			const DWORD size = GetEnvironmentVariable(var, nullptr, 0);
			std::string value(size, 0);
			if (!value.empty())
			{
				GetEnvironmentVariable(var, &value.front(), size);
				value.pop_back();
			}
			Compat::Log() << "Environment variable " << var << " = \"" << value << '"';
		}
	}

	//********** Begin Edit *************
#define	LOAD_ORIGINAL_PROC(procName) \
	Dll::g_origProcs.procName = DDrawCompat::procName ## _out;
//********** End Edit ***************

	BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID /*lpvReserved*/)
	{
		if (fdwReason == DLL_PROCESS_ATTACH)
		{
			char currentProcessPath[MAX_PATH] = {};
			GetModuleFileName(nullptr, currentProcessPath, MAX_PATH);
			Compat::Log() << "Process path: " << currentProcessPath;

			//printEnvironmentVariable("__COMPAT_LAYER");

			char currentDllPath[MAX_PATH] = {};
			GetModuleFileName(hinstDLL, currentDllPath, MAX_PATH);
			Compat::Log() << "Loading DDrawCompat from " << currentDllPath;

			char systemDirectory[MAX_PATH] = {};
			GetSystemDirectory(systemDirectory, MAX_PATH);

			std::string systemDDrawDllPath = std::string(systemDirectory) + "\\ddraw.dll";
			if (0 == _stricmp(currentDllPath, systemDDrawDllPath.c_str()))
			{
				Compat::Log() << "DDrawCompat cannot be installed as the system ddraw.dll";
				return FALSE;
			}
			if (!loadLibrary(systemDirectory, "ddraw.dll", g_origDDrawModule) ||
				!loadLibrary(systemDirectory, "dinput.dll", g_origDInputModule))
			{
				return FALSE;
			}

			VISIT_ALL_PROCS(LOAD_ORIGINAL_PROC);
			Dll::g_origProcs.DirectInputCreateA = GetProcAddress(g_origDInputModule, "DirectInputCreateA");

			const BOOL disablePriorityBoost = TRUE;
			SetProcessPriorityBoost(GetCurrentProcess(), disablePriorityBoost);
			//********** Begin Edit *************
			if (!Config.DDrawCompatNoProcAffinity) SetProcessAffinityMask(GetCurrentProcess(), 1);
			//********** End Edit ***************
			timeBeginPeriod(1);
			SetThemeAppProperties(0);

			Compat::redirectIatHooks("ddraw.dll", "DirectDrawCreate",
				Compat::getProcAddress(hinstDLL, "DirectDrawCreate"));
			Compat::redirectIatHooks("ddraw.dll", "DirectDrawCreateEx",
				Compat::getProcAddress(hinstDLL, "DirectDrawCreateEx"));
			Win32::FontSmoothing::g_origSystemSettings = Win32::FontSmoothing::getSystemSettings();
			Win32::MsgHooks::installHooks();
			Time::init();

			//********** Begin Edit *************
			if (Config.DisableMaxWindowedModeNotSet)
			{
				if (Dll::g_origProcs.SetAppCompatData)
				{
					const DWORD disableMaxWindowedMode = 12;
					CALL_ORIG_PROC(SetAppCompatData, disableMaxWindowedMode, 0);
				}
			}
			//********** End Edit ***************
			Compat::Log() << "DDrawCompat Experimental version loaded successfully";
		}
		else if (fdwReason == DLL_PROCESS_DETACH)
		{
			Compat::Log() << "Detaching DDrawCompat";
			DDraw::uninstallHooks();
			D3dDdi::uninstallHooks();
			//********** Begin Edit *************
			if (!Config.DDrawCompatDisableGDIHook)
			{
				Gdi::uninstallHooks();
			}
			//********** End Edit ***************
			Compat::unhookAllFunctions();
			FreeLibrary(g_origDInputModule);
			FreeLibrary(g_origDDrawModule);
			Win32::FontSmoothing::setSystemSettingsForced(Win32::FontSmoothing::g_origSystemSettings);
			timeEndPeriod(1);
			Compat::Log() << "DDrawCompat detached successfully";
		}

		return TRUE;
	}

	extern "C" HRESULT WINAPI _DirectDrawCreate(
		GUID* lpGUID,
		LPDIRECTDRAW* lplpDD,
		IUnknown* pUnkOuter)
	{
		Compat::LogEnter(__func__, lpGUID, lplpDD, pUnkOuter);
		installHooks();
		DDraw::suppressEmulatedDirectDraw(lpGUID);
		HRESULT result = CALL_ORIG_PROC(DirectDrawCreate, lpGUID, lplpDD, pUnkOuter);
		Compat::LogLeave(__func__, lpGUID, lplpDD, pUnkOuter) << result;
		return result;
	}

	extern "C" HRESULT WINAPI _DirectDrawCreateEx(
		GUID* lpGUID,
		LPVOID* lplpDD,
		REFIID iid,
		IUnknown* pUnkOuter)
	{
		Compat::LogEnter(__func__, lpGUID, lplpDD, iid, pUnkOuter);
		installHooks();
		DDraw::suppressEmulatedDirectDraw(lpGUID);
		HRESULT result = CALL_ORIG_PROC(DirectDrawCreateEx, lpGUID, lplpDD, iid, pUnkOuter);
		Compat::LogLeave(__func__, lpGUID, lplpDD, iid, pUnkOuter) << result;
		return result;
	}

	/*extern "C" HRESULT WINAPI DirectInputCreateA(
		HINSTANCE hinst,
		DWORD dwVersion,
		IDirectInput** lplpDirectInput,
		LPUNKNOWN punkOuter)
	{
		Compat::LogEnter(__func__, hinst, dwVersion, lplpDirectInput, punkOuter);
		HRESULT result = CALL_ORIG_PROC(DirectInputCreateA, hinst, dwVersion, lplpDirectInput, punkOuter);
		Compat::LogLeave(__func__, hinst, dwVersion, lplpDirectInput, punkOuter) << result;
		return result;
	}*/

	extern "C" HRESULT WINAPI _DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
	{
		Compat::LogEnter(__func__, rclsid, riid, ppv);
		LOG_ONCE("COM instantiation of DirectDraw detected");
		installHooks();
		HRESULT result = CALL_ORIG_PROC(DllGetClassObject, rclsid, riid, ppv);
		Compat::LogLeave(__func__, rclsid, riid, ppv) << result;
		return result;
	}
}
