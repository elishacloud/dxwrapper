/**
* Created from source code found in DdrawCompat v2.0
* https://github.com/narzoul/DDrawCompat/
*
* Updated 2017 by Elisha Riedlinger
*/

#include <string>

#include "CompatDirectDraw.h"
#include "CompatDirectDrawSurface.h"
#include "CompatDirectDrawPalette.h"
#include "CompatGdi.h"
#include "CompatVtable.h"
#include "DDrawProcs.h"
//********** Begin Edit *************
#include "DllMain.h"
#define DllMain DllMain_DDrawCompat
//********** End Edit ***************

namespace Compat20
{
	struct IDirectInput;

	namespace
	{
		HMODULE g_origDDrawModule = nullptr;
		HMODULE g_origDInputModule = nullptr;

		template <typename CompatInterface>
		void hookVtable(const GUID& guid, IUnknown& unk)
		{
			typename CompatInterface::Interface* intf = nullptr;
			if (SUCCEEDED(unk.lpVtbl->QueryInterface(&unk, guid, reinterpret_cast<LPVOID*>(&intf))))
			{
				CompatInterface::hookVtable(*intf);
				intf->lpVtbl->Release(intf);
			}
		}

		void hookDirectDraw(IDirectDraw& dd)
		{
			IUnknown& ddUnk = reinterpret_cast<IUnknown&>(dd);
			hookVtable<CompatDirectDraw<IDirectDraw>>(IID_IDirectDraw, ddUnk);
			hookVtable<CompatDirectDraw<IDirectDraw2>>(IID_IDirectDraw2, ddUnk);
			hookVtable<CompatDirectDraw<IDirectDraw4>>(IID_IDirectDraw4, ddUnk);
			hookVtable<CompatDirectDraw<IDirectDraw7>>(IID_IDirectDraw7, ddUnk);
		}

		void hookDirectDrawSurface(IDirectDraw& dd)
		{
			DDSURFACEDESC desc = {};
			desc.dwSize = sizeof(desc);
			desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
			desc.dwWidth = 1;
			desc.dwHeight = 1;
			desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;

			IDirectDrawSurface* surface = nullptr;
			HRESULT result = CompatDirectDraw<IDirectDraw>::s_origVtable.CreateSurface(&dd, &desc, &surface, nullptr);
			if (SUCCEEDED(result))
			{
				IUnknown& surfaceUnk = reinterpret_cast<IUnknown&>(*surface);
				hookVtable<CompatDirectDrawSurface<IDirectDrawSurface>>(IID_IDirectDrawSurface, surfaceUnk);
				hookVtable<CompatDirectDrawSurface<IDirectDrawSurface2>>(IID_IDirectDrawSurface2, surfaceUnk);
				hookVtable<CompatDirectDrawSurface<IDirectDrawSurface3>>(IID_IDirectDrawSurface3, surfaceUnk);
				hookVtable<CompatDirectDrawSurface<IDirectDrawSurface4>>(IID_IDirectDrawSurface4, surfaceUnk);
				hookVtable<CompatDirectDrawSurface<IDirectDrawSurface7>>(IID_IDirectDrawSurface7, surfaceUnk);
				surface->lpVtbl->Release(surface);
			}
			else
			{
				Compat::Log() << "Failed to create a DirectDraw surface for hooking: " << result;
			}
		}

		void hookDirectDrawPalette(IDirectDraw& dd)
		{
			PALETTEENTRY paletteEntries[2] = {};
			IDirectDrawPalette* palette = nullptr;
			HRESULT result = CompatDirectDraw<IDirectDraw>::s_origVtable.CreatePalette(
				&dd, DDPCAPS_1BIT, paletteEntries, &palette, nullptr);
			if (SUCCEEDED(result))
			{
				CompatDirectDrawPalette::hookVtable(*palette);
				palette->lpVtbl->Release(palette);
			}
			else
			{
				Compat::Log() << "Failed to create a DirectDraw palette for hooking: " << result;
			}
		}

		void installHooks()
		{
			static bool isAlreadyInstalled = false;
			if (!isAlreadyInstalled)
			{
				Compat::Log() << "Installing DirectDraw hooks";
				IDirectDraw* dd = nullptr;
				HRESULT result = CALL_ORIG_DDRAW(DirectDrawCreate, nullptr, &dd, nullptr);
				if (SUCCEEDED(result))
				{
					dd->lpVtbl->SetCooperativeLevel(dd, nullptr, DDSCL_NORMAL);

					hookDirectDraw(*dd);
					hookDirectDrawSurface(*dd);
					hookDirectDrawPalette(*dd);

					//********** Begin Edit *************
					if (!Config.DDrawCompatDisableGDIHook)
					{
						Compat::Log() << "Installing GDI hooks";
						CompatGdi::installHooks();
					}
					//********** End Edit ***************

					dd->lpVtbl->Release(dd);
				}
				else
				{
					Compat::Log() << "Failed to create a DirectDraw object for hooking" << result;
				}
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

		void suppressEmulatedDirectDraw(GUID*& guid)
		{
			if (reinterpret_cast<GUID*>(DDCREATE_EMULATIONONLY) == guid)
			{
				LOG_ONCE("Warning: suppressed a request to create an emulated DirectDraw object");
				guid = nullptr;
			}
		}
	}

	//********** Begin Edit *************
#define	LOAD_ORIGINAL_DDRAW_PROC(procName) \
	Compat::origProcs.procName = DDrawCompat::procName ## _out;
//********** End Edit ***************

	BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
	{
		UNREFERENCED_PARAMETER(lpvReserved);

		if (fdwReason == DLL_PROCESS_ATTACH)
		{
			char currentDllPath[MAX_PATH] = {};
			GetModuleFileName(hinstDLL, currentDllPath, MAX_PATH);
			Compat::Log() << "Loading DDrawCompat from " << currentDllPath;

			char systemDirectory[MAX_PATH] = {};
			GetSystemDirectory(systemDirectory, MAX_PATH);

			std::string systemDDrawDllPath = std::string(systemDirectory) + "\\ddraw.dll";
			if (0 == _stricmp(currentDllPath, systemDDrawDllPath.c_str()))
			{
				Compat::Log() << "DDrawCompat cannot be installed as the system ddraw.dll";
				return false;
			}

			if (!loadLibrary(systemDirectory, "ddraw.dll", g_origDDrawModule) ||
				!loadLibrary(systemDirectory, "dinput.dll", g_origDInputModule))
			{
				return FALSE;
			}

			VISIT_ALL_DDRAW_PROCS(LOAD_ORIGINAL_DDRAW_PROC);
			Compat::origProcs.DirectInputCreateA = GetProcAddress(g_origDInputModule, "DirectInputCreateA");

			//********** Begin Edit *************
			if (!Config.DDrawCompatNoProcAffinity) SetProcessAffinityMask(GetCurrentProcess(), 1);
			//********** End Edit ***************

			//********** Begin Edit *************
			if (Config.DXPrimaryEmulation[AppCompatDataType.DisableMaxWindowedMode])
			{
				if (Compat::origProcs.SetAppCompatData)
				{
					//typedef HRESULT WINAPI SetAppCompatDataFunc(DWORD, DWORD);
					//auto setAppCompatData = reinterpret_cast<SetAppCompatDataFunc*>(Compat::origProcs.SetAppCompatData);
					//const DWORD disableMaxWindowedMode = 12;
					//setAppCompatData(disableMaxWindowedMode, 0);
				}
			}
			//********** End Edit ***************

			Compat::Log() << "DDrawCompat v0.2.0b loaded successfully";
		}
		else if (fdwReason == DLL_PROCESS_DETACH)
		{
			FreeLibrary(g_origDInputModule);
			FreeLibrary(g_origDDrawModule);
		}

		return TRUE;
	}

	extern "C" HRESULT WINAPI DC20_DirectDrawCreate(
		GUID* lpGUID,
		LPDIRECTDRAW* lplpDD,
		IUnknown* pUnkOuter)
	{
		Compat::LogEnter(__func__, lpGUID, lplpDD, pUnkOuter);
		installHooks();
		suppressEmulatedDirectDraw(lpGUID);
		HRESULT result = CALL_ORIG_DDRAW(DirectDrawCreate, lpGUID, lplpDD, pUnkOuter);
		Compat::LogLeave(__func__, lpGUID, lplpDD, pUnkOuter) << result;
		return result;
	}

	extern "C" HRESULT WINAPI DC20_DirectDrawCreateEx(
		GUID* lpGUID,
		LPVOID* lplpDD,
		REFIID iid,
		IUnknown* pUnkOuter)
	{
		Compat::LogEnter(__func__, lpGUID, lplpDD, iid, pUnkOuter);
		installHooks();
		suppressEmulatedDirectDraw(lpGUID);
		HRESULT result = CALL_ORIG_DDRAW(DirectDrawCreateEx, lpGUID, lplpDD, iid, pUnkOuter);
		Compat::LogLeave(__func__, lpGUID, lplpDD, iid, pUnkOuter) << result;
		return result;
	}

	//********** Begin Edit *************
	extern "C" HRESULT WINAPI DC20_DllGetClassObject(
		REFCLSID rclsid,
		REFIID   riid,
		LPVOID   *ppv)
	{
		Compat::LogEnter(__func__, rclsid, riid, ppv);
		installHooks();
		HRESULT result = CALL_ORIG_DDRAW(DllGetClassObject, rclsid, riid, ppv);
		Compat::LogLeave(__func__, rclsid, riid, ppv) << result;
		return result;
	}
	//********** End Edit ***************

	/*extern "C" HRESULT WINAPI DirectInputCreateA(
		HINSTANCE hinst,
		DWORD dwVersion,
		IDirectInput** lplpDirectInput,
		LPUNKNOWN punkOuter)
	{
		Compat::LogEnter(__func__, hinst, dwVersion, lplpDirectInput, punkOuter);
		HRESULT result = CALL_ORIG_DDRAW(DirectInputCreateA, hinst, dwVersion, lplpDirectInput, punkOuter);
		Compat::LogLeave(__func__, hinst, dwVersion, lplpDirectInput, punkOuter) << result;
		return result;
	}*/
}
