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

#ifdef PROC_CLASS
PROC_CLASS(dsound, dll, VISIT_PROCS_DSOUND)
#endif
