#pragma once

#define VISIT_PROCS(visit) \
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

PROC_CLASS(dsound, dll)

#undef VISIT_PROCS
