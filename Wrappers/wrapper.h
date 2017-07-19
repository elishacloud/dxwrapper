#pragma once

#define ADD_FARPROC_MEMBER(memberName) FARPROC memberName = jmpaddr;

#define CREATE_PROC_STUB(procName) \
	extern "C" __declspec(naked) void __stdcall Fake ## procName() \
	{ \
		__asm jmp module.procName \
	}

#define	LOAD_ORIGINAL_PROC(procName) \
	procName = GetFunctionAddress(dll, #procName, jmpaddr);

#define jmpaddr (FARPROC)*ReturnProc

HRESULT ReturnProc();

class DllWrapper
{
public:
	DllWrapper() { };
	~DllWrapper() { };

	virtual HMODULE LoadDll(DWORD);
	void DllAttach();
	void DllDetach();

private:
	struct custom_dll
	{
		bool Flag = false;
		HMODULE dll = nullptr;
	};

	custom_dll custom[256];
	custom_dll dllhandle[dtypeArraySize];

	void LoadCustomDll();
	void FreeCustomLibrary();
};

extern DllWrapper Wrapper;
