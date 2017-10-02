#pragma once

#define VISIT_PROCS(visit) \
	visit(DirectSoundCreate) \
	visit(DirectSoundEnumerateA) \
	visit(DirectSoundEnumerateW) \
	visit(DirectSoundCaptureCreate) \
	visit(DirectSoundCaptureEnumerateA) \
	visit(DirectSoundCaptureEnumerateW) \
	visit(GetDeviceID) \
	visit(DirectSoundFullDuplexCreate) \
	visit(DirectSoundCreate8) \
	visit(DirectSoundCaptureCreate8)

PROC_CLASS(dsound, dll)

#undef VISIT_PROCS
