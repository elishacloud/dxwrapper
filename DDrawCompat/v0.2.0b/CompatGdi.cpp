#include "CompatDirectDrawSurface.h"
#include "CompatGdi.h"
#include "CompatGdiCaret.h"
#include "CompatGdiDcCache.h"
#include "CompatGdiFunctions.h"
#include "CompatGdiWinProc.h"
#include "CompatPrimarySurface.h"
#include "DDrawProcs.h"
#include "RealPrimarySurface.h"

namespace Compat20
{
	namespace
	{
		DWORD g_renderingDepth = 0;

		FARPROC getProcAddress(HMODULE module, const char* procName)
		{
			if (!module || !procName)
			{
				return nullptr;
			}

			PIMAGE_DOS_HEADER dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(module);
			if (IMAGE_DOS_SIGNATURE != dosHeader->e_magic) {
				return nullptr;
			}
			char* moduleBase = reinterpret_cast<char*>(module);

			PIMAGE_NT_HEADERS ntHeader = reinterpret_cast<PIMAGE_NT_HEADERS>(
				reinterpret_cast<char*>(dosHeader) + dosHeader->e_lfanew);
			if (IMAGE_NT_SIGNATURE != ntHeader->Signature)
			{
				return nullptr;
			}

			PIMAGE_EXPORT_DIRECTORY exportDir = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(
				moduleBase + ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

			DWORD* rvaOfNames = reinterpret_cast<DWORD*>(moduleBase + exportDir->AddressOfNames);

			for (DWORD i = 0; i < exportDir->NumberOfNames; ++i)
			{
				if (0 == strcmp(procName, moduleBase + rvaOfNames[i]))
				{
					WORD* nameOrds = reinterpret_cast<WORD*>(moduleBase + exportDir->AddressOfNameOrdinals);
					DWORD* rvaOfFunctions = reinterpret_cast<DWORD*>(moduleBase + exportDir->AddressOfFunctions);
					return reinterpret_cast<FARPROC>(moduleBase + rvaOfFunctions[nameOrds[i]]);
				}
			}

			return nullptr;
		}

		BOOL CALLBACK invalidateWindow(HWND hwnd, LPARAM /*lParam*/)
		{
			DWORD processId = 0;
			GetWindowThreadProcessId(hwnd, &processId);
			if (processId == GetCurrentProcessId())
			{
				RedrawWindow(hwnd, nullptr, nullptr, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
			}
			return TRUE;
		}
	}

	namespace CompatGdi
	{
		CRITICAL_SECTION g_gdiCriticalSection;
		std::unordered_map<void*, const char*> g_funcNames;

		GdiScopedThreadLock::GdiScopedThreadLock()
		{
			EnterCriticalSection(&g_gdiCriticalSection);
		}

		GdiScopedThreadLock::~GdiScopedThreadLock()
		{
			LeaveCriticalSection(&g_gdiCriticalSection);
		}

		bool beginGdiRendering()
		{
			if (!RealPrimarySurface::isFullScreen())
			{
				return false;
			}

			Compat::origProcs.AcquireDDThreadLock();
			EnterCriticalSection(&g_gdiCriticalSection);

			if (0 == g_renderingDepth)
			{
				DDSURFACEDESC2 desc = {};
				desc.dwSize = sizeof(desc);
				if (FAILED(CompatDirectDrawSurface<IDirectDrawSurface7>::s_origVtable.Lock(
					CompatPrimarySurface::surface, nullptr, &desc, DDLOCK_WAIT, nullptr)))
				{
					LeaveCriticalSection(&g_gdiCriticalSection);
					Compat::origProcs.ReleaseDDThreadLock();
					return false;
				}
				CompatGdiDcCache::setSurfaceMemory(desc.lpSurface, desc.lPitch);
			}

			++g_renderingDepth;
			return true;
		}

		void endGdiRendering()
		{
			--g_renderingDepth;
			if (0 == g_renderingDepth)
			{
				GdiFlush();
				CompatDirectDrawSurface<IDirectDrawSurface7>::s_origVtable.Unlock(
					CompatPrimarySurface::surface, nullptr);
				RealPrimarySurface::update();
			}

			LeaveCriticalSection(&g_gdiCriticalSection);
			Compat::origProcs.ReleaseDDThreadLock();
		}

		void hookGdiFunction(const char* moduleName, const char* funcName, void*& origFuncPtr, void* newFuncPtr)
		{
#ifdef _DEBUG
			g_funcNames[origFuncPtr] = funcName;
#endif

			FARPROC procAddr = getProcAddress(GetModuleHandle(moduleName), funcName);
			if (!procAddr)
			{
				Compat::Log() << "Failed to load the address of a GDI function: " << funcName;
				return;
			}

			origFuncPtr = procAddr;
			if (NO_ERROR != DetourAttach(&origFuncPtr, newFuncPtr))
			{
				Compat::Log() << "Failed to hook a GDI function: " << funcName;
				return;
			}
		}

		void installHooks()
		{
			InitializeCriticalSection(&g_gdiCriticalSection);
			if (CompatGdiDcCache::init())
			{
				CompatGdiFunctions::installHooks();
				CompatGdiWinProc::installHooks();
				CompatGdiCaret::installHooks();
			}
		}

		void invalidate()
		{
			EnumWindows(&invalidateWindow, 0);
		}

		void updatePalette()
		{
			GdiScopedThreadLock gdiLock;
			CompatGdiDcCache::clear();
		}
	}
}
