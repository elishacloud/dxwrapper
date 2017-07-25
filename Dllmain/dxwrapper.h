//{{NO_DEPENDENCIES}}
// Microsoft Visual C++ generated include file.
// Used by dxwrapper.rc

#ifndef DDRAWLOG_H
#include "Logging\Logging.h"
#endif
#ifndef DXWND_H
#include "dxwnd.h"
#endif
#include "Dllmain\BuildNo.rc"

// Main resource file details
#define APP_NAME				"DirectX Dynamic Link Library"
#define APP_MAJOR				1
#define APP_MINOR				0
#define APP_BUILDNUMBER			BUILD_NUMBER
#define APP_REVISION			19
#define APP_COMPANYNAME			"Sadrate Presents"
#define APP_DESCRPTION			"Wraps or hooks DirectX files to fix compatibility issues in older games. Also allows code to be executed from inside the application. Wraps the following files: d3d8.dll, d3d9.dll, ddraw.dll, dplayx.dll, dsound.dll, dxgi.dll or winmm.dll"
#define APP_COPYRIGHT			"Copyright (C) 2017 Elisha Riedlinger"
#define APP_ORIGINALVERSION		"dxwrapper.dll"
#define APP_INTERNALNAME		"dxwrapper"

// Get APP_VERSION
#define _TO_STRING_(x) #x
#define _TO_STRING(x) _TO_STRING_(x)
#define APP_VERSION _TO_STRING(APP_MAJOR) "." _TO_STRING(APP_MINOR) "." _TO_STRING(APP_BUILDNUMBER) "." _TO_STRING(APP_REVISION) " " _TO_STRING(APP_DDRAWNAME) " " _TO_STRING(APP_DDRAWVERSION) " " _TO_STRING(APP_DXWNDNAME) " " _TO_STRING(APP_DXWNDVERSION)

//Verify defintions exist in DDrawLog.h
#ifndef APP_DDRAWNAME
#include "Missing APP_DDRAWNAME definition in DDrawLog.h file"
#endif
#ifndef APP_DDRAWVERSION
#include "Missing APP_DDRAWVERSION definition in DDrawLog.h file"
#endif

//Verify defintions exist in dxwnd.h
#ifndef APP_DXWNDNAME
#include "Missing APP_DXWNDNAME definition in dxwnd.h file"
#endif
#ifndef APP_DXWNDVERSION
#include "Missing APP_DXWNDVERSION definition in dxwnd.h file"
#endif

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
