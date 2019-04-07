#pragma once

#define VISIT_PROCS_MSACM32(visit) \
visit(XRegThunkEntry, jmpaddr) \
visit(acmDriverAddA, jmpaddr) \
visit(acmDriverAddW, jmpaddr) \
visit(acmDriverClose, jmpaddr) \
visit(acmDriverDetailsA, jmpaddr) \
visit(acmDriverDetailsW, jmpaddr) \
visit(acmDriverEnum, jmpaddr) \
visit(acmDriverID, jmpaddr) \
visit(acmDriverMessage, jmpaddr) \
visit(acmDriverOpen, jmpaddr) \
visit(acmDriverPriority, jmpaddr) \
visit(acmDriverRemove, jmpaddr) \
visit(acmFilterChooseA, jmpaddr) \
visit(acmFilterChooseW, jmpaddr) \
visit(acmFilterDetailsA, jmpaddr) \
visit(acmFilterDetailsW, jmpaddr) \
visit(acmFilterEnumA, jmpaddr) \
visit(acmFilterEnumW, jmpaddr) \
visit(acmFilterTagDetailsA, jmpaddr) \
visit(acmFilterTagDetailsW, jmpaddr) \
visit(acmFilterTagEnumA, jmpaddr) \
visit(acmFilterTagEnumW, jmpaddr) \
visit(acmFormatChooseA, jmpaddr) \
visit(acmFormatChooseW, jmpaddr) \
visit(acmFormatDetailsA, jmpaddr) \
visit(acmFormatDetailsW, jmpaddr) \
visit(acmFormatEnumA, jmpaddr) \
visit(acmFormatEnumW, jmpaddr) \
visit(acmFormatSuggest, jmpaddr) \
visit(acmFormatTagDetailsA, jmpaddr) \
visit(acmFormatTagDetailsW, jmpaddr) \
visit(acmFormatTagEnumA, jmpaddr) \
visit(acmFormatTagEnumW, jmpaddr) \
visit(acmGetVersion, jmpaddr) \
visit(acmMessage32, jmpaddr) \
visit(acmMetrics, jmpaddr) \
visit(acmStreamClose, jmpaddr) \
visit(acmStreamConvert, jmpaddr) \
visit(acmStreamMessage, jmpaddr) \
visit(acmStreamOpen, jmpaddr) \
visit(acmStreamPrepareHeader, jmpaddr) \
visit(acmStreamReset, jmpaddr) \
visit(acmStreamSize, jmpaddr) \
visit(acmStreamUnprepareHeader, jmpaddr)

#ifdef PROC_CLASS
PROC_CLASS(msacm32, dll, VISIT_PROCS_MSACM32, VISIT_PROCS_BLANK)
#endif
