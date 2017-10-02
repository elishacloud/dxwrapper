#pragma once

#define VISIT_PROCS(visit) \
	visit(VideoForWindowsVersion) \
	visit(DrawDibBegin) \
	visit(DrawDibChangePalette) \
	visit(DrawDibClose) \
	visit(DrawDibDraw) \
	visit(DrawDibEnd) \
	visit(DrawDibGetBuffer) \
	visit(DrawDibGetPalette) \
	visit(DrawDibOpen) \
	visit(DrawDibProfileDisplay) \
	visit(DrawDibRealize) \
	visit(DrawDibSetPalette) \
	visit(DrawDibStart) \
	visit(DrawDibStop) \
	visit(DrawDibTime) \
	visit(GetOpenFileNamePreview) \
	visit(GetOpenFileNamePreviewA) \
	visit(GetOpenFileNamePreviewW) \
	visit(GetSaveFileNamePreviewA) \
	visit(GetSaveFileNamePreviewW) \
	visit(ICClose) \
	visit(ICCompress) \
	visit(ICCompressorChoose) \
	visit(ICCompressorFree) \
	visit(ICDecompress) \
	visit(ICDraw) \
	visit(ICDrawBegin) \
	visit(ICGetDisplayFormat) \
	visit(ICGetInfo) \
	visit(ICImageCompress) \
	visit(ICImageDecompress) \
	visit(ICInfo) \
	visit(ICInstall) \
	visit(ICLocate) \
	visit(ICMThunk32) \
	visit(ICOpen) \
	visit(ICOpenFunction) \
	visit(ICRemove) \
	visit(ICSendMessage) \
	visit(ICSeqCompressFrame) \
	visit(ICSeqCompressFrameEnd) \
	visit(ICSeqCompressFrameStart) \
	visit(MCIWndCreate) \
	visit(MCIWndCreateA) \
	visit(MCIWndCreateW) \
	visit(MCIWndRegisterClass) \
	visit(StretchDIB)

PROC_CLASS(msvfw32, dll)

#undef VISIT_PROCS
