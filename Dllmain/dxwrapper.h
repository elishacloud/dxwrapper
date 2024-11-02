//{{NO_DEPENDENCIES}}
// Microsoft Visual C++ generated include file.
// Used by dxwrapper.rc

#include "Dllmain\BuildNo.rc"

// Main resource file details
#define APP_NAME				"DirectX Dynamic Link Library"
#define APP_MAJOR				1
#define APP_MINOR				2
#define APP_BUILDNUMBER			BUILD_NUMBER
#define APP_REVISION			25
#define APP_COMPANYNAME			"Sadrate Presents"
#define APP_DESCRPTION			"Wraps or hooks DirectX files to fix compatibility issues in older games. Also allows code to be executed from inside the application."
#define APP_COPYRIGHT			"Copyright (C) 2024 Elisha Riedlinger"
#define APP_ORIGINALVERSION		"dxwrapper.dll"
#define APP_INTERNALNAME		"DxWrapper"

// Get APP_VERSION
#define _TO_STRING_(x) #x
#define _TO_STRING(x) _TO_STRING_(x)
#define APP_VERSION _TO_STRING(APP_MAJOR) "." _TO_STRING(APP_MINOR) "." _TO_STRING(APP_BUILDNUMBER) "." _TO_STRING(APP_REVISION)
#define VERSION_NUMBER APP_MAJOR, APP_MINOR, APP_BUILDNUMBER, APP_REVISION

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
