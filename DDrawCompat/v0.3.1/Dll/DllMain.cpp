/**
* Created from source code found in DdrawCompat v3.0
* https://github.com/narzoul/DDrawCompat/
*
* Updated 2021 by Elisha Riedlinger
*/

#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <string>

#include <Windows.h>
//********** Begin Edit *************
#include "winmm.h"
#include "DllMain.h"
#include "Dllmain\Dllmain.h"
//********** End Edit ***************
#include <timeapi.h>
#include <ShellScalingApi.h>
#include <Uxtheme.h>

#include <DDrawCompat/v0.3.1/Common/Hook.h>
#include <DDrawCompat/DDrawLog.h>
#include <DDrawCompat/v0.3.1/Common/Path.h>
#include <DDrawCompat/v0.3.1/Common/Time.h>
#include <DDrawCompat/v0.3.1/D3dDdi/Hooks.h>
#include <DDrawCompat/v0.3.1/DDraw/DirectDraw.h>
#include <DDrawCompat/v0.3.1/DDraw/Hooks.h>
#include <DDrawCompat/v0.3.1/Direct3d/Hooks.h>
#include <DDrawCompat/v0.3.1/Dll/Dll.h>
#include <DDrawCompat/v0.3.1/Gdi/Gdi.h>
#include <DDrawCompat/v0.3.1/Gdi/PresentationWindow.h>
#include <DDrawCompat/v0.3.1/Gdi/VirtualScreen.h>
#include <DDrawCompat/v0.3.1/Win32/DisplayMode.h>
#include <DDrawCompat/v0.3.1/Win32/MemoryManagement.h>
#include <DDrawCompat/v0.3.1/Win32/MsgHooks.h>
#include <DDrawCompat/v0.3.1/Win32/Registry.h>
#include <DDrawCompat/v0.3.1/Win32/WaitFunctions.h>
//********** Begin Edit *************
#define DllMain DllMain_DDrawCompat
//********** End Edit ***************

HRESULT WINAPI SetAppCompatData(DWORD, DWORD);

namespace Compat32
{
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
				Compat32::Log() << "Installing Win32 hooks";
				Win32::WaitFunctions::installHooks();

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

				Compat32::closeDbgEng();
				Gdi::PresentationWindow::startThread();
				Compat32::Log() << "Finished installing hooks";
				isAlreadyInstalled = true;
			}
		}

		bool isOtherDDrawWrapperLoaded()
		{
			const auto currentDllPath(Compat32::getModulePath(Dll::g_currentModule));
			const auto ddrawDllPath(Compat32::replaceFilename(currentDllPath, "ddraw.dll"));
			const auto dciman32DllPath(Compat32::replaceFilename(currentDllPath, "dciman32.dll"));

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
				auto setProcessDpiAwareness = reinterpret_cast<decltype(&SetProcessDpiAwareness)>(
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

	//********** Begin Edit *************
#define INITIALIZE_PUBLIC_WRAPPED_PROC(proc) \
	FARPROC proc ## _proc = (FARPROC)static_cast<decltype(&proc)>(&directDrawFunc<&Dll::Procs::proc, decltype(&proc)>);

#define INITIALIZE_PRIVATE_WRAPPED_PROC(proc) \
	FARPROC proc ## _proc = (FARPROC)*DC31_ ## proc;

	VISIT_PUBLIC_DDRAW_PROCS(INITIALIZE_PUBLIC_WRAPPED_PROC);
	VISIT_PRIVATE_DDRAW_PROCS(INITIALIZE_PRIVATE_WRAPPED_PROC);

#define	LOAD_NEW_PROC(proc) \
	Dll::g_origProcs.proc = DDrawCompat::proc ## _out;
	//********** End Edit ***************

#define	LOAD_ORIG_PROC(proc) \
	Dll::g_origProcs.proc = Compat32::getProcAddress(origModule, #proc);

#define HOOK_DDRAW_PROC(proc) \
	Compat32::hookFunction( \
		reinterpret_cast<void*&>(Dll::g_origProcs.proc), \
		static_cast<decltype(&proc)>(&directDrawFunc<&Dll::Procs::proc, decltype(&proc)>), \
		#proc);

	//********** Begin Edit *************
	void InstallDd7to9Hooks()
	{
		static bool RunOnce = true;
		if (RunOnce && !DDrawCompat::IsEnabled())
		{
			RunOnce = false;
			Time::init();
			Compat32::Log() << "Installing memory management hooks";
			Win32::MemoryManagement::installHooks();
			Compat32::Log() << "Installing messaging hooks";
			Win32::MsgHooks::installHooks();
			Compat32::Log() << "Installing display mode hooks";
			Win32::DisplayMode::installHooks();
			if (Config.DDrawCompat)
			{
				Compat32::Log() << "Installing Direct3D driver hooks";
				D3dDdi::installHooks();
			}
			Compat32::Log() << "Installing registry hooks";
			Win32::Registry::installHooks();
			Compat32::Log() << "Installing Win32 hooks";
			Win32::WaitFunctions::installHooks();
			if (Config.DDrawCompat && !Config.DDrawCompatDisableGDIHook)
			{
				Compat32::Log() << "Installing GDI hooks";
				Gdi::installHooks();
			}
			Compat32::closeDbgEng();
		}
	}
	//********** End Edit ***************

	BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
	{
		static bool skipDllMain = false;
		if (skipDllMain)
		{
			skipDllMain = true;
			return TRUE;
		}

		if (fdwReason == DLL_PROCESS_ATTACH)
		{
			Dll::g_currentModule = hinstDLL;
			//********** Begin Edit *************
			/*if (isOtherDDrawWrapperLoaded())
			{
				skipDllMain = true;
				return TRUE;
			}*/
			//********** End Edit ***************

			auto processPath(Compat32::getModulePath(nullptr));
			//********** Begin Edit *************
			//Compat32::Log::initLogging(processPath);
			//********** End Edit ***************

			Compat32::Log() << "Process path: " << processPath.u8string();
			//********** Begin Edit *************
			//printEnvironmentVariable("__COMPAT_LAYER");
			//********** End Edit ***************
			auto currentDllPath(Compat32::getModulePath(hinstDLL));
			Compat32::Log() << "Loading DDrawCompat " << (lpvReserved ? "statically" : "dynamically") << " from " << currentDllPath.u8string();

			auto systemPath(Compat32::getSystemPath());
			if (Compat32::isEqual(currentDllPath.parent_path(), systemPath))
			{
				Compat32::Log() << "DDrawCompat cannot be installed in the Windows system directory";
				return FALSE;
			}

			Dll::g_origDDrawModule = LoadLibraryW((systemPath / "ddraw.dll").c_str());

			if (!Dll::g_origDDrawModule)
			{
				Compat::Log() << "ERROR: Failed to load system ddraw.dll from " << systemPath.u8string();
				return FALSE;
			}

			Dll::pinModule(Dll::g_origDDrawModule);
			Dll::pinModule(Dll::g_currentModule);

			HMODULE origModule = Dll::g_origDDrawModule;
			//********** Begin Edit *************
			HMODULE hModule = nullptr;
			GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCTSTR)DDrawCompat::DirectDrawCreate_out, &hModule);
			if (hModule == Dll::g_origDDrawModule)
			{
				VISIT_DDRAW_PROCS(LOAD_NEW_PROC);
			}
			else
			{
				VISIT_DDRAW_PROCS(LOAD_ORIG_PROC);
			}
			//********** End Edit ***************

			Dll::g_origDciman32Module = LoadLibraryW((systemPath / "dciman32.dll").c_str());
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
			if (!Config.DDrawCompatNoProcAffinity) SetProcessAffinityMask(GetCurrentProcess(), 1);
			//********** End Edit ***************

			timeBeginPeriod(1);
			setDpiAwareness();
			SetThemeAppProperties(0);
			if (!Config.Dd7to9)
			{
				Win32::MemoryManagement::installHooks();
				Win32::MsgHooks::installHooks();
				Time::init();
				Compat32::closeDbgEng();
			}

			//********** Begin Edit *************
			if (Config.DisableMaxWindowedModeNotSet)
			{
				const DWORD disableMaxWindowedMode = 12;
				CALL_ORIG_PROC(SetAppCompatData)(disableMaxWindowedMode, 0);
			}
			//********** End Edit ***************

			Compat32::Log() << "DDrawCompat v0.3.1 version loaded successfully";
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
}