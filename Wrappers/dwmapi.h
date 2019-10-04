#pragma once

#define VISIT_PROCS_DWMAPI(visit) \
	visit(DwmAttachMilContent, jmpaddr) \
	visit(DwmDefWindowProc, jmpaddr) \
	visit(DwmDetachMilContent, jmpaddr) \
	visit(DwmEnableBlurBehindWindow, jmpaddr) \
	visit(DwmEnableComposition, jmpaddr) \
	visit(DwmEnableMMCSS, jmpaddr) \
	visit(DwmExtendFrameIntoClientArea, jmpaddr) \
	visit(DwmFlush, jmpaddrvoid) \
	visit(DwmGetColorizationColor, jmpaddr) \
	visit(DwmGetCompositionTimingInfo, jmpaddr) \
	visit(DwmGetGraphicsStreamClient, jmpaddr) \
	visit(DwmGetGraphicsStreamTransformHint, jmpaddr) \
	visit(DwmGetTransportAttributes, jmpaddr) \
	visit(DwmGetUnmetTabRequirements, jmpaddr) \
	visit(DwmGetWindowAttribute, jmpaddr) \
	visit(DwmInvalidateIconicBitmaps, jmpaddr) \
	visit(DwmIsCompositionEnabled, jmpaddr) \
	visit(DwmModifyPreviousDxFrameDuration, jmpaddr) \
	visit(DwmQueryThumbnailSourceSize, jmpaddr) \
	visit(DwmRegisterThumbnail, jmpaddr) \
	visit(DwmRenderGesture, jmpaddr) \
	visit(DwmSetDxFrameDuration, jmpaddr) \
	visit(DwmSetIconicLivePreviewBitmap, jmpaddr) \
	visit(DwmSetIconicThumbnail, jmpaddr) \
	visit(DwmSetPresentParameters, jmpaddr) \
	visit(DwmSetWindowAttribute, jmpaddr) \
	visit(DwmShowContact, jmpaddr) \
	visit(DwmTetherContact, jmpaddr) \
	visit(DwmTetherTextContact, jmpaddr) \
	visit(DwmTransitionOwnedWindow, jmpaddr) \
	visit(DwmUnregisterThumbnail, jmpaddr) \
	visit(DwmUpdateThumbnailProperties, jmpaddr) \
	visit(DwmpAllocateSecurityDescriptor, jmpaddr) \
	visit(DwmpDxBindSwapChain, jmpaddr) \
	visit(DwmpDxGetWindowSharedSurface, jmpaddr) \
	visit(DwmpDxUnbindSwapChain, jmpaddr) \
	visit(DwmpDxUpdateWindowRedirectionBltSurface, jmpaddr) \
	visit(DwmpDxUpdateWindowSharedSurface, jmpaddr) \
	visit(DwmpDxgiIsThreadDesktopComposited, jmpaddr) \
	visit(DwmpEnableDDASupport, jmpaddr) \
	visit(DwmpFreeSecurityDescriptor, jmpaddr) \
	visit(DwmpGetColorizationParameters, jmpaddr) \
	visit(DwmpRenderFlick, jmpaddr) \
	visit(DwmpSetColorizationParameters, jmpaddr)

#define VISIT_PROCS_DWMAPI_SHARED(visit) \
	visit(DllCanUnloadNow, jmpaddrvoid) \
	visit(DllGetClassObject, jmpaddr)

#define VISIT_SHARED_DWMAPI_PROCS(visit) \
	visit(DllCanUnloadNow, DllCanUnloadNow_dwmapi, jmpaddrvoid) \
	visit(DllGetClassObject, DllGetClassObject_dwmapi, jmpaddr)

#ifdef PROC_CLASS
PROC_CLASS(dwmapi, dll, VISIT_PROCS_DWMAPI, VISIT_SHARED_DWMAPI_PROCS)
#endif
