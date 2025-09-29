/**
* Created from source code found in DdrawCompat v3.0
* https://github.com/narzoul/DDrawCompat/
*
* Updated 2024 by Elisha Riedlinger
*/

#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <string>

#include <Windows.h>
//********** Begin Edit *************
#include "winmm.h"
#include "DllMain.h"
//********** End Edit ***************
#include <ShellScalingApi.h>
#include <timeapi.h>
#include <Uxtheme.h>

#include <DDrawCompat/v0.3.2/Common/Hook.h>
#include <DDrawCompat/DDrawLog.h>
#include <DDrawCompat/v0.3.2/Common/Path.h>
#include <DDrawCompat/v0.3.2/Common/Time.h>
#include <DDrawCompat/v0.3.2/D3dDdi/Hooks.h>
#include <DDrawCompat/v0.3.2/DDraw/DirectDraw.h>
#include <DDrawCompat/v0.3.2/DDraw/Hooks.h>
#include <DDrawCompat/v0.3.2/Direct3d/Hooks.h>
#include <DDrawCompat/v0.3.2/Dll/Dll.h>
#include <DDrawCompat/v0.3.2/Gdi/Gdi.h>
#include <DDrawCompat/v0.3.2/Gdi/PresentationWindow.h>
#include <DDrawCompat/v0.3.2/Gdi/VirtualScreen.h>
#include <DDrawCompat/v0.3.2/Win32/DisplayMode.h>
#include <DDrawCompat/v0.3.2/Win32/MemoryManagement.h>
#include <DDrawCompat/v0.3.2/Win32/MsgHooks.h>
#include <DDrawCompat/v0.3.2/Win32/Version.h>
#include <DDrawCompat/v0.3.2/Win32/Registry.h>
#include <DDrawCompat/v0.3.2/Win32/WaitFunctions.h>
//********** Begin Edit *************
#define DllMain Compat32::DllMain_DDrawCompat

typedef HRESULT(WINAPI* SetProcessDpiAwarenessProc)(PROCESS_DPI_AWARENESS value);
//********** End Edit ***************

HRESULT WINAPI SetAppCompatData(DWORD, DWORD);

namespace
{
	template <typename Result, typename... Params>
	using FuncPtr = Result(WINAPI*)(Params...);

	template <FARPROC(Dll::Procs::* origFunc)>
	const char* getFuncName();

#define DEFINE_FUNC_NAME(func) template <> const char* getFuncName<&Dll::Procs::func>() { return #func; }
	VISIT_PUBLIC_DDRAW_PROCS(DEFINE_FUNC_NAME)
#undef  DEFINE_FUNC_NAME

	void installHooks();

	template <FARPROC(Dll::Procs::* origFunc), typename OrigFuncPtrType, typename FirstParam, typename... Params>
	HRESULT WINAPI directDrawFunc(FirstParam firstParam, Params... params)
	{
		LOG_FUNC(getFuncName<origFunc>(), firstParam, params...);
		installHooks();
		suppressEmulatedDirectDraw(firstParam);
		return LOG_RESULT(reinterpret_cast<OrigFuncPtrType>(Dll::g_origProcs.*origFunc)(firstParam, params...));
	}

	void installHooks()
	{
		static bool isAlreadyInstalled = false;
		if (!isAlreadyInstalled)
		{
			Compat32::Log() << "Installing display mode hooks";
			Win32::DisplayMode::installHooks();
			Compat32::Log() << "Installing registry hooks";
			Win32::Registry::installHooks();
			Compat32::Log() << "Installing Direct3D driver hooks";
			D3dDdi::installHooks();
			Gdi::VirtualScreen::init();

			CompatPtr<IDirectDraw> dd;
			CALL_ORIG_PROC(DirectDrawCreate)(nullptr, &dd.getRef(), nullptr);
			CompatPtr<IDirectDraw7> dd7;
			CALL_ORIG_PROC(DirectDrawCreateEx)(nullptr, reinterpret_cast<void**>(&dd7.getRef()), IID_IDirectDraw7, nullptr);
			if (!dd || !dd7)
			{
				Compat32::Log() << "ERROR: Failed to create a DirectDraw object for hooking";
				return;
			}

			CompatVtable<IDirectDrawVtbl>::s_origVtable = *dd.get()->lpVtbl;
			HRESULT result = dd->SetCooperativeLevel(dd, nullptr, DDSCL_NORMAL);
			if (SUCCEEDED(result))
			{
				CompatVtable<IDirectDraw7Vtbl>::s_origVtable = *dd7.get()->lpVtbl;
				dd7->SetCooperativeLevel(dd7, nullptr, DDSCL_NORMAL);
			}
			if (FAILED(result))
			{
				Compat32::Log() << "ERROR: Failed to set the cooperative level for hooking: " << Compat32::hex(result);
				return;
			}

			Compat32::Log() << "Installing DirectDraw hooks";
			DDraw::installHooks(dd7);
			Compat32::Log() << "Installing Direct3D hooks";
			Direct3d::installHooks(dd, dd7);

			//********** Begin Edit *************
			if (!Config.DDrawCompatDisableGDIHook)
			{
				Compat32::Log() << "Installing GDI hooks";
				Gdi::installHooks();
			}
			//********** End Edit ***************

			Gdi::PresentationWindow::startThread();
			Compat32::Log() << "Finished installing hooks";
			isAlreadyInstalled = true;
		}
	}

	bool isOtherDDrawWrapperLoaded()
	{
		const auto currentDllPath(Compat32::getModulePath(Dll::g_currentModule));
		const auto ddrawDllPath(Compat32::replaceFilename(currentDllPath, L"ddraw.dll"));
		const auto dciman32DllPath(Compat32::replaceFilename(currentDllPath, L"dciman32.dll"));

		return (!Compat32::isEqual(currentDllPath, ddrawDllPath) && GetModuleHandleW(ddrawDllPath.c_str())) ||
			(!Compat32::isEqual(currentDllPath, dciman32DllPath) && GetModuleHandleW(dciman32DllPath.c_str()));
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
		Compat32::Log() << "Environment variable " << var << " = \"" << value << '"';
	}

	void setDpiAwareness()
	{
		HMODULE shcore = LoadLibrary("shcore");
		if (shcore)
		{
			auto setProcessDpiAwareness = reinterpret_cast<SetProcessDpiAwarenessProc>(
				Compat32::getProcAddress(shcore, "SetProcessDpiAwareness"));
			if (setProcessDpiAwareness && SUCCEEDED(setProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE)))
			{
				return;
			}
		}
		SetProcessDPIAware();
	}

	template <typename Param>
	void suppressEmulatedDirectDraw(Param)
	{
	}

	void suppressEmulatedDirectDraw(GUID*& guid)
	{
		DDraw::DirectDraw::suppressEmulatedDirectDraw(guid);
	}
}

#define	LOAD_ORIG_PROC(procName) \
	Dll::g_origProcs.procName = Compat32::getProcAddress(origModule, #procName);

#define HOOK_DDRAW_PROC(procName) \
	Compat32::hookFunction( \
		reinterpret_cast<void*&>(Dll::g_origProcs.procName), \
		static_cast<decltype(&procName)>(&directDrawFunc<&Dll::Procs::procName, decltype(&procName)>), \
		#procName);

//********** Begin Edit *************
#define INITIALIZE_ALL_WRAPPED_PROC(procName) \
	FARPROC Compat32::procName ## _proc = nullptr;

VISIT_ALL_DDRAW_PROCS(INITIALIZE_ALL_WRAPPED_PROC);

#define SET_ALL_WRAPPED_PROC(procName) \
	procName ## _proc = ddraw::procName ## _var;

#define SET_PUBLIC_WRAPPED_PROC(procName) \
	procName ## _proc = (FARPROC)static_cast<decltype(&procName)>(&directDrawFunc<&Dll::Procs::procName, decltype(&procName)>);

void Compat32::Prepair_DDrawCompat()
{
	VISIT_ALL_DDRAW_PROCS(SET_ALL_WRAPPED_PROC);
	VISIT_PUBLIC_DDRAW_PROCS(SET_PUBLIC_WRAPPED_PROC);
}

#define	LOAD_WRAPPED_PROC(procName) \
	Dll::g_origProcs.procName = DDrawCompat::procName ## _out;

void Compat32::InstallDd7to9Hooks(HMODULE hModule)
{
	static bool RunOnce = true;
	if (RunOnce && !DDrawCompat::IsEnabled())
	{
		RunOnce = false;
		Dll::g_currentModule = hModule;
		auto systemPath(Compat32::getSystemPath());
		Dll::g_origDDrawModule = LoadLibraryA("d3d9.dll");	// Just use d3d9 for this
		auto currentDllPath(Compat32::getModulePath(hModule));
		Dll::g_localDDModule = LoadLibraryW(std::wstring(getParentPath(currentDllPath) + L"\\ddraw.dll").c_str());
		if (!Dll::g_localDDModule)
		{
			Dll::g_localDDModule = LoadLibraryA("ddraw.dll");
		}
		if (!Dll::g_localDDModule)
		{
			Dll::g_localDDModule = Dll::g_currentModule;
		}
		Time::init();
		Compat32::Log() << "Installing memory management hooks";
		Win32::MemoryManagement::installHooks();
		Compat32::Log() << "Installing messaging hooks";
		Win32::MsgHooks::installHooks();
		Compat32::Log() << "Installing version hooks";
		Win32::Version::installHooks();
		Compat32::Log() << "Installing display mode hooks";
		Win32::DisplayMode::installHooks();
		Compat32::Log() << "Installing registry hooks";
		Win32::Registry::installHooks();
		Compat32::Log() << "Installing Win32 hooks";
		Win32::WaitFunctions::installHooks();
		DisableProcessWindowsGhosting();
	}
}
//********** End Edit ***************

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	UNREFERENCED_PARAMETER(lpvReserved);

	//********** Begin Edit *************
	//static bool skipDllMain = false;
	//if (skipDllMain)
	//{
	//	return TRUE;
	//}
	//********** End Edit ***************

	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		Dll::g_currentModule = hinstDLL;
		//********** Begin Edit *************
		//if (isOtherDDrawWrapperLoaded())
		//{
		//	skipDllMain = true;
		//	return TRUE;
		//}
		//********** End Edit ***************

		auto processPath(Compat32::getModulePath(nullptr));
		//********** Begin Edit *************
		//Compat32::Log::initLogging(processPath);
		//********** End Edit ***************

		Compat32::Log() << "Process path: " << processPath.c_str();
		//********** Begin Edit *************
		//printEnvironmentVariable("__COMPAT_LAYER");
		//********** End Edit ***************
		auto currentDllPath(Compat32::getModulePath(hinstDLL));
		//********** Begin Edit *************
		//Compat32::Log() << "Loading DDrawCompat " << (lpvReserved ? "statically" : "dynamically") << " from " << currentDllPath.u8string();
		//********** End Edit ***************

		auto systemPath(Compat32::getSystemPath());
		if (Compat32::isEqual(getParentPath(currentDllPath), systemPath))
		{
			Compat32::Log() << "DDrawCompat cannot be installed in the Windows system directory";
			return FALSE;
		}

		Dll::g_origDDrawModule = LoadLibraryW(std::wstring(systemPath + L"\\ddraw.dll").c_str());
		if (!Dll::g_origDDrawModule)
		{
			Compat32::Log() << "ERROR: Failed to load system ddraw.dll from " << systemPath.c_str();
			return FALSE;
		}
		Dll::g_localDDModule = LoadLibraryW(std::wstring(getParentPath(currentDllPath) + L"\\ddraw.dll").c_str());
		if (!Dll::g_localDDModule)
		{
			Dll::g_localDDModule = Dll::g_origDDrawModule;
		}

		Dll::pinModule(Dll::g_origDDrawModule);
		Dll::pinModule(Dll::g_currentModule);

		HMODULE origModule = Dll::g_origDDrawModule;
		//********** Begin Edit *************
		VISIT_ALL_DDRAW_PROCS(LOAD_WRAPPED_PROC);
		//********** End Edit ***************

		Dll::g_origDciman32Module = LoadLibraryW(std::wstring(systemPath + L"\\dciman32.dll").c_str());
		if (Dll::g_origDciman32Module)
		{
			origModule = Dll::g_origDciman32Module;
			VISIT_DCIMAN32_PROCS(LOAD_ORIG_PROC);
		}

		Dll::g_jmpTargetProcs = Dll::g_origProcs;

		//********** Begin Edit *************
		//VISIT_PUBLIC_DDRAW_PROCS(HOOK_DDRAW_PROC);
		//********** End Edit ***************

		const BOOL disablePriorityBoost = TRUE;
		SetProcessPriorityBoost(GetCurrentProcess(), disablePriorityBoost);
		//********** Begin Edit *************
		if (!Config.DDrawCompatNoProcAffinity)
		{
			SetProcessAffinityMask(GetCurrentProcess(), 1);
		}
		//********** End Edit ***************
		timeBeginPeriod(1);
		setDpiAwareness();
		SetThemeAppProperties(0);

		if (!Config.Dd7to9)
		{
			Win32::MemoryManagement::installHooks();
			Win32::MsgHooks::installHooks();
			Win32::Version::installHooks();
			Time::init();
			Compat32::Log() << "Installing Win32 hooks";
			Win32::WaitFunctions::installHooks();
		}

		const DWORD disableMaxWindowedMode = 12;
		CALL_ORIG_PROC(SetAppCompatData)(disableMaxWindowedMode, 0);

		Compat32::Log() << "DDrawCompat v0.3.2 version loaded successfully";
	}
	else if (fdwReason == DLL_PROCESS_DETACH)
	{
		Compat32::Log() << "DDrawCompat detached successfully";
	}
	else if (fdwReason == DLL_THREAD_DETACH)
	{
		Gdi::dllThreadDetach();
	}

	return TRUE;
}
