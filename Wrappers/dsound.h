#pragma once

#define VISIT_PROCS_DSOUND(visit) \
	visit(DirectSoundCreate, jmpaddr) \
	visit(DirectSoundEnumerateA, jmpaddr) \
	visit(DirectSoundEnumerateW, jmpaddr) \
	visit(DirectSoundCaptureCreate, jmpaddr) \
	visit(DirectSoundCaptureEnumerateA, jmpaddr) \
	visit(DirectSoundCaptureEnumerateW, jmpaddr) \
	visit(GetDeviceID, jmpaddr) \
	visit(DirectSoundFullDuplexCreate, jmpaddr) \
	visit(DirectSoundCreate8, jmpaddr) \
	visit(DirectSoundCaptureCreate8, jmpaddr)

#define VISIT_PROCS_DSOUND_SHARED(visit) \
	visit(DllCanUnloadNow, jmpaddr) \
	visit(DllGetClassObject, jmpaddr)

#define VISIT_SHARED_DSOUND_PROCS(visit) \
	visit(DllCanUnloadNow, DllCanUnloadNow_dsound, jmpaddr) \
	visit(DllGetClassObject, DllGetClassObject_dsound, jmpaddr)

#ifdef PROC_CLASS
PROC_CLASS(dsound, dll, VISIT_PROCS_DSOUND, VISIT_SHARED_DSOUND_PROCS)
#endif
