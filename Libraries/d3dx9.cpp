/**
* Copyright (C) 2017 Elisha Riedlinger
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

#include "d3dx9.h"
#include "Utils\Utils.h"
#include "Logging\Logging.h"

typedef HRESULT(WINAPI *PFN_D3DXAssembleShader)(LPCSTR pSrcData, UINT SrcDataLen, const D3DXMACRO *pDefines, LPD3DXINCLUDE pInclude, DWORD Flags, LPD3DXBUFFER *ppShader, LPD3DXBUFFER *ppErrorMsgs);
typedef HRESULT(WINAPI *PFN_D3DXDisassembleShader)(const DWORD *pShader, BOOL EnableColorCode, LPCSTR pComments, LPD3DXBUFFER *ppDisassembly);
typedef HRESULT(WINAPI *PFN_D3DXLoadSurfaceFromSurface)(LPDIRECT3DSURFACE9 pDestSurface, const PALETTEENTRY *pDestPalette, const RECT *pDestRect, LPDIRECT3DSURFACE9 pSrcSurface, const PALETTEENTRY *pSrcPalette, const RECT *pSrcRect, DWORD Filter, D3DCOLOR ColorKey);
PFN_D3DXAssembleShader D3DXAssembleShaderPtr = nullptr;
PFN_D3DXDisassembleShader D3DXDisassembleShaderPtr = nullptr;
PFN_D3DXLoadSurfaceFromSurface D3DXLoadSurfaceFromSurfacePtr = nullptr;
HMODULE d3dx9Module = nullptr;

void Loadd3dx9()
{
	static bool IsLoaded = false;
	if (IsLoaded)
	{
		return; // Only load the dll once
	}
	IsLoaded = true;

	// Declare module vars
	d3dx9Module = nullptr;

	// Declare d3dx9_xx.dll name
	static char d3dx9name[MAX_PATH];
	if (!d3dx9Module)
	{
		Logging::Log() << "Loading d3dx9_xx.dll";
		// Declare d3dx9_xx.dll version
		for (int x = 99; x > 9 && d3dx9Module == nullptr; x--)
		{
			// Get dll name
			sprintf_s(d3dx9name, "d3dx9_%d.dll", x);
			// Load dll
			d3dx9Module = LoadLibrary(d3dx9name, false);
		}
	}

	if (d3dx9Module)
	{
		D3DXAssembleShaderPtr = reinterpret_cast<PFN_D3DXAssembleShader>(GetProcAddress(d3dx9Module, "D3DXAssembleShader"));
		D3DXDisassembleShaderPtr = reinterpret_cast<PFN_D3DXDisassembleShader>(GetProcAddress(d3dx9Module, "D3DXDisassembleShader"));
		D3DXLoadSurfaceFromSurfacePtr = reinterpret_cast<PFN_D3DXLoadSurfaceFromSurface>(GetProcAddress(d3dx9Module, "D3DXLoadSurfaceFromSurface"));
		if (!D3DXAssembleShaderPtr)
		{
			Logging::Log() << "Failed to get 'D3DXAssembleShader' ProcAddress of d3dx9_xx.dll!";
		}
		if (!D3DXDisassembleShaderPtr)
		{
			Logging::Log() << "Failed to get 'D3DXDisassembleShader' ProcAddress of d3dx9_xx.dll!";
		}
		if (!D3DXLoadSurfaceFromSurfacePtr)
		{
			Logging::Log() << "Failed to get 'D3DXLoadSurfaceFromSurface' ProcAddress of d3dx9_xx.dll!";
		}
	}
	else
	{
		Logging::Log() << "Failed to load dwmapi.dll!";
	}
}

HRESULT D3DXAssembleShader(LPCSTR pSrcData, UINT SrcDataLen, const D3DXMACRO *pDefines, LPD3DXINCLUDE pInclude, DWORD Flags, LPD3DXBUFFER *ppShader, LPD3DXBUFFER *ppErrorMsgs)
{
	// Load module
	Loadd3dx9();

	// Call function
	if (D3DXAssembleShaderPtr)
	{
		return D3DXAssembleShaderPtr(pSrcData, SrcDataLen, pDefines, pInclude, Flags, ppShader, ppErrorMsgs);
	}
	return D3DERR_INVALIDCALL;
}

HRESULT D3DXDisassembleShader(const DWORD *pShader, BOOL EnableColorCode, LPCSTR pComments, LPD3DXBUFFER *ppDisassembly)
{
	// Load module
	Loadd3dx9();

	// Call function
	if (D3DXDisassembleShaderPtr)
	{
		return D3DXDisassembleShaderPtr(pShader, EnableColorCode, pComments, ppDisassembly);
	}
	return D3DERR_INVALIDCALL;
}

HRESULT D3DXLoadSurfaceFromSurface(LPDIRECT3DSURFACE9 pDestSurface, const PALETTEENTRY *pDestPalette, const RECT *pDestRect, LPDIRECT3DSURFACE9 pSrcSurface, const PALETTEENTRY *pSrcPalette, const RECT *pSrcRect, DWORD Filter, D3DCOLOR ColorKey)
{
	// Load module
	Loadd3dx9();

	// Call function
	if (D3DXLoadSurfaceFromSurfacePtr)
	{
		return D3DXLoadSurfaceFromSurfacePtr(pDestSurface, pDestPalette, pDestRect, pSrcSurface, pSrcPalette, pSrcRect, Filter, ColorKey);
	}
	return D3DERR_INVALIDCALL;
}