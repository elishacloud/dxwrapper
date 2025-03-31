/**
* Copyright (C) 2025 Elisha Riedlinger
*
* This software is  provided 'as-is', without any express  or implied  warranty. In no event will the
* authors be held liable for any damages arising from the use of this software.
* Permission  is granted  to anyone  to use  this software  for  any  purpose,  including  commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*   1. The origin of this software must not be misrepresented; you must not claim that you  wrote the
*      original  software. If you use this  software  in a product, an  acknowledgment in the product
*      documentation would be appreciated but is not required.
*   2. Altered source versions must  be plainly  marked as such, and  must not be  misrepresented  as
*      being the original software.
*   3. This notice may not be removed or altered from any source distribution.
*/

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "ddraw\ddrawExternal.h"
#include "GDI.h"
#include "Settings\Settings.h"
#include "Logging\Logging.h"

#undef GetOpenFileName
#undef GetSaveFileName

namespace GdiWrapper
{
	INITIALIZE_OUT_WRAPPED_PROC(GetOpenFileNameA, unused);
	INITIALIZE_OUT_WRAPPED_PROC(GetOpenFileNameW, unused);
	INITIALIZE_OUT_WRAPPED_PROC(GetSaveFileNameA, unused);
	INITIALIZE_OUT_WRAPPED_PROC(GetSaveFileNameW, unused);
}

using namespace GdiWrapper;

template <class T>
void UpdateOpenFileNameStruct(T& OpenFile)
{
	OpenFile.Flags &= ~(OFN_ENABLEHOOK | OFN_ENABLETEMPLATE);
}

BOOL WINAPI comdlg_GetOpenFileNameA(LPOPENFILENAMEA lpOpenFile)
{
	Logging::LogDebug() << __FUNCTION__;

	DEFINE_STATIC_PROC_ADDRESS(GetOpenFileNameAProc, GetOpenFileName, GetOpenFileNameA_out);

	if (!GetOpenFileName)
	{
		return 0;
	}

	if (lpOpenFile && (lpOpenFile->Flags & OFN_ENABLEHOOK))
	{
		OPENFILENAMEA OpenFile = {};
		memcpy(&OpenFile, lpOpenFile, lpOpenFile->lStructSize);
		UpdateOpenFileNameStruct(OpenFile);

		return GetOpenFileName(&OpenFile);
	}

	return GetOpenFileName(lpOpenFile);
}

BOOL WINAPI comdlg_GetOpenFileNameW(LPOPENFILENAMEW lpOpenFile)
{
	Logging::LogDebug() << __FUNCTION__;

	DEFINE_STATIC_PROC_ADDRESS(GetOpenFileNameWProc, GetOpenFileName, GetOpenFileNameW_out);

	if (!GetOpenFileName)
	{
		return 0;
	}

	if (lpOpenFile && (lpOpenFile->Flags & OFN_ENABLEHOOK))
	{
		OPENFILENAMEW OpenFile = {};
		memcpy(&OpenFile, lpOpenFile, lpOpenFile->lStructSize);
		UpdateOpenFileNameStruct(OpenFile);

		return GetOpenFileName(&OpenFile);
	}

	return GetOpenFileName(lpOpenFile);
}

BOOL WINAPI comdlg_GetSaveFileNameA(LPOPENFILENAMEA lpOpenFile)
{
	Logging::LogDebug() << __FUNCTION__;

	DEFINE_STATIC_PROC_ADDRESS(GetSaveFileNameAProc, GetSaveFileName, GetSaveFileNameA_out);

	if (!GetSaveFileName)
	{
		return 0;
	}

	if (lpOpenFile && (lpOpenFile->Flags & OFN_ENABLEHOOK))
	{
		OPENFILENAMEA OpenFile = {};
		memcpy(&OpenFile, lpOpenFile, lpOpenFile->lStructSize);
		UpdateOpenFileNameStruct(OpenFile);

		return GetSaveFileName(&OpenFile);
	}

	return GetSaveFileName(lpOpenFile);
}

BOOL WINAPI comdlg_GetSaveFileNameW(LPOPENFILENAMEW lpOpenFile)
{
	Logging::LogDebug() << __FUNCTION__;

	DEFINE_STATIC_PROC_ADDRESS(GetSaveFileNameWProc, GetSaveFileName, GetSaveFileNameW_out);

	if (!GetSaveFileName)
	{
		return 0;
	}

	if (lpOpenFile && (lpOpenFile->Flags & OFN_ENABLEHOOK))
	{
		OPENFILENAMEW OpenFile = {};
		memcpy(&OpenFile, lpOpenFile, lpOpenFile->lStructSize);
		UpdateOpenFileNameStruct(OpenFile);

		return GetSaveFileName(&OpenFile);
	}

	return GetSaveFileName(lpOpenFile);
}
