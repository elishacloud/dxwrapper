#pragma once

#define VISIT_PROCS(visit) \
visit(XRegThunkEntry) \
visit(acmDriverAddA) \
visit(acmDriverAddW) \
visit(acmDriverClose) \
visit(acmDriverDetailsA) \
visit(acmDriverDetailsW) \
visit(acmDriverEnum) \
visit(acmDriverID) \
visit(acmDriverMessage) \
visit(acmDriverOpen) \
visit(acmDriverPriority) \
visit(acmDriverRemove) \
visit(acmFilterChooseA) \
visit(acmFilterChooseW) \
visit(acmFilterDetailsA) \
visit(acmFilterDetailsW) \
visit(acmFilterEnumA) \
visit(acmFilterEnumW) \
visit(acmFilterTagDetailsA) \
visit(acmFilterTagDetailsW) \
visit(acmFilterTagEnumA) \
visit(acmFilterTagEnumW) \
visit(acmFormatChooseA) \
visit(acmFormatChooseW) \
visit(acmFormatDetailsA) \
visit(acmFormatDetailsW) \
visit(acmFormatEnumA) \
visit(acmFormatEnumW) \
visit(acmFormatSuggest) \
visit(acmFormatTagDetailsA) \
visit(acmFormatTagDetailsW) \
visit(acmFormatTagEnumA) \
visit(acmFormatTagEnumW) \
visit(acmGetVersion) \
visit(acmMessage32) \
visit(acmMetrics) \
visit(acmStreamClose) \
visit(acmStreamConvert) \
visit(acmStreamMessage) \
visit(acmStreamOpen) \
visit(acmStreamPrepareHeader) \
visit(acmStreamReset) \
visit(acmStreamSize) \
visit(acmStreamUnprepareHeader)

PROC_CLASS(msacm32, dll)

#undef VISIT_PROCS
