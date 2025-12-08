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

#include "d3d8External.h"
#include "d3d9\d3d9External.h"
#include "External\d3d8to9\source\d3d8to9.hpp"
#include "External\d3d8to9\source\d3dx9.hpp"
#include "IClassFactory\IClassFactory.h"
#include "Utils\Utils.h"
#include "Settings\Settings.h"
#include "Logging\Logging.h"
#include "BuildNo.rc"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define APP_VERSION TOSTRING(FILEVERSION)

extern FARPROC f_D3DXAssembleShader;
extern FARPROC f_D3DXDisassembleShader;
extern FARPROC f_D3DXLoadSurfaceFromSurface;

PFN_D3DXAssembleShader D3DXAssembleShader = (PFN_D3DXAssembleShader)f_D3DXAssembleShader;
PFN_D3DXDisassembleShader D3DXDisassembleShader = (PFN_D3DXDisassembleShader)f_D3DXDisassembleShader;
PFN_D3DXLoadSurfaceFromSurface D3DXLoadSurfaceFromSurface = (PFN_D3DXLoadSurfaceFromSurface)f_D3DXLoadSurfaceFromSurface;

namespace D3d8Wrapper
{
	INITIALIZE_OUT_WRAPPED_PROC(Direct3DCreate9, unused);

	static void CheckSystemModule()
	{
		static bool RunOnce = true;
		if (RunOnce && Utils::CheckIfSystemModuleLoaded("d3d8.dll"))
		{
			Logging::Log() << "Warning: System 'd3d8.dll' is already loaded before dxwrapper!";
		}
		RunOnce = false;
	}
}

using namespace D3d8Wrapper;

HRESULT WINAPI d8_ValidatePixelShader(const DWORD* pPixelShader, const D3DCAPS8* pCaps, BOOL ErrorsFlag, char** Errors)
{
	LOG_LIMIT(1, __FUNCTION__);

	HRESULT hr = E_FAIL;
	const char* message = "";

	// Check null
	if (!pPixelShader)
	{
		message = "Invalid shader code pointer.\n";
	}
	else
	{
		// Get shader version
		DWORD version = *pPixelShader;

		// Supported versions
		bool supported = false;
		switch (version)
		{
		case D3DPS_VERSION(1, 0):
		case D3DPS_VERSION(1, 1):
		case D3DPS_VERSION(1, 2):
		case D3DPS_VERSION(1, 3):
		case D3DPS_VERSION(1, 4):
			supported = true;
			break;
		}

		if (!supported)
		{
			message = "Unsupported shader version.\n";
		}
		else if (pCaps && version > pCaps->PixelShaderVersion)
		{
			message = "Shader version not supported by caps.\n";
		}
		else
		{
			// Try disassembling to see if it's valid bytecode
			ID3DXBuffer* pDisasm = nullptr;
			hr = D3DXDisassembleShader(pPixelShader, FALSE, nullptr, &pDisasm);

			if (SUCCEEDED(hr))
			{
				hr = D3D_OK;
			}
			else
			{
				message = "Shader disassembly failed. Possibly invalid bytecode.\n";
			}

			if (pDisasm)
			{
				pDisasm->Release();
			}
		}
	}

	// Only output error if flag is set
	if (!ErrorsFlag)
	{
		message = "";
	}

	if (Errors)
	{
		size_t size = strlen(message) + 1;
		*Errors = (char*)HeapAlloc(GetProcessHeap(), 0, size);
		if (*Errors)
		{
			memcpy(*Errors, message, size);
		}
	}

	return hr;
}

HRESULT WINAPI d8_ValidateVertexShader(const DWORD* pVertexShader, const DWORD* pDeclaration, const D3DCAPS8* pCaps, BOOL ErrorsFlag, char** Errors)
{
	LOG_LIMIT(1, __FUNCTION__);

	HRESULT hr = E_FAIL;
	const char* message = "";

	// Check shader pointer
	if (!pVertexShader)
	{
		message = "Invalid vertex shader pointer.\n";
	}
	else
	{
		DWORD version = *pVertexShader;

		// Check if supported
		bool supported = false;
		switch (version)
		{
		case D3DVS_VERSION(1, 0):
		case D3DVS_VERSION(1, 1):
			supported = true;
			break;
		}

		if (!supported)
		{
			message = "Unsupported vertex shader version.\n";
		}
		else if (pCaps && version > pCaps->VertexShaderVersion)
		{
			message = "Vertex shader version not supported by caps.\n";
		}
		else
		{
			// Attempt to disassemble shader (soft validation)
			ID3DXBuffer* pDisasm = nullptr;
			hr = D3DXDisassembleShader(pVertexShader, FALSE, nullptr, &pDisasm);

			if (FAILED(hr))
			{
				message = "Shader disassembly failed. Possibly invalid bytecode.\n";
			}
			else
			{
				hr = D3D_OK;
			}

			if (pDisasm)
			{
				pDisasm->Release();
			}
		}
	}

	// Check vertex declaration pointer
	if (SUCCEEDED(hr) && pDeclaration)
	{
		// Ensure declaration ends with D3DVSD_END()
		const DWORD* decl = pDeclaration;
		const size_t maxDeclDWords = 256; // Prevent runaway loop

		for (size_t i = 0; i < maxDeclDWords; ++i, ++decl)
		{
			if (*decl == D3DVSD_END())
			{
				break; // Valid end found
			}
		}

		if (*decl != D3DVSD_END())
		{
			message = "Vertex declaration appears malformed or unterminated.\n";
			hr = E_FAIL;
		}
	}

	// Clear error if not requested
	if (!ErrorsFlag)
	{
		message = "";
	}

	// Output error if requested
	if (Errors)
	{
		size_t size = strlen(message) + 1;
		*Errors = (char*)HeapAlloc(GetProcessHeap(), 0, size);
		if (*Errors)
		{
			memcpy(*Errors, message, size);
		}
	}

	return hr;
}

Direct3D8 *WINAPI d8_Direct3DCreate8(UINT SDKVersion)
{
	LOG_LIMIT(1, __FUNCTION__);

	CheckSystemModule();

	if (!Config.D3d8to9)
	{
		return nullptr;
	}
	
	if (Config.SetSwapEffectShim < 2)
	{
		Direct3D9SetSwapEffectUpgradeShim(Config.SetSwapEffectShim);
	}

	LOG_ONCE("Starting D3d8to9 v" << APP_VERSION);

	LOG_LIMIT(3, "Redirecting 'Direct3DCreate8' to --> 'Direct3DCreate9' (" << SDKVersion << ")");

	// Declare Direct3DCreate9
	DEFINE_STATIC_PROC_ADDRESS(Direct3DCreate9Proc, Direct3DCreate9, Direct3DCreate9_out);

	if (!Direct3DCreate9)
	{
		LOG_LIMIT(100, "Failed to get 'Direct3DCreate9' ProcAddress of d3d9.dll!");
		return nullptr;
	}

	IDirect3D9 *const d3d = Direct3DCreate9(D3D_SDK_VERSION);

	if (!d3d)
	{
		return nullptr;
	}

	// Set DirectX version
	m_IDirect3D9Ex* D3DX = nullptr;
	if (SUCCEEDED(d3d->QueryInterface(IID_GetInterfaceX, reinterpret_cast<LPVOID*>(&D3DX))))
	{
		D3DX->SetDirectXVersion(8);
	}

	return new Direct3D8(d3d);
}
