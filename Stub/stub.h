//{{NO_DEPENDENCIES}}
// Microsoft Visual C++ generated include file.
// Used by stub.rc

#include "..\Dllmain\Resource.h"

// Main resource file details
#define APP_NAME				"DxWrapper Stub"
#define APP_COMPANYNAME			"Sadrate Presents"
#define APP_DESCRPTION			"Stub for DxWrapper. Supports: bcrypt.dll, cryptsp.dll, d2d1.dll, d3d8.dll, d3d9.dll, d3d10.dll, d3d11.dll, d3d12.dll, dciman32.dll, ddraw.dll, dinput.dll, dinput8.dll, dplayx.dll, dsound.dll, msacm32.dll, msvfw32.dll, version.dll, winmm.dll, winmmbase.dll"  // Can only be 256 characters long
#define APP_COPYRIGHT			"Copyright (C) 2024 Elisha Riedlinger"
#define APP_ORIGINALVERSION		"Stub.dll"
#define APP_INTERNALNAME		"DxWrapper Stub"

// Get APP_VERSION
#define _TO_STRING_(x) #x
#define _TO_STRING(x) _TO_STRING_(x)
#define APP_VERSION _TO_STRING(APP_MAJOR) "." _TO_STRING(APP_MINOR) "." _TO_STRING(APP_BUILDNUMBER) "." _TO_STRING(APP_REVISION)
#define VERSION_NUMBER APP_MAJOR, APP_MINOR, APP_BUILDNUMBER, APP_REVISION

// Forward declaration
namespace DdrawWrapper
{
	void Start(const char *name);
}
namespace D3d8Wrapper
{
	void Start(const char *name);
}
namespace DinputWrapper
{
	void Start(const char *name);
}

// Next default values for new objects
// 
#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS
#define _APS_NEXT_RESOURCE_VALUE        101
#define _APS_NEXT_COMMAND_VALUE         40001
#define _APS_NEXT_CONTROL_VALUE         1001
#define _APS_NEXT_SYMED_VALUE           101
#endif
#endif
