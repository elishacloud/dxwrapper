#pragma once

#define VISIT_DINPUT_PROCS(visit) \
	visit(DirectInputCreateA) \
	visit(DirectInputCreateEx) \
	visit(DirectInputCreateW) \
	visit(DllRegisterServer) \
	visit(DllUnregisterServer) \
	//visit(DllCanUnloadNow) \		 // <---  Shared with dsound.dll
	//visit(DllGetClassObject)		 // <---  Shared with dsound.dll

class dinput_dll
{
public:
	dinput_dll() { };
	~dinput_dll() { };

	void Load();

	HMODULE dll = nullptr;

private:
	VISIT_DINPUT_PROCS(ADD_FARPROC_MEMBER);
};

extern dinput_dll dinput;
