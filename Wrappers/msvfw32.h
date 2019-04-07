#pragma once

#define VISIT_PROCS_MSVFW32(visit) \
	visit(VideoForWindowsVersion, jmpaddr) \
	visit(DrawDibBegin, jmpaddr) \
	visit(DrawDibChangePalette, jmpaddr) \
	visit(DrawDibClose, jmpaddr) \
	visit(DrawDibDraw, jmpaddr) \
	visit(DrawDibEnd, jmpaddr) \
	visit(DrawDibGetBuffer, jmpaddr) \
	visit(DrawDibGetPalette, jmpaddr) \
	visit(DrawDibOpen, jmpaddr) \
	visit(DrawDibProfileDisplay, jmpaddr) \
	visit(DrawDibRealize, jmpaddr) \
	visit(DrawDibSetPalette, jmpaddr) \
	visit(DrawDibStart, jmpaddr) \
	visit(DrawDibStop, jmpaddr) \
	visit(DrawDibTime, jmpaddr) \
	visit(GetOpenFileNamePreview, jmpaddr) \
	visit(GetOpenFileNamePreviewA, jmpaddr) \
	visit(GetOpenFileNamePreviewW, jmpaddr) \
	visit(GetSaveFileNamePreviewA, jmpaddr) \
	visit(GetSaveFileNamePreviewW, jmpaddr) \
	visit(ICClose, jmpaddr) \
	visit(ICCompress, jmpaddr) \
	visit(ICCompressorChoose, jmpaddr) \
	visit(ICCompressorFree, jmpaddr) \
	visit(ICDecompress, jmpaddr) \
	visit(ICDraw, jmpaddr) \
	visit(ICDrawBegin, jmpaddr) \
	visit(ICGetDisplayFormat, jmpaddr) \
	visit(ICGetInfo, jmpaddr) \
	visit(ICImageCompress, jmpaddr) \
	visit(ICImageDecompress, jmpaddr) \
	visit(ICInfo, jmpaddr) \
	visit(ICInstall, jmpaddr) \
	visit(ICLocate, jmpaddr) \
	visit(ICMThunk32, jmpaddr) \
	visit(ICOpen, jmpaddr) \
	visit(ICOpenFunction, jmpaddr) \
	visit(ICRemove, jmpaddr) \
	visit(ICSendMessage, jmpaddr) \
	visit(ICSeqCompressFrame, jmpaddr) \
	visit(ICSeqCompressFrameEnd, jmpaddr) \
	visit(ICSeqCompressFrameStart, jmpaddr) \
	visit(MCIWndCreate, jmpaddr) \
	visit(MCIWndCreateA, jmpaddr) \
	visit(MCIWndCreateW, jmpaddr) \
	visit(MCIWndRegisterClass, jmpaddr) \
	visit(StretchDIB, jmpaddr)

#ifdef PROC_CLASS
PROC_CLASS(msvfw32, dll, VISIT_PROCS_MSVFW32, VISIT_PROCS_BLANK)
#endif
