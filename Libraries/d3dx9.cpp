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
#include "d3dx9.h"
#include "d3dx9_data_43.h"
#include "d3dx9_data_47.h"
#include <string>
#include "External\MemoryModule\MemoryModule.h"
#include "ComPtr.h"
#include "IClassFactory\IClassFactory.h"
#include "Utils\Utils.h"
#include "Logging\Logging.h"

#define D3DX_DEFINE_FUNCT(procName) \
	PFN_ ## procName procName = nullptr;

#define D3DX_LOAD_FUNCT(procName) \
	procName = reinterpret_cast<PFN_ ## procName>(MemoryGetProcAddress(dll, #procName)); \
	if (!procName) Logging::Log() << __FUNCTION__ << " Error: failed to find d3dx9 '" << #procName << "' function!";

VISIT_D3DX_MODULE_FUNCT(D3DX_DEFINE_FUNCT);
VISIT_D3DX_COMPILE_FUNCT(D3DX_DEFINE_FUNCT);

typedef HRESULT(WINAPI* PFN_D3DXLoadSurfaceFromMemory)(LPDIRECT3DSURFACE9 pDestSurface, const PALETTEENTRY* pDestPalette, const RECT* pDestRect, LPCVOID pSrcMemory, D3DFORMAT SrcFormat, UINT SrcPitch, const PALETTEENTRY* pSrcPalette, const RECT* pSrcRect, DWORD Filter, D3DCOLOR ColorKey);
typedef HRESULT(WINAPI* PFN_D3DXLoadSurfaceFromSurface)(LPDIRECT3DSURFACE9 pDestSurface, const PALETTEENTRY* pDestPalette, const RECT* pDestRect, LPDIRECT3DSURFACE9 pSrcSurface, const PALETTEENTRY* pSrcPalette, const RECT* pSrcRect, DWORD Filter, D3DCOLOR ColorKey);

HMEMORYMODULE d3dx9Module = nullptr;
HMEMORYMODULE d3dCompileModule = nullptr;

PFN_D3DXLoadSurfaceFromMemory p_D3DXLoadSurfaceFromMemory = nullptr;
PFN_D3DXLoadSurfaceFromSurface p_D3DXLoadSurfaceFromSurface = nullptr;

FARPROC f_D3DXAssembleShader = (FARPROC)*D3DXAssembleShader;
FARPROC f_D3DXDisassembleShader = (FARPROC)*D3DXDisassembleShader;
FARPROC f_D3DXLoadSurfaceFromSurface = (FARPROC)*D3DXLoadSurfaceFromSurface;

void LoadD3dx9()
{
	static bool RunOnce = true;
	if (!RunOnce)
	{
		return;
	}
	RunOnce = false;

	Logging::Log() << "Loading d3dx9 libraries";

	d3dx9Module = Utils::LoadMemoryToDLL((LPVOID)D3DX9_43, sizeof(D3DX9_43));
#if (_WIN32_WINNT >= 0x0502)
	d3dCompileModule = Utils::LoadMemoryToDLL((LPVOID)D3DCompiler_47, sizeof(D3DCompiler_47));
#else
	d3dCompileModule = Utils::LoadMemoryToDLL((LPVOID)D3DCompiler_43, sizeof(D3DCompiler_43));
#endif

	if (d3dx9Module)
	{
		const HMEMORYMODULE dll = d3dx9Module;

		p_D3DXLoadSurfaceFromMemory = reinterpret_cast<PFN_D3DXLoadSurfaceFromMemory>(MemoryGetProcAddress(d3dx9Module, "D3DXLoadSurfaceFromMemory"));
		if (!p_D3DXLoadSurfaceFromMemory)
		{
			Logging::Log() << __FUNCTION__ << " Error: failed to find d3dx9 'D3DXLoadSurfaceFromMemory' function!";
		}

		p_D3DXLoadSurfaceFromSurface = reinterpret_cast<PFN_D3DXLoadSurfaceFromSurface>(MemoryGetProcAddress(d3dx9Module, "D3DXLoadSurfaceFromSurface"));
		if (!p_D3DXLoadSurfaceFromSurface)
		{
			Logging::Log() << __FUNCTION__ << " Error: failed to find d3dx9 'D3DXLoadSurfaceFromSurface' function!";
		}

		VISIT_D3DX_MODULE_FUNCT(D3DX_LOAD_FUNCT);
	}
	else
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to load d3dx9 module!";
	}
	if (d3dCompileModule)
	{
		const HMEMORYMODULE dll = d3dCompileModule;

		VISIT_D3DX_COMPILE_FUNCT(D3DX_LOAD_FUNCT);
	}
	else
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to load d3dx9 compile module!";
	}
}

HRESULT WINAPI D3DXLoadSurfaceFromMemory(LPDIRECT3DSURFACE9 pDestSurface, const PALETTEENTRY* pDestPalette, const RECT* pDestRect, LPCVOID pSrcMemory, D3DFORMAT SrcFormat, UINT SrcPitch, const PALETTEENTRY* pSrcPalette, const RECT* pSrcRect, DWORD Filter, D3DCOLOR ColorKey)
{
	Logging::LogDebug() << __FUNCTION__;

	// Get real d3d9 surface (no need to Release)
	LPDIRECT3DSURFACE9 pDestRealSurface = nullptr;
	if (SUCCEEDED(pDestSurface->QueryInterface(IID_GetRealInterface, (void**)&pDestRealSurface)))
	{
		pDestSurface = pDestRealSurface;
	}

	HRESULT hr = p_D3DXLoadSurfaceFromMemory(pDestSurface, pDestPalette, pDestRect, pSrcMemory, SrcFormat, SrcPitch, pSrcPalette, pSrcRect, Filter, ColorKey);

	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Error: Failed to Copy surface from memory!";
		return hr;
	}

	// Add dirty rect if surface is a texture
	ComPtr<IDirect3DTexture9> pDestTexture;
	if (SUCCEEDED(pDestSurface->GetContainer(IID_IDirect3DTexture9, (void**)pDestTexture.GetAddressOf())))
	{
		pDestTexture->AddDirtyRect(pDestRect);
	}

	return D3D_OK;
}

HRESULT WINAPI D3DXLoadSurfaceFromSurface(LPDIRECT3DSURFACE9 pDestSurface, const PALETTEENTRY* pDestPalette, const RECT* pDestRect, LPDIRECT3DSURFACE9 pSrcSurface, const PALETTEENTRY* pSrcPalette, const RECT* pSrcRect, DWORD Filter, D3DCOLOR ColorKey)
{
	Logging::LogDebug() << __FUNCTION__;

	// Get real d3d9 surface (no need to Release)
	LPDIRECT3DSURFACE9 pDestRealSurface = nullptr;
	if (SUCCEEDED(pDestSurface->QueryInterface(IID_GetRealInterface, (void**)&pDestRealSurface)))
	{
		pDestSurface = pDestRealSurface;
	}

	LPDIRECT3DSURFACE9 pSrcRealSurface = nullptr;
	if (SUCCEEDED(pSrcSurface->QueryInterface(IID_GetRealInterface, (void**)&pSrcRealSurface)))
	{
		pSrcSurface = pSrcRealSurface;
	}

	HRESULT hr = p_D3DXLoadSurfaceFromSurface(pDestSurface, pDestPalette, pDestRect, pSrcSurface, pSrcPalette, pSrcRect, Filter, ColorKey);

	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Error: Failed to Copy surface!";
		return hr;
	}

	// Add dirty rect if surface is a texture
	ComPtr<IDirect3DTexture9> pDestTexture;
	if (SUCCEEDED(pDestSurface->GetContainer(IID_IDirect3DTexture9, (void**)pDestTexture.GetAddressOf())))
	{
		pDestTexture->AddDirtyRect(pDestRect);
	}

	return D3D_OK;
}

HRESULT WINAPI D3DXAssembleShader(LPCSTR pSrcData, UINT SrcDataLen, const D3DXMACRO* pDefines, LPD3DXINCLUDE pInclude, DWORD Flags, LPD3DXBUFFER* ppShader, LPD3DXBUFFER* ppErrorMsgs)
{
	Logging::LogDebug() << __FUNCTION__;

	return D3DAssemble(pSrcData, SrcDataLen, nullptr, pDefines, (ID3DInclude*)pInclude, Flags, ppShader, ppErrorMsgs);
}

HRESULT WINAPI D3DXDisassembleShader(const DWORD* pShader, BOOL EnableColorCode, LPCSTR pComments, LPD3DXBUFFER* ppDisassembly)
{
	Logging::LogDebug() << __FUNCTION__;

	if (!pShader)
	{
		return D3DERR_INVALIDCALL;
	}

	// Get ASM flags
	UINT Flags = (EnableColorCode) ? D3D_DISASM_ENABLE_COLOR_CODE : 0;

	// Get shader size
	SIZE_T SrcDataSize = 0;
	if (((byte*)pShader)[0] <= 0x04 &&	// Minor version
		((byte*)pShader)[1] <= 0x02 &&	// Majer version
		(((byte*)pShader)[2] == 0xFF || ((byte*)pShader)[2] == 0xFE) &&	// 0xFF = ps_x_x, 0xFE = vs_x_x
		((byte*)pShader)[3] == 0xFF)
	{
		SrcDataSize = 4;
		while (true)
		{
			if (((byte*)pShader)[SrcDataSize + 0] == 0xFF &&
				((byte*)pShader)[SrcDataSize + 1] == 0xFF &&
				((byte*)pShader)[SrcDataSize + 2] == 0x00 &&
				((byte*)pShader)[SrcDataSize + 3] == 0x00)
			{
				SrcDataSize += 4;
				break;
			}
			SrcDataSize++;
			if (SrcDataSize > 50000)
			{
				Logging::Log() << __FUNCTION__ " Error: Shader buffer exceeded!";
				return E_OUTOFMEMORY;
			}
		}
	}
	// Shader byte header not recognized, cannot get shader size
	else
	{
		// Log the byte header
		Logging::Log() << __FUNCTION__ " Error: Shader format not recognized!";
		std::string strerr;
		for (UINT x = 0; x < 11; x++)
		{
			strerr.append(", x");
			char str[20] = { '\0' };
			sprintf_s(str, "%x", ((byte*)pShader)[x]);
			strerr.append(str);
		}
		Logging::Log() << __FUNCTION__ " Beginning bytes: " << strerr.c_str();
		return D3DERR_INVALIDCALL;
	}

	return D3DDisassemble(pShader, SrcDataSize, Flags, pComments, ppDisassembly);
}
