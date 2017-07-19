#pragma once

#define VISIT_DSOUND_PROCS(visit) \
	visit(DirectSoundCreate) \
	visit(DirectSoundEnumerateA) \
	visit(DirectSoundEnumerateW) \
	visit(DllCanUnloadNow) \
	visit(DllGetClassObject) \
	visit(DirectSoundCaptureCreate) \
	visit(DirectSoundCaptureEnumerateA) \
	visit(DirectSoundCaptureEnumerateW) \
	visit(GetDeviceID) \
	visit(DirectSoundFullDuplexCreate) \
	visit(DirectSoundCreate8) \
	visit(DirectSoundCaptureCreate8)

class dsound_dll
{
public:
	dsound_dll() { };
	~dsound_dll() { };

	void Load();

	void Set_DllCanUnloadNow(FARPROC ProcAddress)
	{
		DllCanUnloadNow = ProcAddress;
	}

	void Set_DllGetClassObject(FARPROC ProcAddress)
	{
		DllGetClassObject = ProcAddress;
	}

	HMODULE dll = nullptr;

private:
	VISIT_DSOUND_PROCS(ADD_FARPROC_MEMBER);
};

extern dsound_dll dsound;
