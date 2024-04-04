/**
* Copyright (C) 2023 Elisha Riedlinger
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
#include "d3dx9_data.h"
#include "External\MemoryModule\MemoryModule.h"
#include "Utils\Utils.h"
#include "Logging\Logging.h"

typedef HRESULT(WINAPI* PFN_D3DXCreateTexture)(LPDIRECT3DDEVICE9 pDevice, UINT Width, UINT Height, UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, LPDIRECT3DTEXTURE9* ppTexture);
typedef HRESULT(WINAPI* PFN_D3DXLoadSurfaceFromMemory)(LPDIRECT3DSURFACE9 pDestSurface, const PALETTEENTRY* pDestPalette, const RECT* pDestRect, LPCVOID pSrcMemory, D3DFORMAT SrcFormat, UINT SrcPitch, const PALETTEENTRY* pSrcPalette, const RECT* pSrcRect, DWORD Filter, D3DCOLOR ColorKey);
typedef HRESULT(WINAPI* PFN_D3DXLoadSurfaceFromSurface)(LPDIRECT3DSURFACE9 pDestSurface, const PALETTEENTRY* pDestPalette, const RECT* pDestRect, LPDIRECT3DSURFACE9 pSrcSurface, const PALETTEENTRY* pSrcPalette, const RECT* pSrcRect, DWORD Filter, D3DCOLOR ColorKey);
typedef HRESULT(WINAPI* PFN_D3DXSaveSurfaceToFileInMemory)(LPD3DXBUFFER* ppDestBuf, D3DXIMAGE_FILEFORMAT DestFormat, LPDIRECT3DSURFACE9 pSrcSurface, const PALETTEENTRY* pSrcPalette, const RECT* SrcRect);
typedef HRESULT(WINAPI* PFN_D3DXSaveTextureToFileInMemory)(LPD3DXBUFFER* ppDestBuf, D3DXIMAGE_FILEFORMAT DestFormat, LPDIRECT3DBASETEXTURE9 pSrcTexture, const PALETTEENTRY* pSrcPalette);
typedef HRESULT(WINAPI* PFN_D3DXAssembleShader)(LPCSTR pSrcData, UINT SrcDataLen, const D3DXMACRO* pDefines, LPD3DXINCLUDE pInclude, DWORD Flags, LPD3DXBUFFER* ppShader, LPD3DXBUFFER* ppErrorMsgs);
typedef HRESULT(WINAPI* PFN_D3DXDisassembleShader)(const DWORD* pShader, BOOL EnableColorCode, LPCSTR pComments, LPD3DXBUFFER* ppDisassembly);

typedef HRESULT(WINAPI* PFN_D3DAssemble)(const void* pSrcData, SIZE_T SrcDataSize, const char* pFileName, const D3D_SHADER_MACRO* pDefines, ID3DInclude* pInclude, UINT Flags, ID3DBlob** ppShader, ID3DBlob** ppErrorMsgs);
typedef HRESULT(WINAPI* PFN_D3DCompile)(LPCVOID pSrcData, SIZE_T SrcDataSize, LPCSTR pSourceName, const D3D_SHADER_MACRO* pDefines, ID3DInclude* pInclude, LPCSTR pEntrypoint, LPCSTR pTarget, UINT Flags1, UINT Flags2, ID3DBlob** ppCode, ID3DBlob** ppErrorMsgs);
typedef HRESULT(WINAPI* PFN_D3DDisassemble)(LPCVOID pSrcData, SIZE_T SrcDataSize, UINT Flags, LPCSTR szComments, ID3DBlob** ppDisassembly);

typedef HRESULT(WINAPI* PFN_D3DXFillTexture)(LPVOID pTexture, LPD3DXFILL3D pFunction, LPVOID pData);

HMEMORYMODULE d3dx9Module = nullptr;
HMEMORYMODULE d3dCompileModule = nullptr;

PFN_D3DXCreateTexture p_D3DXCreateTexture = nullptr;
PFN_D3DXLoadSurfaceFromMemory p_D3DXLoadSurfaceFromMemory = nullptr;
PFN_D3DXLoadSurfaceFromSurface p_D3DXLoadSurfaceFromSurface = nullptr;
PFN_D3DXSaveSurfaceToFileInMemory p_D3DXSaveSurfaceToFileInMemory = nullptr;
PFN_D3DXSaveTextureToFileInMemory p_D3DXSaveTextureToFileInMemory = nullptr;
PFN_D3DXAssembleShader p_D3DXAssembleShader = nullptr;
PFN_D3DXDisassembleShader p_D3DXDisassembleShader = nullptr;

PFN_D3DAssemble p_D3DAssemble = nullptr;
PFN_D3DCompile p_D3DCompile = nullptr;
PFN_D3DDisassemble p_D3DDisassemble = nullptr;

PFN_D3DXFillTexture p_D3DXFillTexture = nullptr;

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

	d3dx9Module = Utils::LoadMemoryToDLL((LPVOID)D3DX9_43, sizeof(D3DX9_43));
	d3dCompileModule = Utils::LoadMemoryToDLL((LPVOID)D3DCompiler_47, sizeof(D3DCompiler_47));

	if (!d3dx9Module || !d3dCompileModule)
	{
		Logging::Log() << __FUNCTION__ << "Error: failed to load d3dx9 modules!";
	}
	if (d3dx9Module)
	{
		p_D3DXCreateTexture = reinterpret_cast<PFN_D3DXCreateTexture>(MemoryGetProcAddress(d3dx9Module, "D3DXCreateTexture"));
		p_D3DXLoadSurfaceFromMemory = reinterpret_cast<PFN_D3DXLoadSurfaceFromMemory>(MemoryGetProcAddress(d3dx9Module, "D3DXLoadSurfaceFromMemory"));
		p_D3DXLoadSurfaceFromSurface = reinterpret_cast<PFN_D3DXLoadSurfaceFromSurface>(MemoryGetProcAddress(d3dx9Module, "D3DXLoadSurfaceFromSurface"));
		p_D3DXSaveSurfaceToFileInMemory = reinterpret_cast<PFN_D3DXSaveSurfaceToFileInMemory>(MemoryGetProcAddress(d3dx9Module, "D3DXSaveSurfaceToFileInMemory"));
		p_D3DXSaveTextureToFileInMemory = reinterpret_cast<PFN_D3DXSaveTextureToFileInMemory>(MemoryGetProcAddress(d3dx9Module, "D3DXSaveTextureToFileInMemory"));
		p_D3DXAssembleShader = reinterpret_cast<PFN_D3DXAssembleShader>(MemoryGetProcAddress(d3dx9Module, "D3DXAssembleShader"));
		p_D3DXDisassembleShader = reinterpret_cast<PFN_D3DXDisassembleShader>(MemoryGetProcAddress(d3dx9Module, "D3DXDisassembleShader"));
		p_D3DXFillTexture = reinterpret_cast<PFN_D3DXFillTexture>(MemoryGetProcAddress(d3dx9Module, "D3DXFillTexture"));
	}
	if (d3dCompileModule)
	{
		p_D3DAssemble = reinterpret_cast<PFN_D3DAssemble>(MemoryGetProcAddress(d3dCompileModule, "D3DAssemble"));
		p_D3DCompile = reinterpret_cast<PFN_D3DCompile>(MemoryGetProcAddress(d3dCompileModule, "D3DCompile"));
		p_D3DDisassemble = reinterpret_cast<PFN_D3DDisassemble>(MemoryGetProcAddress(d3dCompileModule, "D3DDisassemble"));
	}
}

HRESULT WINAPI D3DXCreateTexture(LPDIRECT3DDEVICE9 pDevice, UINT Width, UINT Height, UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, LPDIRECT3DTEXTURE9* ppTexture)
{
	Logging::LogDebug() << __FUNCTION__;

	LoadD3dx9();

	if (!p_D3DXCreateTexture)
	{
		LOG_ONCE(__FUNCTION__ << " Error: Could not find ProcAddress!");
		return D3DERR_INVALIDCALL;
	}

	HRESULT hr = p_D3DXCreateTexture(pDevice, Width, Height, MipLevels, Usage, Format, Pool, ppTexture);

	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Warning: Failed to Create texture!";
	}

	return hr;
}

HRESULT WINAPI D3DXLoadSurfaceFromMemory(LPDIRECT3DSURFACE9 pDestSurface, const PALETTEENTRY* pDestPalette, const RECT* pDestRect, LPCVOID pSrcMemory, D3DFORMAT SrcFormat, UINT SrcPitch, const PALETTEENTRY* pSrcPalette, const RECT* pSrcRect, DWORD Filter, D3DCOLOR ColorKey)
{
	Logging::LogDebug() << __FUNCTION__;

	LoadD3dx9();

	if (!p_D3DXLoadSurfaceFromMemory)
	{
		LOG_ONCE(__FUNCTION__ << " Error: Could not find ProcAddress!");
		return D3DERR_INVALIDCALL;
	}

	HRESULT hr = p_D3DXLoadSurfaceFromMemory(pDestSurface, pDestPalette, pDestRect, pSrcMemory, SrcFormat, SrcPitch, pSrcPalette, pSrcRect, Filter, ColorKey);

	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Warning: Failed to Copy surface from memory!";
		return hr;
	}

	LPDIRECT3DTEXTURE9 pDestTexture = nullptr;
	if (SUCCEEDED(pDestSurface->GetContainer(IID_IDirect3DTexture9, (void**)&pDestTexture)))
	{
		pDestTexture->AddDirtyRect(pDestRect);
		pDestTexture->Release();
	}

	return D3D_OK;
}

HRESULT WINAPI D3DXLoadSurfaceFromSurface(LPDIRECT3DSURFACE9 pDestSurface, const PALETTEENTRY* pDestPalette, const RECT* pDestRect, LPDIRECT3DSURFACE9 pSrcSurface, const PALETTEENTRY* pSrcPalette, const RECT* pSrcRect, DWORD Filter, D3DCOLOR ColorKey)
{
	Logging::LogDebug() << __FUNCTION__;

	LoadD3dx9();

	if (!p_D3DXLoadSurfaceFromSurface)
	{
		LOG_ONCE(__FUNCTION__ << " Error: Could not find ProcAddress!");
		return D3DERR_INVALIDCALL;
	}

	HRESULT hr = p_D3DXLoadSurfaceFromSurface(pDestSurface, pDestPalette, pDestRect, pSrcSurface, pSrcPalette, pSrcRect, Filter, ColorKey);

	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Warning: Failed to Copy surface!";
		return hr;
	}

	LPDIRECT3DTEXTURE9 pDestTexture = nullptr;
	if (SUCCEEDED(pDestSurface->GetContainer(IID_IDirect3DTexture9, (void**)&pDestTexture)))
	{
		pDestTexture->AddDirtyRect(pDestRect);
		pDestTexture->Release();
	}

	return D3D_OK;
}

HRESULT WINAPI D3DXSaveSurfaceToFileInMemory(LPD3DXBUFFER* ppDestBuf, D3DXIMAGE_FILEFORMAT DestFormat, LPDIRECT3DSURFACE9 pSrcSurface, const PALETTEENTRY* pSrcPalette, const RECT* SrcRect)
{
	Logging::LogDebug() << __FUNCTION__;

	LoadD3dx9();

	if (!p_D3DXSaveSurfaceToFileInMemory)
	{
		LOG_ONCE(__FUNCTION__ << " Error: Could not find ProcAddress!");
		return D3DERR_INVALIDCALL;
	}

	HRESULT hr = p_D3DXSaveSurfaceToFileInMemory(ppDestBuf, DestFormat, pSrcSurface, pSrcPalette, SrcRect);

	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Warning: Failed to Save surface!";
	}

	return hr;
}

HRESULT WINAPI D3DXSaveTextureToFileInMemory(LPD3DXBUFFER* ppDestBuf, D3DXIMAGE_FILEFORMAT DestFormat, LPDIRECT3DBASETEXTURE9 pSrcTexture, const PALETTEENTRY* pSrcPalette)
{
	Logging::LogDebug() << __FUNCTION__;

	LoadD3dx9();

	if (!p_D3DXSaveTextureToFileInMemory)
	{
		LOG_ONCE(__FUNCTION__ << " Error: Could not find ProcAddress!");
		return D3DERR_INVALIDCALL;
	}

	HRESULT hr = p_D3DXSaveTextureToFileInMemory(ppDestBuf, DestFormat, pSrcTexture, pSrcPalette);

	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Warning: Failed to Save texture!";
	}

	return hr;
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

HRESULT WINAPI D3DAssemble(const void* pSrcData, SIZE_T SrcDataSize, const char* pFileName, const D3D_SHADER_MACRO* pDefines, ID3DInclude* pInclude, UINT Flags, ID3DBlob** ppShader, ID3DBlob** ppErrorMsgs)
{
	Logging::LogDebug() << __FUNCTION__;

	LoadD3dx9();

	if (!p_D3DAssemble)
	{
		LOG_ONCE(__FUNCTION__ << " Error: Could not find ProcAddress!");
		return D3DERR_INVALIDCALL;
	}

	HRESULT hr = p_D3DAssemble(pSrcData, SrcDataSize, pFileName, pDefines, pInclude, Flags, ppShader, ppErrorMsgs);

	if (FAILED(hr))
	{
		if (ppErrorMsgs && *ppErrorMsgs)
		{
			Logging::Log() << __FUNCTION__ << " Warning: Failed to Assemble shader! " << static_cast<const char*>((*ppErrorMsgs)->GetBufferPointer());
		}
		else
		{
			Logging::Log() << __FUNCTION__ << " Warning: Failed to Assemble shader!";
		}
	}

	return hr;
}

HRESULT WINAPI D3DCompile(LPCVOID pSrcData, SIZE_T SrcDataSize, LPCSTR pSourceName, const D3D_SHADER_MACRO* pDefines, ID3DInclude* pInclude, LPCSTR pEntrypoint, LPCSTR pTarget, UINT Flags1, UINT Flags2, ID3DBlob** ppCode, ID3DBlob** ppErrorMsgs)
{
	Logging::LogDebug() << __FUNCTION__;

	LoadD3dx9();

	if (!p_D3DCompile)
	{
		LOG_ONCE(__FUNCTION__ << " Error: Could not find ProcAddress!");
		return D3DERR_INVALIDCALL;
	}

	HRESULT hr = p_D3DCompile(pSrcData, SrcDataSize, pSourceName, pDefines, pInclude, pEntrypoint, pTarget, Flags1, Flags2, ppCode, ppErrorMsgs);

	if (FAILED(hr))
	{
		if (ppErrorMsgs && *ppErrorMsgs)
		{
			Logging::Log() << __FUNCTION__ << " Warning: Failed to Compile shader! " << static_cast<const char*>((*ppErrorMsgs)->GetBufferPointer());
		}
		else
		{
			Logging::Log() << __FUNCTION__ << " Warning: Failed to Compile shader!";
		}
	}

	return hr;
}

HRESULT WINAPI D3DDisassemble(LPCVOID pSrcData, SIZE_T SrcDataSize, UINT Flags, LPCSTR szComments, ID3DBlob** ppDisassembly)
{
	Logging::LogDebug() << __FUNCTION__;

	LoadD3dx9();

	if (!p_D3DDisassemble)
	{
		LOG_ONCE(__FUNCTION__ << " Error: Could not find ProcAddress!");
		return D3DERR_INVALIDCALL;
	}

	HRESULT hr = p_D3DDisassemble(pSrcData, SrcDataSize, Flags, szComments, ppDisassembly);

	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Warning: Failed to Disassemble shader!";
	}

	return hr;
}

HRESULT WINAPI D3DXFillTexture(LPVOID pTexture, LPD3DXFILL3D pFunction, LPVOID pData)
{
	Logging::LogDebug() << __FUNCTION__;

	LoadD3dx9();

	if (!p_D3DXFillTexture)
	{
		LOG_ONCE(__FUNCTION__ << " Error: Could not find ProcAddress!");
		return D3DERR_INVALIDCALL;
	}

	HRESULT hr = p_D3DXFillTexture(pTexture, pFunction, pData);

	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Warning: Failed to fill texture!";
	}

	return hr;
}
