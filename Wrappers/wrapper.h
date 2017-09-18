#pragma once

#define VISIT_WRAPPERS(visit) \
	visit(bcrypt, BCRYPT) \
	visit(cryptsp, CRYTPSP) \
	visit(d3d8, D3D8) \
	visit(d3d9, D3D9) \
	visit(ddraw, DDRAW) \
	visit(dinput, DINPUT) \
	visit(dplayx, DPLAYX) \
	visit(dsound, DSOUND) \
	visit(dxgi, DXGI) \
	visit(winmm, WINMM) \
	visit(winspool, WINSPOOL) \
	visit(dciman32, DCIMAN32)

#define ADD_FARPROC_MEMBER(memberName) FARPROC memberName = jmpaddr;

#define ADD_NAMESPACE_CLASS(className, Z) className::className ## _dll className::module;

#define CREATE_ALL_PROC_STUB(module, MACRO) \
	namespace my ## module \
	{ \
		using namespace module; \
		VISIT_ ## MACRO ## _PROCS(CREATE_PROC_STUB); \
	}

#define CREATE_PROC_STUB(procName) \
	extern "C" __declspec(naked) void __stdcall Fake ## procName() \
	{ \
		__asm jmp module.procName \
	}

#define	LOAD_ORIGINAL_PROC(procName) \
	procName = GetProcAddress(dll, #procName, jmpaddr);

#define	LOAD_WRAPPER(className, Z) \
		className::module.Load();

#define	UNHOOK_WRAPPER(className, Z) \
		className::module.Unhook();

namespace Wrapper
{
	__declspec(naked) static HRESULT __stdcall ReturnProc()
	{
		__asm {
			mov eax, 0x80004001L	// return E_NOTIMPL
			retn 16
		}
	}

	constexpr FARPROC jmpaddr = (FARPROC)*ReturnProc;

	HMODULE LoadDll(DWORD);
	void DllAttach();
	void DllDetach();
	FARPROC GetProcAddress(HMODULE, LPCSTR, FARPROC);

	namespace d3d9_Wrapper
	{
		extern FARPROC _Direct3DCreate9_RealProc;
		extern FARPROC _Direct3DCreate9_WrapperProc;
	}

	namespace D3d8to9
	{
		extern FARPROC _Direct3DCreate8;
		extern FARPROC _Direct3DCreate9;
	}

	namespace DDrawCompat
	{
		extern FARPROC _DirectDrawCreate;
		extern FARPROC _DirectDrawCreateEx;
		extern FARPROC _DllGetClassObject;
	}

	namespace DSoundCtrl
	{
		extern FARPROC _DirectSoundCreate;
		extern FARPROC _DirectSoundEnumerateA;
		extern FARPROC _DirectSoundEnumerateW;
		extern FARPROC _DllCanUnloadNow;
		extern FARPROC _DllGetClassObject;
		extern FARPROC _DirectSoundCaptureCreate;
		extern FARPROC _DirectSoundCaptureEnumerateA;
		extern FARPROC _DirectSoundCaptureEnumerateW;
		extern FARPROC _GetDeviceID;
		extern FARPROC _DirectSoundFullDuplexCreate;
		extern FARPROC _DirectSoundCreate8;
		extern FARPROC _DirectSoundCaptureCreate8;
	}
};
