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

#include "ddraw.h"
#include "d3d9\d3d9External.h"

// ******************************
// IUnknown functions
// ******************************

HRESULT m_IDirect3DDeviceX::QueryInterface(REFIID riid, LPVOID FAR * ppvObj, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") " << riid;

	if (!ppvObj)
	{
		return E_POINTER;
	}
	*ppvObj = nullptr;

	if (riid == IID_GetRealInterface)
	{
		*ppvObj = ProxyInterface;
		return D3D_OK;
	}
	if (riid == IID_GetInterfaceX)
	{
		*ppvObj = this;
		return D3D_OK;
	}

	DWORD DxVersion = (CheckWrapperType(riid) && Config.Dd7to9) ? GetGUIDVersion(riid) : DirectXVersion;

	if (riid == GetWrapperType(DxVersion) || riid == IID_IUnknown)
	{
		*ppvObj = GetWrapperInterfaceX(DxVersion);

		AddRef(DxVersion);

		return D3D_OK;
	}

	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, GetWrapperType(DirectXVersion));
}

ULONG m_IDirect3DDeviceX::AddRef(DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") v" << DirectXVersion;

	if (Config.Dd7to9)
	{
		// Some Direct3DDevices share reference count with parent surfaces
		if (parent3DSurface.Interface)
		{
			return parent3DSurface.Interface->AddRef(parent3DSurface.DxVersion);
		}

		switch (DirectXVersion)
		{
		case 1:
			return InterlockedIncrement(&RefCount1);
		case 2:
			return InterlockedIncrement(&RefCount2);
		case 3:
			return InterlockedIncrement(&RefCount3);
		case 7:
			return InterlockedIncrement(&RefCount7);
		default:
			LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
			return 0;
		}
	}

	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DDeviceX::Release(DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") v" << DirectXVersion;

	if (Config.Dd7to9)
	{
		ULONG ref;

		// Some Direct3DDevices share reference count with parent surfaces
		if (parent3DSurface.Interface)
		{
			return parent3DSurface.Interface->Release(parent3DSurface.DxVersion);
		}

		switch (DirectXVersion)
		{
		case 1:
			ref = (InterlockedCompareExchange(&RefCount1, 0, 0)) ? InterlockedDecrement(&RefCount1) : 0;
			break;
		case 2:
			ref = (InterlockedCompareExchange(&RefCount2, 0, 0)) ? InterlockedDecrement(&RefCount2) : 0;
			break;
		case 3:
			ref = (InterlockedCompareExchange(&RefCount3, 0, 0)) ? InterlockedDecrement(&RefCount3) : 0;
			break;
		case 7:
			ref = (InterlockedCompareExchange(&RefCount7, 0, 0)) ? InterlockedDecrement(&RefCount7) : 0;
			break;
		default:
			LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
			ref = 0;
		}

		if (InterlockedCompareExchange(&RefCount1, 0, 0) + InterlockedCompareExchange(&RefCount2, 0, 0) +
			InterlockedCompareExchange(&RefCount3, 0, 0) + InterlockedCompareExchange(&RefCount7, 0, 0) == 0)
		{
			delete this;
		}

		return ref;
	}

	ULONG ref = ProxyInterface->Release();

	if (ref == 0)
	{
		delete this;
	}

	return ref;
}

// ******************************
// IDirect3DDevice v1 functions
// ******************************

HRESULT m_IDirect3DDeviceX::Initialize(LPDIRECT3D lpd3d, LPGUID lpGUID, LPD3DDEVICEDESC lpd3ddvdesc)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Returns D3D_OK if successful, otherwise it returns an error.
		return D3D_OK;
	}

	if (lpd3d)
	{
		lpd3d->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpd3d);
	}

	return GetProxyInterfaceV1()->Initialize(lpd3d, lpGUID, lpd3ddvdesc);
}

HRESULT m_IDirect3DDeviceX::GetCaps(LPD3DDEVICEDESC lpD3DHWDevDesc, LPD3DDEVICEDESC lpD3DHELDevDesc)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if ((!lpD3DHWDevDesc && !lpD3DHELDevDesc) ||
			(lpD3DHWDevDesc && lpD3DHWDevDesc->dwSize != D3DDEVICEDESC1_SIZE && lpD3DHWDevDesc->dwSize != D3DDEVICEDESC5_SIZE && lpD3DHWDevDesc->dwSize != D3DDEVICEDESC6_SIZE) ||
			(lpD3DHELDevDesc && lpD3DHELDevDesc->dwSize != D3DDEVICEDESC1_SIZE && lpD3DHELDevDesc->dwSize != D3DDEVICEDESC5_SIZE && lpD3DHELDevDesc->dwSize != D3DDEVICEDESC6_SIZE))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << ((lpD3DHWDevDesc) ? lpD3DHWDevDesc->dwSize : -1) << " " << ((lpD3DHELDevDesc) ? lpD3DHELDevDesc->dwSize : -1));
			return DDERR_INVALIDPARAMS;
		}

		D3DDEVICEDESC7 D3DDevDesc;
		HRESULT hr = GetCaps(&D3DDevDesc);

		if (SUCCEEDED(hr))
		{
			if (lpD3DHWDevDesc)
			{
				ConvertDeviceDesc(*lpD3DHWDevDesc, D3DDevDesc);
			}

			if (lpD3DHELDevDesc)
			{
				ConvertDeviceDesc(*lpD3DHELDevDesc, D3DDevDesc);
			}
		}

		return hr;
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
		return GetProxyInterfaceV1()->GetCaps(lpD3DHWDevDesc, lpD3DHELDevDesc);
	case 2:
		return GetProxyInterfaceV2()->GetCaps(lpD3DHWDevDesc, lpD3DHELDevDesc);
	case 3:
		return GetProxyInterfaceV3()->GetCaps(lpD3DHWDevDesc, lpD3DHELDevDesc);
	default:
		return DDERR_GENERIC;
	}
}

HRESULT m_IDirect3DDeviceX::SwapTextureHandles(LPDIRECT3DTEXTURE2 lpD3DTex1, LPDIRECT3DTEXTURE2 lpD3DTex2)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpD3DTex1 || !lpD3DTex2)
		{
			return DDERR_INVALIDPARAMS;
		}

		m_IDirect3DTextureX* pTextureX1 = nullptr;
		lpD3DTex1->QueryInterface(IID_GetInterfaceX, (LPVOID*)&pTextureX1);

		m_IDirect3DTextureX* pTextureX2 = nullptr;
		lpD3DTex2->QueryInterface(IID_GetInterfaceX, (LPVOID*)&pTextureX2);

		if (!pTextureX1 || !pTextureX2)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get texture wrapper!");
			return DDERR_INVALIDPARAMS;
		}

		// Find handle associated with texture1
		D3DTEXTUREHANDLE TexHandle1 = 0;
		if (FAILED(pTextureX1->GetHandle((LPDIRECT3DDEVICE2)GetWrapperInterfaceX(0), &TexHandle1)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not find texture1 handle!");
			return DDERR_INVALIDPARAMS;
		}

		// Find handle associated with texture2
		D3DTEXTUREHANDLE TexHandle2 = 0;
		if (FAILED(pTextureX2->GetHandle((LPDIRECT3DDEVICE2)GetWrapperInterfaceX(0), &TexHandle2)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not find texture2 handle!");
			return DDERR_INVALIDPARAMS;
		}

		// Swap texture handle1
		pTextureX1->SetHandle(TexHandle2);
		TextureHandleMap[TexHandle2] = pTextureX1;

		// Swap texture handle2
		pTextureX2->SetHandle(TexHandle1);
		TextureHandleMap[TexHandle1] = pTextureX2;

		// If texture handle is set then use new texture
		if (rsTextureHandle == TexHandle1 || rsTextureHandle == TexHandle2)
		{
			SetTextureHandle(rsTextureHandle);
		}

		return D3D_OK;
	}

	if (lpD3DTex1)
	{
		lpD3DTex1->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpD3DTex1);
	}
	if (lpD3DTex2)
	{
		lpD3DTex2->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpD3DTex2);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
		return GetProxyInterfaceV1()->SwapTextureHandles((LPDIRECT3DTEXTURE)lpD3DTex1, (LPDIRECT3DTEXTURE)lpD3DTex2);
	case 2:
		return GetProxyInterfaceV2()->SwapTextureHandles(lpD3DTex1, lpD3DTex2);
	default:
		return DDERR_GENERIC;
	}
}

HRESULT m_IDirect3DDeviceX::CreateExecuteBuffer(LPD3DEXECUTEBUFFERDESC lpDesc, LPDIRECT3DEXECUTEBUFFER * lplpDirect3DExecuteBuffer, IUnknown * pUnkOuter)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lplpDirect3DExecuteBuffer || !lpDesc)
		{
			return DDERR_INVALIDPARAMS;
		}

		if (lpDesc->dwSize != sizeof(D3DEXECUTEBUFFERDESC))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << lpDesc->dwSize);
			return DDERR_INVALIDPARAMS;
		}

		// Validate dwFlags
		if (!(lpDesc->dwFlags & D3DDEB_BUFSIZE))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: D3DDEB_BUFSIZE flag not set.");
			return DDERR_INVALIDPARAMS;
		}

		// Validate dwBufferSize
		if (lpDesc->dwBufferSize == 0 || lpDesc->dwBufferSize > MAX_EXECUTE_BUFFER_SIZE)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid dwBufferSize: " << lpDesc->dwBufferSize);
			return DDERR_INVALIDPARAMS;
		}

		// Validate dwCaps
		if ((lpDesc->dwFlags & D3DDEB_CAPS) && (lpDesc->dwCaps & D3DDEBCAPS_SYSTEMMEMORY) && (lpDesc->dwCaps & D3DDEBCAPS_VIDEOMEMORY))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Unsupported dwCaps: " << Logging::hex(lpDesc->dwCaps));
			return DDERR_INVALIDPARAMS;
		}

		// Validate lpData
		if ((lpDesc->dwFlags & D3DDEB_LPDATA) && lpDesc->lpData)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: lpData is non-null, using application data.");
		}

		m_IDirect3DExecuteBuffer* pExecuteBuffer = m_IDirect3DExecuteBuffer::CreateDirect3DExecuteBuffer(*lplpDirect3DExecuteBuffer, this, lpDesc);

		*lplpDirect3DExecuteBuffer = pExecuteBuffer;

		return D3D_OK;
	}

	HRESULT hr = GetProxyInterfaceV1()->CreateExecuteBuffer(lpDesc, lplpDirect3DExecuteBuffer, pUnkOuter);

	if (SUCCEEDED(hr) && lplpDirect3DExecuteBuffer)
	{
		*lplpDirect3DExecuteBuffer = m_IDirect3DExecuteBuffer::CreateDirect3DExecuteBuffer(*lplpDirect3DExecuteBuffer, nullptr, nullptr);
	}

	return hr;
}

HRESULT m_IDirect3DDeviceX::GetStats(LPD3DSTATS lpD3DStats)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// The method returns E_NOTIMPL / DDERR_UNSUPPORTED.
		return DDERR_UNSUPPORTED;
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
		return GetProxyInterfaceV1()->GetStats(lpD3DStats);
	case 2:
		return GetProxyInterfaceV2()->GetStats(lpD3DStats);
	case 3:
		return GetProxyInterfaceV3()->GetStats(lpD3DStats);
	default:
		return DDERR_GENERIC;
	}
}

HRESULT m_IDirect3DDeviceX::Execute(LPDIRECT3DEXECUTEBUFFER lpDirect3DExecuteBuffer, LPDIRECT3DVIEWPORT lpDirect3DViewport, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpDirect3DExecuteBuffer || !lpDirect3DViewport)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: invalid params: " << lpDirect3DExecuteBuffer << " " << lpDirect3DViewport);
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_INVALIDOBJECT;
		}

		// Flags
		// D3DEXECUTE_CLIPPED - Clip any primitives in the buffer that are outside or partially outside the viewport. 
		// D3DEXECUTE_UNCLIPPED - All primitives in the buffer are contained within the viewport.

		m_IDirect3DExecuteBuffer* pExecuteBuffer = (m_IDirect3DExecuteBuffer*)lpDirect3DExecuteBuffer;

		LPVOID lpData;
		D3DEXECUTEDATA ExecuteData;
		LPD3DSTATUS lpStatus;

		// Get execute data and desc
		if (FAILED(pExecuteBuffer->GetBuffer(&lpData, ExecuteData, &lpStatus)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: get execute data failed!");
			return DDERR_INVALIDPARAMS;
		}

		if (FAILED(SetCurrentViewport((LPDIRECT3DVIEWPORT3)lpDirect3DViewport)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to set the specified viewport!");
			return DDERR_INVALIDPARAMS;
		}

		// Pointer to the start of the instruction data
		BYTE* instructionData = reinterpret_cast<BYTE*>(lpData) + ExecuteData.dwInstructionOffset;
		BYTE* instructionEnd = instructionData + ExecuteData.dwInstructionLength;

		DWORD opcode = NULL;

		// ToDo: figure out which vertex type is being used D3DFVF_VERTEX, D3DFVF_LVERTEX or D3DFVF_TLVERTEX
		DWORD VertexTypeDesc = D3DFVF_TLVERTEX;

		// Primitive structures and related defines. Vertex offsets are to types D3DVERTEX, D3DLVERTEX, or D3DTLVERTEX.
		BYTE* vertexBuffer = reinterpret_cast<BYTE*>(lpData) + ExecuteData.dwVertexOffset;
		const DWORD vertexCount = ExecuteData.dwVertexCount;

		DWORD EmulatedDriverStatus = 0;

		// Iterate through the instructions
		while (instructionData < instructionEnd)
		{
			const D3DINSTRUCTION* instruction = (const D3DINSTRUCTION*)(instructionData);
			const DWORD instructionSize = sizeof(D3DINSTRUCTION) + (instruction->wCount * instruction->bSize);

			opcode = instruction->bOpcode;
			BYTE* opstruct = instructionData + sizeof(D3DINSTRUCTION);

			if (opcode == D3DOP_EXIT || instructionData + instructionSize > instructionEnd)
			{
				break;
			}

			// Just keep emulated driver status to 0 for now
			//EmulatedDriverStatus |= (1 << opcode); // Set bit based on opcode

			bool SkipNextMove = false;

			switch (opcode)
			{
			case D3DOP_POINT:
				// Sends a point to the renderer. Operand data is described by the D3DPOINT structure.
			{
				D3DPOINT* point = reinterpret_cast<D3DPOINT*>(opstruct);

				if (instruction->bSize != sizeof(D3DPOINT))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Warning: point instruction size does not match!");
				}

				DrawExecutePoint(point, instruction->wCount, vertexCount, vertexBuffer, VertexTypeDesc);

				break;
			}
			case D3DOP_LINE:
				// Sends a line to the renderer. Operand data is described by the D3DLINE structure.
			{
				D3DLINE* line = reinterpret_cast<D3DLINE*>(opstruct);

				if (instruction->bSize != sizeof(D3DLINE))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Warning: line instruction size does not match!");
				}

				DrawExecuteLine(line, instruction->wCount, vertexCount, vertexBuffer, VertexTypeDesc);

				break;
			}
			case D3DOP_TRIANGLE:
				// Sends a triangle to the renderer. Operand data is described by the D3DTRIANGLE structure.
			{
				D3DTRIANGLE* triangle = reinterpret_cast<D3DTRIANGLE*>(opstruct);

				if (instruction->bSize != sizeof(D3DTRIANGLE))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Warning: triangle instruction size does not match!");
				}

				DrawExecuteTriangle(triangle, instruction->wCount, vertexCount, vertexBuffer, VertexTypeDesc);

				break;
			}
			case D3DOP_MATRIXLOAD:
				// Triggers a data transfer in the rendering engine. Operand data is described by the D3DMATRIXLOAD structure.
			{
				D3DMATRIXLOAD* matrixLoad = reinterpret_cast<D3DMATRIXLOAD*>(opstruct);

				if (instruction->bSize != sizeof(D3DMATRIXLOAD))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Warning: matrix load instruction size does not match!");
				}

				for (DWORD i = 0; i < instruction->wCount; i++)
				{
					// Copy matrix to dest
					D3DMATRIX* pSrcMatrix = GetMatrix(matrixLoad[i].hSrcMatrix);
					D3DMATRIX* pDestMatrix = GetMatrix(matrixLoad[i].hDestMatrix);
					if (pSrcMatrix && pDestMatrix)
					{
						*pDestMatrix = *pSrcMatrix;
					}
					else
					{
						LOG_LIMIT(100, __FUNCTION__ << " Error: failed to find matrix handle for load!");
					}
				}

				break;
			}
			case D3DOP_MATRIXMULTIPLY:
				// Triggers a data transfer in the rendering engine. Operand data is described by the D3DMATRIXMULTIPLY structure.
			{
				D3DMATRIXMULTIPLY* matrixMultiply = reinterpret_cast<D3DMATRIXMULTIPLY*>(opstruct);

				if (instruction->bSize != sizeof(D3DMATRIXMULTIPLY))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Warning: matrix multiply instruction size does not match!");
				}

				for (DWORD i = 0; i < instruction->wCount; i++)
				{
					// Multiply matrix to dest
					D3DMATRIX* pSrcMatrix1 = GetMatrix(matrixMultiply[i].hSrcMatrix1);
					D3DMATRIX* pSrcMatrix2 = GetMatrix(matrixMultiply[i].hSrcMatrix2);
					D3DMATRIX* pDestMatrix = GetMatrix(matrixMultiply[i].hDestMatrix);
					if (pSrcMatrix1 && pSrcMatrix2 && pDestMatrix)
					{
						using namespace DirectX;

						// Load D3DMATRIX into XMMATRIX
						XMMATRIX xmMatrix1 = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(pSrcMatrix1));
						XMMATRIX xmMatrix2 = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(pSrcMatrix2));

						// Perform the multiplication
						XMMATRIX xmResult = XMMatrixMultiply(xmMatrix1, xmMatrix2);

						// Store the result back into a D3DMATRIX
						XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(pDestMatrix), xmResult);
					}
					else
					{
						LOG_LIMIT(100, __FUNCTION__ << " Error: failed to find matrix handle for multiply!");
					}
				}

				break;
			}
			case D3DOP_STATETRANSFORM:
				// Sets the value of internal state variables in the rendering engine for the transformation module. Operand data is a variable token
				// and the new value. The token identifies the internal state variable, and the new value is the value to which that variable should
				// be set. For more information about these variables, see the D3DSTATE structure and the D3DLIGHTSTATETYPE enumerated type.
			{
				D3DSTATE* state = reinterpret_cast<D3DSTATE*>(opstruct);

				if (instruction->bSize != sizeof(D3DSTATE))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Warning: state transform instruction size does not match!");
				}

				for (DWORD i = 0; i < instruction->wCount; i++)
				{
					D3DMATRIX* pMatrix = GetMatrix(state[i].dwArg[0]);
					if (pMatrix)
					{
						SetTransform(state[i].dtstTransformStateType, pMatrix);
					}
					else
					{
						LOG_LIMIT(100, __FUNCTION__ << " Error: failed to find matrix handle for transform!");
					}
				}

				break;
			}
			case D3DOP_STATELIGHT:
				// Sets the value of internal state variables in the rendering engine for the lighting module. Operand data is a variable token
				// and the new value. The token identifies the internal state variable, and the new value is the value to which that variable
				// should be set. For more information about these variables, see the D3DSTATE structure and the D3DLIGHTSTATETYPE enumerated type.
			{
				D3DSTATE* state = reinterpret_cast<D3DSTATE*>(opstruct);

				if (instruction->bSize != sizeof(D3DSTATE))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Warning: state light instruction size does not match!");
				}

				for (DWORD i = 0; i < instruction->wCount; i++)
				{
					SetLightState(state[i].dlstLightStateType, state[i].dwArg[0]);
				}

				break;
			}
			case D3DOP_STATERENDER:
				// Sets the value of internal state variables in the rendering engine for the rendering module. Operand data is a variable token
				// and the new value. The token identifies the internal state variable, and the new value is the value to which that variable
				// should be set. For more information about these variables, see the D3DSTATE structure and the D3DLIGHTSTATETYPE enumerated type.
			{
				D3DSTATE* state = reinterpret_cast<D3DSTATE*>(opstruct);

				if (instruction->bSize != sizeof(D3DSTATE))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Warning: state render instruction size does not match!");
				}

				for (DWORD i = 0; i < instruction->wCount; i++)
				{
					SetRenderState(state[i].drstRenderStateType, state[i].dwArg[0]);
				}

				break;
			}
			case D3DOP_TEXTURELOAD:
				// Triggers a data transfer in the rendering engine. Operand data is described by the D3DTEXTURELOAD structure.
			{
				D3DTEXTURELOAD* textureLoad = reinterpret_cast<D3DTEXTURELOAD*>(opstruct);

				if (instruction->bSize != sizeof(D3DTEXTURELOAD))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Warning: texture load instruction size does not match!");
				}

				for (DWORD i = 0; i < instruction->wCount; i++)
				{
					// Copy texture to dest
					m_IDirect3DTextureX* lpTextureSrcX = GetTexture(textureLoad[i].hSrcTexture);
					m_IDirect3DTextureX* lpTextureDestX = GetTexture(textureLoad[i].hDestTexture);
					if (lpTextureSrcX && lpTextureDestX)
					{
						LPDIRECT3DTEXTURE2 lpTextureSrc = (LPDIRECT3DTEXTURE2)lpTextureSrcX->GetWrapperInterfaceX(0);
						lpTextureDestX->Load(lpTextureSrc);
					}
					else
					{
						LOG_LIMIT(100, __FUNCTION__ << " Error: failed to find texture handle!");
					}
				}

				break;
			}
			case D3DOP_PROCESSVERTICES:
				// Sets both lighting and transformations for vertices. Operand data is described by the D3DPROCESSVERTICES structure.
			{
				D3DPROCESSVERTICES* processVertices = reinterpret_cast<D3DPROCESSVERTICES*>(opstruct);

				if (instruction->bSize != sizeof(D3DPROCESSVERTICES))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Warning: process vertices instruction size does not match!");
				}

				if (processVertices->dwFlags != D3DPROCESSVERTICES_COPY && processVertices->dwFlags != (D3DPROCESSVERTICES_COPY | D3DPROCESSVERTICES_UPDATEEXTENTS))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Warning: process vertices instruction is not implemented! Flags: " << Logging::hex(processVertices->dwFlags));
				}

				// ToDo: implement process vertices opcode

				for (DWORD i = 0; i < instruction->wCount; i++)
				{
					UNREFERENCED_PARAMETER(processVertices);
				}

				break;
			}
			case D3DOP_SPAN:
				// Spans a list of points with the same y value. For more information, see the D3DSPAN structure.
			{
				D3DSPAN* span = reinterpret_cast<D3DSPAN*>(opstruct);

				if (instruction->bSize != sizeof(D3DSPAN))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Warning: span instruction size does not match!");
				}

				LOG_LIMIT(100, __FUNCTION__ << " Warning: span instruction is not implemented!");

				// ToDo: implement span opcode

				for (DWORD i = 0; i < instruction->wCount; i++)
				{
					UNREFERENCED_PARAMETER(span);
				}

				break;
			}
			case D3DOP_SETSTATUS:
				// Resets the status of the execute buffer. For more information, see the D3DSTATUS structure.
			{
				D3DSTATUS* status = reinterpret_cast<D3DSTATUS*>(opstruct);

				if (instruction->bSize != sizeof(D3DSTATUS))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Warning: status instruction size does not match!");
				}

				if (instruction->wCount > 1)
				{
					LOG_LIMIT(100, __FUNCTION__ << " Warning: more than 1 count in set status instruction!");
				}

				switch (status->dwFlags)
				{
				case D3DSETSTATUS_STATUS:
					// Set execute status
					*lpStatus = *status;
					break;
				case D3DSETSTATUS_EXTENTS:
					// ToDo: Set extents status
					break;
				}

				break;
			}
			case D3DOP_BRANCHFORWARD:
				// Enables a branching mechanism within the execute buffer. For more information, see the D3DBRANCH structure.
			{
				// Parse the branch structure
				D3DBRANCH* branch = reinterpret_cast<D3DBRANCH*>(opstruct);

				if (instruction->bSize != sizeof(D3DBRANCH))
				{
					LOG_LIMIT(100, __FUNCTION__ << " Warning: branch instruction size does not match!");
				}

				if (instruction->wCount > 1)
				{
					LOG_LIMIT(100, __FUNCTION__ << " Warning: more than 1 count in branch forward instruction!");
				}

				if (branch->dwMask || branch->dwValue)
				{
					LOG_LIMIT(100, __FUNCTION__ << " Warning: branch forward emulated driver status is not working right!" <<
						" Mask: " << branch->dwMask << " Value: " << branch->dwValue);
				}

				// ToDo: fix implementation of EmulatedDriverStatus

				// Apply the mask to the current status
				DWORD maskedStatus = EmulatedDriverStatus & branch->dwMask;

				// Compare the masked status with the value
				bool condition = (maskedStatus == branch->dwValue);

				// Negate the condition if bNegate is TRUE
				if (branch->bNegate)
				{
					condition = !condition;
				}

				// If the condition is true, branch forward
				if (condition)
				{
					SkipNextMove = true;
					if (branch->dwOffset == 0)
					{
						// Exit the execute buffer if offset is 0
						opcode = D3DOP_EXIT;
						break;
					}
					else
					{
						// Move the instruction pointer forward by the offset
						instructionData += branch->dwOffset;
					}
				}

				// Otherwise, continue to the next instruction
				break;
			}
			case D3DOP_EXIT:
				// Signals that the end of the list has been reached.
				break;
			default:
				// Handle unknown or unsupported opcodes
				LOG_LIMIT(100, __FUNCTION__ << " Warning: Unknown opcode: " << opcode);
				break;
			}

			// Exit loop
			if (opcode == D3DOP_EXIT)
			{
				break;
			}

			// Move to the next instruction
			if (!SkipNextMove)
			{
				instructionData += instructionSize;
			}
		}

		// ToDo: set lpExecuteData->dsStatus status after Execute() has finished

		return D3D_OK;
	}

	if (lpDirect3DExecuteBuffer)
	{
		lpDirect3DExecuteBuffer->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDirect3DExecuteBuffer);
	}
	if (lpDirect3DViewport)
	{
		lpDirect3DViewport->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDirect3DViewport);
	}

	return GetProxyInterfaceV1()->Execute(lpDirect3DExecuteBuffer, lpDirect3DViewport, dwFlags);
}

HRESULT m_IDirect3DDeviceX::AddViewport(LPDIRECT3DVIEWPORT3 lpDirect3DViewport)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// This method will fail, returning DDERR_INVALIDPARAMS, if you attempt to add a viewport that has already been assigned to the device.
		if (!lpDirect3DViewport || IsViewportAttached(lpDirect3DViewport))
		{
			return DDERR_INVALIDPARAMS;
		}

		m_IDirect3DViewportX* lpViewportX = nullptr;
		if (FAILED(lpDirect3DViewport->QueryInterface(IID_GetInterfaceX, (LPVOID*)&lpViewportX)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get ViewportX interface!");
			return DDERR_GENERIC;
		}

		// Associate device with the viewport
		lpViewportX->AddD3DDevice(this);

		AttachedViewports.push_back(lpDirect3DViewport);

		lpDirect3DViewport->AddRef();

		// The first version of the device doesn't have SetCurrentViewport()
		if (ClientDirectXVersion == 1)
		{
			if (AttachedViewports.size() == 1)
			{
				SetCurrentViewport(lpDirect3DViewport);
			}
		}

		return D3D_OK;
	}

	if (lpDirect3DViewport)
	{
		lpDirect3DViewport->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDirect3DViewport);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
		return GetProxyInterfaceV1()->AddViewport(lpDirect3DViewport);
	case 2:
		return GetProxyInterfaceV2()->AddViewport(lpDirect3DViewport);
	case 3:
		return GetProxyInterfaceV3()->AddViewport(lpDirect3DViewport);
	default:
		return DDERR_GENERIC;
	}
}

HRESULT m_IDirect3DDeviceX::DeleteViewport(LPDIRECT3DVIEWPORT3 lpDirect3DViewport)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpDirect3DViewport)
		{
			return DDERR_INVALIDPARAMS;
		}

		m_IDirect3DViewportX* lpViewportX = nullptr;
		if (FAILED(lpDirect3DViewport->QueryInterface(IID_GetInterfaceX, (LPVOID*)&lpViewportX)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get ViewportX interface!");
			return DDERR_GENERIC;
		}

		lpViewportX->ClearD3DDevice(this);

		bool ret = DeleteAttachedViewport(lpDirect3DViewport);

		if (!ret)
		{
			return DDERR_INVALIDPARAMS;
		}

		if (lpDirect3DViewport == lpCurrentViewport)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: deleting active viewport!");

			// Backup old viewport
			m_IDirect3DViewportX* lpOldViewportX = lpCurrentViewportX;

			// Set to null first
			lpCurrentViewport = nullptr;
			lpCurrentViewportX = nullptr;

			// Clear old viewport data
			if (lpOldViewportX)
			{
				lpOldViewportX->ClearCurrentViewport(this, true);
			}
		}

		lpDirect3DViewport->Release();

		return D3D_OK;
	}

	if (lpDirect3DViewport)
	{
		lpDirect3DViewport->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDirect3DViewport);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
		return GetProxyInterfaceV1()->DeleteViewport(lpDirect3DViewport);
	case 2:
		return GetProxyInterfaceV2()->DeleteViewport(lpDirect3DViewport);
	case 3:
		return GetProxyInterfaceV3()->DeleteViewport(lpDirect3DViewport);
	default:
		return DDERR_GENERIC;
	}
}

HRESULT m_IDirect3DDeviceX::NextViewport(LPDIRECT3DVIEWPORT3 lpDirect3DViewport, LPDIRECT3DVIEWPORT3* lplpDirect3DViewport, DWORD dwFlags, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lplpDirect3DViewport || (dwFlags == D3DNEXT_NEXT && !lpDirect3DViewport))
		{
			return DDERR_INVALIDPARAMS;
		}
		*lplpDirect3DViewport = nullptr;

		if (AttachedViewports.size() == 0)
		{
			return D3DERR_NOVIEWPORTS;
		}

		switch (dwFlags)
		{
		case D3DNEXT_HEAD:
			// Retrieve the item at the beginning of the list.
			*lplpDirect3DViewport = AttachedViewports.front();
			break;
		case D3DNEXT_TAIL:
			// Retrieve the item at the end of the list.
			*lplpDirect3DViewport = AttachedViewports.back();
			break;
		case D3DNEXT_NEXT:
			// Retrieve the next item in the list.
			// If you attempt to retrieve the next viewport in the list when you are at the end of the list, this method returns D3D_OK but lplpAnotherViewport is NULL.
			for (UINT x = 1; x < AttachedViewports.size(); x++)
			{
				if (AttachedViewports[x - 1] == lpDirect3DViewport)
				{
					*lplpDirect3DViewport = AttachedViewports[x];
					break;
				}
			}
			break;
		default:
			return DDERR_INVALIDPARAMS;
			break;
		}

		// The first version of the device doesn't have SetCurrentViewport()
		if (DirectXVersion == 1)
		{
			if (*lplpDirect3DViewport)
			{
				SetCurrentViewport(*lplpDirect3DViewport);
			}
		}

		return D3D_OK;
	}

	if (lpDirect3DViewport)
	{
		lpDirect3DViewport->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDirect3DViewport);
	}

	HRESULT hr = DDERR_GENERIC;

	switch (ProxyDirectXVersion)
	{
	case 1:
		hr = GetProxyInterfaceV1()->NextViewport(lpDirect3DViewport, (LPDIRECT3DVIEWPORT*)lplpDirect3DViewport, dwFlags);
		break;
	case 2:
		hr = GetProxyInterfaceV2()->NextViewport(lpDirect3DViewport, (LPDIRECT3DVIEWPORT2*)lplpDirect3DViewport, dwFlags);
		break;
	case 3:
		hr = GetProxyInterfaceV3()->NextViewport(lpDirect3DViewport, lplpDirect3DViewport, dwFlags);
		break;
	}

	if (SUCCEEDED(hr) && lplpDirect3DViewport)
	{
		*lplpDirect3DViewport = ProxyAddressLookupTable.FindAddress<m_IDirect3DViewport3>(*lplpDirect3DViewport, DirectXVersion);
	}

	return hr;
}

HRESULT m_IDirect3DDeviceX::Pick(LPDIRECT3DEXECUTEBUFFER lpDirect3DExecuteBuffer, LPDIRECT3DVIEWPORT lpDirect3DViewport, DWORD dwFlags, LPD3DRECT lpRect)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	if (lpDirect3DExecuteBuffer)
	{
		lpDirect3DExecuteBuffer->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDirect3DExecuteBuffer);
	}
	if (lpDirect3DViewport)
	{
		lpDirect3DViewport->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDirect3DViewport);
	}

	return GetProxyInterfaceV1()->Pick(lpDirect3DExecuteBuffer, lpDirect3DViewport, dwFlags, lpRect);
}

HRESULT m_IDirect3DDeviceX::GetPickRecords(LPDWORD lpCount, LPD3DPICKRECORD lpD3DPickRec)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	return GetProxyInterfaceV1()->GetPickRecords(lpCount, lpD3DPickRec);
}

HRESULT m_IDirect3DDeviceX::EnumTextureFormats(LPD3DENUMTEXTUREFORMATSCALLBACK lpd3dEnumTextureProc, LPVOID lpArg)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpd3dEnumTextureProc)
		{
			return DDERR_INVALIDPARAMS;
		}

		struct EnumPixelFormat
		{
			LPVOID lpContext;
			LPD3DENUMTEXTUREFORMATSCALLBACK lpCallback;

			static HRESULT CALLBACK ConvertCallback(LPDDPIXELFORMAT lpDDPixFmt, LPVOID lpContext)
			{
				EnumPixelFormat* self = (EnumPixelFormat*)lpContext;

				// Only RGB formats are supported
				if ((lpDDPixFmt->dwFlags & DDPF_RGB) == NULL)
				{
					return DDENUMRET_OK;
				}

				DDSURFACEDESC Desc = {};
				Desc.dwSize = sizeof(DDSURFACEDESC);
				Desc.dwFlags = DDSD_CAPS | DDSD_PIXELFORMAT;
				Desc.ddpfPixelFormat = *lpDDPixFmt;
				Desc.ddsCaps.dwCaps = DDSCAPS_TEXTURE;

				return self->lpCallback(&Desc, self->lpContext);
			}
		} CallbackContext = {};
		CallbackContext.lpContext = lpArg;
		CallbackContext.lpCallback = lpd3dEnumTextureProc;

		return EnumTextureFormats(EnumPixelFormat::ConvertCallback, &CallbackContext);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
		return GetProxyInterfaceV1()->EnumTextureFormats(lpd3dEnumTextureProc, lpArg);
	case 2:
		return GetProxyInterfaceV2()->EnumTextureFormats(lpd3dEnumTextureProc, lpArg);
	default:
		return DDERR_GENERIC;
	}
}

HRESULT m_IDirect3DDeviceX::CreateMatrix(LPD3DMATRIXHANDLE lpD3DMatHandle)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpD3DMatHandle)
		{
			return DDERR_INVALIDPARAMS;
		}

		D3DMATRIX Matrix = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};

		D3DMATRIXHANDLE D3DMatHandle = ComputeRND((DWORD)&Matrix, (DWORD)lpD3DMatHandle);

		// Make sure the material handle is unique
		while (D3DMatHandle == NULL || GetMatrix(D3DMatHandle))
		{
			D3DMatHandle += 4;
		}

		MatrixMap[D3DMatHandle] = { true, Matrix };

		*lpD3DMatHandle = D3DMatHandle;

		return D3D_OK;
	}

	return GetProxyInterfaceV1()->CreateMatrix(lpD3DMatHandle);
}

HRESULT m_IDirect3DDeviceX::SetMatrix(D3DMATRIXHANDLE D3DMatHandle, const LPD3DMATRIX lpD3DMatrix)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpD3DMatrix || !GetMatrix(D3DMatHandle))
		{
			return DDERR_INVALIDPARAMS;
		}

		MatrixMap[D3DMatHandle] = { true, *lpD3DMatrix };

		return D3D_OK;
	}

	return GetProxyInterfaceV1()->SetMatrix(D3DMatHandle, lpD3DMatrix);
}

HRESULT m_IDirect3DDeviceX::GetMatrix(D3DMATRIXHANDLE D3DMatHandle, LPD3DMATRIX lpD3DMatrix)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpD3DMatrix || !GetMatrix(D3DMatHandle))
		{
			return DDERR_INVALIDPARAMS;
		}

		*lpD3DMatrix = MatrixMap[D3DMatHandle].m;

		return D3D_OK;
	}

	return GetProxyInterfaceV1()->GetMatrix(D3DMatHandle, lpD3DMatrix);
}

HRESULT m_IDirect3DDeviceX::DeleteMatrix(D3DMATRIXHANDLE D3DMatHandle)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!GetMatrix(D3DMatHandle))
		{
			return DDERR_INVALIDPARAMS;
		}

		MatrixMap.erase(D3DMatHandle);

		return D3D_OK;
	}

	return GetProxyInterfaceV1()->DeleteMatrix(D3DMatHandle);
}

HRESULT m_IDirect3DDeviceX::BeginScene()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_INVALIDOBJECT;
		}

		ScopedCriticalSection ThreadLockDD(DdrawWrapper::GetDDCriticalSection());

		// Set 3D Enabled
		ddrawParent->Enable3D();

		HRESULT hr = (*d3d9Device)->BeginScene();

		if (SUCCEEDED(hr))
		{
			IsInScene = true;

#ifdef ENABLE_PROFILING
			Logging::Log() << __FUNCTION__ << " (" << this << ") hr = " << (D3DERR)hr;
			sceneTime = std::chrono::high_resolution_clock::now();
#endif

			PrepDevice();
		}

		return hr;
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
		return GetProxyInterfaceV1()->BeginScene();
	case 2:
		return GetProxyInterfaceV2()->BeginScene();
	case 3:
		return GetProxyInterfaceV3()->BeginScene();
	case 7:
		return GetProxyInterfaceV7()->BeginScene();
	default:
		return DDERR_GENERIC;
	}
}

HRESULT m_IDirect3DDeviceX::EndScene()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_INVALIDOBJECT;
		}

		// The IDirect3DDevice7::EndScene method ends a scene that was begun by calling the IDirect3DDevice7::BeginScene method.
		// When this method succeeds, the scene has been rendered, and the device surface holds the rendered scene.

		ScopedCriticalSection ThreadLockDD(DdrawWrapper::GetDDCriticalSection());

		HRESULT hr = (*d3d9Device)->EndScene();

		if (SUCCEEDED(hr))
		{
			IsInScene = false;

#ifdef ENABLE_PROFILING
			Logging::Log() << __FUNCTION__ << " (" << this << ") hr = " << (D3DERR)hr << " Timing = " << Logging::GetTimeLapseInMS(sceneTime);
#endif

			m_IDirectDrawSurfaceX* PrimarySurface = ddrawParent->GetPrimarySurface();
			if (!PrimarySurface || FAILED(PrimarySurface->GetFlipStatus(DDGFS_CANFLIP, true)) || PrimarySurface == ddrawParent->GetRenderTargetSurface() || !PrimarySurface->IsRenderTarget())
			{
				ddrawParent->PresentScene(nullptr);
			}
		}

		return hr;
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
		return GetProxyInterfaceV1()->EndScene();
	case 2:
		return GetProxyInterfaceV2()->EndScene();
	case 3:
		return GetProxyInterfaceV3()->EndScene();
	case 7:
		return GetProxyInterfaceV7()->EndScene();
	default:
		return DDERR_GENERIC;
	}
}

HRESULT m_IDirect3DDeviceX::GetDirect3D(LPDIRECT3D7* lplpD3D, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lplpD3D)
		{
			return DDERR_INVALIDPARAMS;
		}
		*lplpD3D = nullptr;

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, false)))
		{
			return DDERR_INVALIDOBJECT;
		}

		m_IDirect3DX** lplpD3DX = ddrawParent->GetCurrentD3D();

		if (!(*lplpD3DX))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: missing Direct3D wrapper!");
			return DDERR_GENERIC;
		}

		*lplpD3D = (LPDIRECT3D7)(*lplpD3DX)->GetWrapperInterfaceX(DirectXVersion);

		if (!(*lplpD3D))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get Direct3D interface!");
			return DDERR_GENERIC;
		}

		(*lplpD3D)->AddRef();

		return D3D_OK;
	}

	HRESULT hr = DDERR_GENERIC;

	switch (ProxyDirectXVersion)
	{
	case 1:
		hr = GetProxyInterfaceV1()->GetDirect3D((LPDIRECT3D*)lplpD3D);
		break;
	case 2:
		hr = GetProxyInterfaceV2()->GetDirect3D((LPDIRECT3D2*)lplpD3D);
		break;
	case 3:
		hr = GetProxyInterfaceV3()->GetDirect3D((LPDIRECT3D3*)lplpD3D);
		break;
	case 7:
		hr = GetProxyInterfaceV7()->GetDirect3D(lplpD3D);
		break;
	}

	if (SUCCEEDED(hr) && lplpD3D)
	{
		*lplpD3D = ProxyAddressLookupTable.FindAddress<m_IDirect3D7>(*lplpD3D, DirectXVersion);
	}

	return hr;
}

// ******************************
// IDirect3DDevice v2 functions
// ******************************

HRESULT m_IDirect3DDeviceX::SetCurrentViewport(LPDIRECT3DVIEWPORT3 lpd3dViewport)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Before calling this method, applications must have already called the AddViewport method to add the viewport to the device.
		if (!lpd3dViewport || !IsViewportAttached(lpd3dViewport))
		{
			return DDERR_INVALIDPARAMS;
		}

		m_IDirect3DViewportX* lpViewportX = nullptr;
		if (FAILED(lpd3dViewport->QueryInterface(IID_GetInterfaceX, (LPVOID*)&lpViewportX)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get ViewportX interface!");
			return DDERR_GENERIC;
		}

		D3DVIEWPORT Viewport = {};
		Viewport.dwSize = sizeof(D3DVIEWPORT);

		HRESULT hr = lpd3dViewport->GetViewport(&Viewport);

		if (SUCCEEDED(hr))
		{
			D3DVIEWPORT7 Viewport7;

			ConvertViewport(Viewport7, Viewport);

			hr = SetViewport(&Viewport7);

			if (SUCCEEDED(hr))
			{
				// Backup old viewport
				m_IDirect3DViewportX* lpOldViewportX = lpCurrentViewportX;

				// Set new viewport first
				lpCurrentViewport = lpd3dViewport;
				lpCurrentViewportX = lpViewportX;

				// Clear old viewport data and apply new viewport data
				if (lpOldViewportX != lpCurrentViewportX)
				{
					if (lpOldViewportX)
					{
						lpOldViewportX->ClearCurrentViewport(this, false);
					}

					lpCurrentViewportX->SetCurrentViewportActive(false, true, true);
				}
			}
		}

		return hr;
	}

	if (lpd3dViewport)
	{
		lpd3dViewport->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpd3dViewport);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	default:
		return DDERR_GENERIC;
	case 2:
		return GetProxyInterfaceV2()->SetCurrentViewport(lpd3dViewport);
	case 3:
		return GetProxyInterfaceV3()->SetCurrentViewport(lpd3dViewport);
	}
}

HRESULT m_IDirect3DDeviceX::GetCurrentViewport(LPDIRECT3DVIEWPORT3* lplpd3dViewport, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lplpd3dViewport)
		{
			return DDERR_INVALIDPARAMS;
		}

		if (!lpCurrentViewport)
		{
			return D3DERR_NOCURRENTVIEWPORT;
		}

		*lplpd3dViewport = lpCurrentViewport;

		lpCurrentViewport->AddRef();

		return D3D_OK;
	}

	HRESULT hr = DDERR_GENERIC;

	switch (ProxyDirectXVersion)
	{
	case 2:
		hr = GetProxyInterfaceV2()->GetCurrentViewport((LPDIRECT3DVIEWPORT2*)lplpd3dViewport);
		break;
	case 3:
		hr = GetProxyInterfaceV3()->GetCurrentViewport(lplpd3dViewport);
		break;
	}

	if (SUCCEEDED(hr) && lplpd3dViewport)
	{
		*lplpd3dViewport = ProxyAddressLookupTable.FindAddress<m_IDirect3DViewport3>(*lplpd3dViewport, DirectXVersion);
	}

	return hr;
}

HRESULT m_IDirect3DDeviceX::SetRenderTarget(LPDIRECTDRAWSURFACE7 lpNewRenderTarget, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpNewRenderTarget)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Don't reset existing render target
		if (CurrentRenderTarget == lpNewRenderTarget)
		{
			return D3D_OK;
		}

		// dwFlags: Not currently used; set to 0.

		// ToDo: if DirectXVersion < 7 then invalidate the current material and viewport:
		// Unlike this method's implementation in previous interfaces, IDirect3DDevice7::SetRenderTarget does not invalidate the current material or viewport for the device.

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_INVALIDOBJECT;
		}

		m_IDirectDrawSurfaceX* lpDDSrcSurfaceX = nullptr;
		lpNewRenderTarget->QueryInterface(IID_GetInterfaceX, (LPVOID*)&lpDDSrcSurfaceX);

		if (!lpDDSrcSurfaceX)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get surface wrapper!");
			return DDERR_INVALIDPARAMS;
		}

		ScopedCriticalSection ThreadLockDD(DdrawWrapper::GetDDCriticalSection());

		HRESULT hr = ddrawParent->SetRenderTargetSurface(lpDDSrcSurfaceX);

		if (SUCCEEDED(hr))
		{
			if (CurrentRenderTarget)
			{
				CurrentRenderTarget->Release();
			}

			CurrentRenderTarget = lpNewRenderTarget;

			lpCurrentRenderTargetX = lpDDSrcSurfaceX;

			CurrentRenderTarget->AddRef();
		}

		return D3D_OK;
	}

	if (lpNewRenderTarget)
	{
		lpNewRenderTarget->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpNewRenderTarget);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	default:
		return DDERR_GENERIC;
	case 2:
		return GetProxyInterfaceV2()->SetRenderTarget((LPDIRECTDRAWSURFACE)lpNewRenderTarget, dwFlags);
	case 3:
		return GetProxyInterfaceV3()->SetRenderTarget((LPDIRECTDRAWSURFACE4)lpNewRenderTarget, dwFlags);
	case 7:
		return GetProxyInterfaceV7()->SetRenderTarget(lpNewRenderTarget, dwFlags);
	}
}

HRESULT m_IDirect3DDeviceX::GetRenderTarget(LPDIRECTDRAWSURFACE7* lplpRenderTarget, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lplpRenderTarget)
		{
			return DDERR_INVALIDPARAMS;
		}

		if (!CurrentRenderTarget)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: render target not set.");
			return DDERR_GENERIC;
		}

		*lplpRenderTarget = CurrentRenderTarget;

		CurrentRenderTarget->AddRef();

		return D3D_OK;
	}

	HRESULT hr = DDERR_GENERIC;

	switch (ProxyDirectXVersion)
	{
	case 2:
		hr = GetProxyInterfaceV2()->GetRenderTarget((LPDIRECTDRAWSURFACE*)lplpRenderTarget);
		break;
	case 3:
		hr = GetProxyInterfaceV3()->GetRenderTarget((LPDIRECTDRAWSURFACE4*)lplpRenderTarget);
		break;
	case 7:
		hr = GetProxyInterfaceV7()->GetRenderTarget(lplpRenderTarget);
		break;
	}

	if (SUCCEEDED(hr) && lplpRenderTarget)
	{
		*lplpRenderTarget = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(*lplpRenderTarget, DirectXVersion);
	}

	return hr;
}

HRESULT m_IDirect3DDeviceX::Begin(D3DPRIMITIVETYPE d3dpt, DWORD d3dvt, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	default:
		return DDERR_GENERIC;
	case 2:
		return GetProxyInterfaceV2()->Begin(d3dpt, (D3DVERTEXTYPE)d3dvt, dwFlags);
	case 3:
		return GetProxyInterfaceV3()->Begin(d3dpt, d3dvt, dwFlags);
	}
}

HRESULT m_IDirect3DDeviceX::BeginIndexed(D3DPRIMITIVETYPE dptPrimitiveType, DWORD dvtVertexType, LPVOID lpvVertices, DWORD dwNumVertices, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	default:
		return DDERR_GENERIC;
	case 2:
		return GetProxyInterfaceV2()->BeginIndexed(dptPrimitiveType, (D3DVERTEXTYPE)dvtVertexType, lpvVertices, dwNumVertices, dwFlags);
	case 3:
		return GetProxyInterfaceV3()->BeginIndexed(dptPrimitiveType, dvtVertexType, lpvVertices, dwNumVertices, dwFlags);
	}
}

HRESULT m_IDirect3DDeviceX::Vertex(LPVOID lpVertexType)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	default:
		return DDERR_GENERIC;
	case 2:
		return GetProxyInterfaceV2()->Vertex(lpVertexType);
	case 3:
		return GetProxyInterfaceV3()->Vertex(lpVertexType);
	}
}

HRESULT m_IDirect3DDeviceX::Index(WORD wVertexIndex)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	default:
		return DDERR_GENERIC;
	case 2:
		return GetProxyInterfaceV2()->Index(wVertexIndex);
	case 3:
		return GetProxyInterfaceV3()->Index(wVertexIndex);
	}
}

HRESULT m_IDirect3DDeviceX::End(DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	default:
		return DDERR_GENERIC;
	case 2:
		return GetProxyInterfaceV2()->End(dwFlags);
	case 3:
		return GetProxyInterfaceV3()->End(dwFlags);
	}
}

HRESULT m_IDirect3DDeviceX::GetRenderState(D3DRENDERSTATETYPE dwRenderStateType, LPDWORD lpdwRenderState)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") " << dwRenderStateType;

	if (Config.Dd7to9)
	{
		if (!lpdwRenderState)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			*lpdwRenderState = 0;
			return DDERR_INVALIDOBJECT;
		}

		if (!IsValidRenderState(dwRenderStateType, *lpdwRenderState, ClientDirectXVersion))
		{
			if (dwRenderStateType > 0 && dwRenderStateType < D3D_MAXRENDERSTATES && ClientDirectXVersion < 7)
			{
				*lpdwRenderState = DeviceStates.RenderState[dwRenderStateType].State;
				return D3D_OK;
			}
			else
			{
				*lpdwRenderState = 0;
				return DDERR_INVALIDPARAMS;
			}
		}

		if (dwRenderStateType > 95 && (dwRenderStateType < 128 || IsOutOfRangeRenderState(dwRenderStateType, ClientDirectXVersion)))
		{
			if (dwRenderStateType < D3D_MAXRENDERSTATES)
			{
				if (OverloadedD9RenderState(dwRenderStateType))
				{
					*lpdwRenderState = DeviceStates.rsMap[dwRenderStateType];
				}
				else
				{
					*lpdwRenderState = DeviceStates.RenderState[dwRenderStateType].State;
				}
				return D3D_OK;
			}
			else
			{
				*lpdwRenderState = (DWORD)-1;
				return DDERR_INVALIDPARAMS;
			}
		}

		switch ((DWORD)dwRenderStateType)
		{
		case D3DRENDERSTATE_FOGTABLEMODE:		// 35
		case D3DRENDERSTATE_FOGSTART:			// 36
		case D3DRENDERSTATE_FOGEND:				// 37
		case D3DRENDERSTATE_FOGDENSITY:			// 38
		case D3DRENDERSTATE_AMBIENT:			// 139
		case D3DRENDERSTATE_FOGVERTEXMODE:		// 140
		case D3DRENDERSTATE_COLORVERTEX:		// 141
			*lpdwRenderState = DeviceStates.rsMap[dwRenderStateType];
			return D3D_OK;
		case D3DRENDERSTATE_NONE:
		case D3DRENDERSTATE_TEXTUREHANDLE:		// 1
		case D3DRENDERSTATE_ANTIALIAS:			// 2
		case D3DRENDERSTATE_TEXTUREADDRESS:		// 3
		case D3DRENDERSTATE_TEXTUREPERSPECTIVE:	// 4
		case D3DRENDERSTATE_WRAPU:				// 5
		case D3DRENDERSTATE_WRAPV:				// 6
		case D3DRENDERSTATE_LINEPATTERN:		// 10
		case D3DRENDERSTATE_MONOENABLE:			// 11
		case D3DRENDERSTATE_ROP2:				// 12
		case D3DRENDERSTATE_PLANEMASK:			// 13
		case D3DRENDERSTATE_TEXTUREMAG:			// 17
		case D3DRENDERSTATE_TEXTUREMIN:			// 18
		case D3DRENDERSTATE_TEXTUREMAPBLEND:	// 21
		case D3DRENDERSTATE_ZVISIBLE:			// 30
		case D3DRENDERSTATE_SUBPIXEL:			// 31
		case D3DRENDERSTATE_SUBPIXELX:			// 32
		case D3DRENDERSTATE_STIPPLEDALPHA:		// 33
		case D3DRENDERSTATE_STIPPLEENABLE:		// 39
		case D3DRENDERSTATE_EDGEANTIALIAS:		// 40
		case D3DRENDERSTATE_COLORKEYENABLE:		// 41
		case D3DRENDERSTATE_OLDALPHABLENDENABLE:// 42
		case D3DRENDERSTATE_BORDERCOLOR:		// 43
		case D3DRENDERSTATE_TEXTUREADDRESSU:	// 44
		case D3DRENDERSTATE_TEXTUREADDRESSV:	// 45
		case D3DRENDERSTATE_MIPMAPLODBIAS:		// 46
		case D3DRENDERSTATE_ZBIAS:				// 47
		case D3DRENDERSTATE_ANISOTROPY:			// 49
		case D3DRENDERSTATE_FLUSHBATCH:			// 50
		case D3DRENDERSTATE_TRANSLUCENTSORTINDEPENDENT:	// 51
		case 61:
		case 62:
		case 63:
		case D3DRENDERSTATE_STIPPLEPATTERN00:	// 64
		case D3DRENDERSTATE_STIPPLEPATTERN01:	// 65
		case D3DRENDERSTATE_STIPPLEPATTERN02:	// 66
		case D3DRENDERSTATE_STIPPLEPATTERN03:	// 67
		case D3DRENDERSTATE_STIPPLEPATTERN04:	// 68
		case D3DRENDERSTATE_STIPPLEPATTERN05:	// 69
		case D3DRENDERSTATE_STIPPLEPATTERN06:	// 70
		case D3DRENDERSTATE_STIPPLEPATTERN07:	// 71
		case D3DRENDERSTATE_STIPPLEPATTERN08:	// 72
		case D3DRENDERSTATE_STIPPLEPATTERN09:	// 73
		case D3DRENDERSTATE_STIPPLEPATTERN10:	// 74
		case D3DRENDERSTATE_STIPPLEPATTERN11:	// 75
		case D3DRENDERSTATE_STIPPLEPATTERN12:	// 76
		case D3DRENDERSTATE_STIPPLEPATTERN13:	// 77
		case D3DRENDERSTATE_STIPPLEPATTERN14:	// 78
		case D3DRENDERSTATE_STIPPLEPATTERN15:	// 79
		case D3DRENDERSTATE_STIPPLEPATTERN16:	// 80
		case D3DRENDERSTATE_STIPPLEPATTERN17:	// 81
		case D3DRENDERSTATE_STIPPLEPATTERN18:	// 82
		case D3DRENDERSTATE_STIPPLEPATTERN19:	// 83
		case D3DRENDERSTATE_STIPPLEPATTERN20:	// 84
		case D3DRENDERSTATE_STIPPLEPATTERN21:	// 85
		case D3DRENDERSTATE_STIPPLEPATTERN22:	// 86
		case D3DRENDERSTATE_STIPPLEPATTERN23:	// 87
		case D3DRENDERSTATE_STIPPLEPATTERN24:	// 88
		case D3DRENDERSTATE_STIPPLEPATTERN25:	// 89
		case D3DRENDERSTATE_STIPPLEPATTERN26:	// 90
		case D3DRENDERSTATE_STIPPLEPATTERN27:	// 91
		case D3DRENDERSTATE_STIPPLEPATTERN28:	// 92
		case D3DRENDERSTATE_STIPPLEPATTERN29:	// 93
		case D3DRENDERSTATE_STIPPLEPATTERN30:	// 94
		case D3DRENDERSTATE_STIPPLEPATTERN31:	// 95
		case D3DRENDERSTATE_EXTENTS:			// 138
		case D3DRENDERSTATE_COLORKEYBLENDENABLE:// 144
			*lpdwRenderState = DeviceStates.RenderState[dwRenderStateType].State;
			return D3D_OK;
		}

		return GetD9RenderState(dwRenderStateType, lpdwRenderState);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	default:
		return DDERR_GENERIC;
	case 2:
		return GetProxyInterfaceV2()->GetRenderState(dwRenderStateType, lpdwRenderState);
	case 3:
		return GetProxyInterfaceV3()->GetRenderState(dwRenderStateType, lpdwRenderState);
	case 7:
		return GetProxyInterfaceV7()->GetRenderState(dwRenderStateType, lpdwRenderState);
	}
}

HRESULT m_IDirect3DDeviceX::SetRenderState(D3DRENDERSTATETYPE dwRenderStateType, DWORD dwRenderState)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") " << dwRenderStateType << " " << dwRenderState;

	if (Config.Dd7to9)
	{
		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_INVALIDOBJECT;
		}

		if (DWORD Value; !IsValidRenderState(dwRenderStateType, Value, ClientDirectXVersion))
		{
			return DDERR_INVALIDPARAMS;
		}

		if (dwRenderStateType > 95 && (dwRenderStateType < 128 || IsOutOfRangeRenderState(dwRenderStateType, ClientDirectXVersion)))
		{
			if (dwRenderStateType < D3D_MAXRENDERSTATES)
			{
				if (OverloadedD9RenderState(dwRenderStateType))
				{
					SetRenderStateMap(dwRenderStateType, dwRenderState);
				}
				else
				{
					DeviceStates.RenderState[dwRenderStateType].State = dwRenderState;
				}
				return D3D_OK;
			}
			else
			{
				return DDERR_INVALIDPARAMS;
			}
		}

		switch ((DWORD)dwRenderStateType)
		{
		case D3DRENDERSTATE_FOGTABLEMODE:		// 35
		case D3DRENDERSTATE_FOGSTART:			// 36
		case D3DRENDERSTATE_FOGEND:				// 37
		case D3DRENDERSTATE_FOGDENSITY:			// 38
		case D3DRENDERSTATE_AMBIENT:			// 139
		case D3DRENDERSTATE_FOGVERTEXMODE:		// 140
		case D3DRENDERSTATE_COLORVERTEX:		// 141
			SetRenderStateMap(dwRenderStateType, dwRenderState);
			break;
		case D3DRENDERSTATE_NONE:
			DeviceStates.RenderState[dwRenderStateType].State = dwRenderState;
			return D3D_OK;
		case D3DRENDERSTATE_TEXTUREHANDLE:		// 1
			DeviceStates.RenderState[dwRenderStateType].State = dwRenderState;
			return SetTextureHandle(dwRenderState);
		case D3DRENDERSTATE_ANTIALIAS:			// 2
			rsAntiAliasChanged = true;
			DeviceStates.RenderState[dwRenderStateType].State = dwRenderState;
			return D3D_OK;
		case D3DRENDERSTATE_TEXTUREADDRESS:		// 3
			DeviceStates.RenderState[dwRenderStateType].State = dwRenderState;
			SetD9SamplerState(0, D3DSAMP_ADDRESSU, dwRenderState);
			return SetD9SamplerState(0, D3DSAMP_ADDRESSV, dwRenderState);
		case D3DRENDERSTATE_TEXTUREPERSPECTIVE:	// 4
			DeviceStates.RenderState[dwRenderStateType].State = dwRenderState;
			if (dwRenderState != FALSE)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DRENDERSTATE_TEXTUREPERSPECTIVE' not implemented: " << dwRenderState);
			}
			return D3D_OK;
		case D3DRENDERSTATE_WRAPU:				// 5
		case D3DRENDERSTATE_WRAPV:				// 6
			rsWrapChanged = true;
			DeviceStates.RenderState[dwRenderStateType].State = dwRenderState;
			return D3D_OK;
		case D3DRENDERSTATE_LINEPATTERN:		// 10
			DeviceStates.RenderState[dwRenderStateType].State = dwRenderState;
			if (dwRenderState != 0)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DRENDERSTATE_LINEPATTERN' not implemented: " << dwRenderState);
			}
			return D3D_OK;
		case D3DRENDERSTATE_MONOENABLE:			// 11
			DeviceStates.RenderState[dwRenderStateType].State = dwRenderState;
			if (dwRenderState != FALSE)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DRENDERSTATE_MONOENABLE' not implemented: " << dwRenderState);
			}
			return D3D_OK;
		case D3DRENDERSTATE_ROP2:				// 12
			DeviceStates.RenderState[dwRenderStateType].State = dwRenderState;
			if (dwRenderState != R2_COPYPEN)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DRENDERSTATE_ROP2' not implemented: " << dwRenderState);
			}
			return D3D_OK;
		case D3DRENDERSTATE_PLANEMASK:			// 13
			DeviceStates.RenderState[dwRenderStateType].State = dwRenderState;
			if (dwRenderState != ~0)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DRENDERSTATE_PLANEMASK' not implemented: " << dwRenderState);
			}
			return D3D_OK;
		case D3DRENDERSTATE_TEXTUREMAG:			// 17
			DeviceStates.RenderState[dwRenderStateType].State = dwRenderState;
			switch (dwRenderState)
			{
			// Only the first two (D3DFILTER_NEAREST and D3DFILTER_LINEAR) are valid with D3DRENDERSTATE_TEXTUREMAG.
			case D3DFILTER_NEAREST:
			case D3DFILTER_LINEAR:
				return SetD9SamplerState(0, D3DSAMP_MAGFILTER, dwRenderState);
			default:
				LOG_LIMIT(100, __FUNCTION__ << " Warning: unsupported 'D3DRENDERSTATE_TEXTUREMAG' state: " << dwRenderState);
				return D3D_OK;
			}
		case D3DRENDERSTATE_TEXTUREMIN:			// 18
			DeviceStates.RenderState[dwRenderStateType].State = dwRenderState;
			switch (dwRenderState)
			{
			case D3DFILTER_NEAREST:
			case D3DFILTER_LINEAR:
				SetD9SamplerState(0, D3DSAMP_MINFILTER, dwRenderState);
				return SetD9SamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
			case D3DFILTER_MIPNEAREST:
				SetD9SamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
				return SetD9SamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
			case D3DFILTER_MIPLINEAR:
				SetD9SamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
				return SetD9SamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
			case D3DFILTER_LINEARMIPNEAREST:
				SetD9SamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
				return SetD9SamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
			case D3DFILTER_LINEARMIPLINEAR:
				SetD9SamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
				return SetD9SamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
			default:
				LOG_LIMIT(100, __FUNCTION__ << " Warning: unsupported 'D3DRENDERSTATE_TEXTUREMIN' state: " << dwRenderState);
				return D3D_OK;
			}
		case D3DRENDERSTATE_TEXTUREMAPBLEND:	// 21
			DeviceStates.RenderState[dwRenderStateType].State = dwRenderState;
			switch (dwRenderState)
			{
			case D3DTBLEND_COPY:
			case D3DTBLEND_DECAL:
				// Reset states
				SetD9TextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
				SetD9TextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
				SetD9TextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
				SetD9TextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_CURRENT);

				// Decal texture-blending mode is supported. In this mode, the RGB and alpha values of the texture replace the colors that would have been used with no texturing.
				SetD9TextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
				SetD9TextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

				return D3D_OK;
			case D3DTBLEND_DECALALPHA:
				// Reset states
				SetD9TextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
				SetD9TextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

				// Decal-alpha texture-blending mode is supported. In this mode, the RGB and alpha values of the texture are 
				// blended with the colors that would have been used with no texturing.
				SetD9TextureStageState(0, D3DTSS_COLOROP, D3DTOP_BLENDTEXTUREALPHA);
				SetD9TextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
				SetD9TextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
				SetD9TextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

				return D3D_OK;
			case D3DTBLEND_DECALMASK:
				// This blending mode is not supported. When the least-significant bit of the texture's alpha component is zero, 
				// the effect is as if texturing were disabled.
				LOG_LIMIT(100, __FUNCTION__ << " Warning: unsupported 'D3DTBLEND_DECALMASK' state: " << dwRenderState);

				return D3D_OK;
			case D3DTBLEND_MODULATE:
				// Reset states
				SetD9TextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
				SetD9TextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

				// Modulate texture-blending mode is supported. In this mode, the RGB values of the texture are multiplied 
				// with the RGB values that would have been used with no texturing. Any alpha values in the texture replace 
				// the alpha values in the colors that would have been used with no texturing; if the texture does not contain 
				// an alpha component, alpha values at the vertices in the source are interpolated between vertices.
				SetD9TextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
				SetD9TextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
				SetD9TextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
				SetD9TextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

				return D3D_OK;
			case D3DTBLEND_MODULATEALPHA:
				// Reset states
				SetD9TextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
				SetD9TextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

				// Modulate-alpha texture-blending mode is supported. In this mode, the RGB values of the texture are multiplied 
				// with the RGB values that would have been used with no texturing, and the alpha values of the texture are multiplied 
				// with the alpha values that would have been used with no texturing.
				SetD9TextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
				SetD9TextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
				SetD9TextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
				SetD9TextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

				return D3D_OK;
			case D3DTBLEND_MODULATEMASK:
				// This blending mode is not supported. When the least-significant bit of the texture's alpha component is zero, 
				// the effect is as if texturing were disabled.
				LOG_LIMIT(100, __FUNCTION__ << " Warning: unsupported 'D3DTBLEND_MODULATEMASK' state: " << dwRenderState);

				return D3D_OK;
			case D3DTBLEND_ADD:
				// Reset states
				SetD9TextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
				SetD9TextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

				// Add the Gouraud interpolants to the texture lookup with saturation semantics
				// (that is, if the color value overflows it is set to the maximum possible value).
				SetD9TextureStageState(0, D3DTSS_COLOROP, D3DTOP_ADD);
				SetD9TextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
				SetD9TextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
				SetD9TextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

				return D3D_OK;
			default:
				LOG_LIMIT(100, __FUNCTION__ << " Warning: unsupported 'D3DRENDERSTATE_TEXTUREMAPBLEND' state: " << dwRenderState);
				return D3D_OK;
			}
		case D3DRENDERSTATE_ALPHAREF:			// 24
			dwRenderState &= 0xFF;
			break;
		case D3DRENDERSTATE_ZVISIBLE:			// 30
			DeviceStates.RenderState[dwRenderStateType].State = dwRenderState;
			if (dwRenderState != FALSE)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DRENDERSTATE_ZVISIBLE' not implemented: " << dwRenderState);
			}
			return D3D_OK;
		case D3DRENDERSTATE_SUBPIXEL:			// 31
			DeviceStates.RenderState[dwRenderStateType].State = dwRenderState;
			if (dwRenderState != FALSE)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DRENDERSTATE_SUBPIXEL' not implemented: " << dwRenderState);
			}
			return D3D_OK;
		case D3DRENDERSTATE_SUBPIXELX:			// 32
			DeviceStates.RenderState[dwRenderStateType].State = dwRenderState;
			if (dwRenderState != FALSE)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DRENDERSTATE_SUBPIXELX' not implemented: " << dwRenderState);
			}
			return D3D_OK;
		case D3DRENDERSTATE_STIPPLEDALPHA:		// 33
			DeviceStates.RenderState[dwRenderStateType].State = dwRenderState;
			if (dwRenderState != FALSE)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DRENDERSTATE_STIPPLEDALPHA' not implemented! " << dwRenderState);
			}
			return D3D_OK;
		case D3DRENDERSTATE_STIPPLEENABLE:		// 39
			DeviceStates.RenderState[dwRenderStateType].State = dwRenderState;
			if (dwRenderState != FALSE)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DRENDERSTATE_STIPPLEENABLE' not implemented! " << dwRenderState);
			}
			return D3D_OK;
		case D3DRENDERSTATE_EDGEANTIALIAS:		// 40
			rsAntiAliasChanged = true;
			DeviceStates.RenderState[dwRenderStateType].State = dwRenderState;
			return D3D_OK;
		case D3DRENDERSTATE_COLORKEYENABLE:		// 41
			DeviceStates.RenderState[dwRenderStateType].State = dwRenderState;
			return D3D_OK;
		case D3DRENDERSTATE_OLDALPHABLENDENABLE:// 42
			DeviceStates.RenderState[dwRenderStateType].State = dwRenderState;
			if (dwRenderState != FALSE)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DRENDERSTATE_OLDALPHABLENDENABLE' not implemented! " << dwRenderState);
			}
			return D3D_OK;
		case D3DRENDERSTATE_BORDERCOLOR:		// 43
			DeviceStates.RenderState[dwRenderStateType].State = dwRenderState;
			return SetD9SamplerState(0, D3DSAMP_BORDERCOLOR, dwRenderState);
		case D3DRENDERSTATE_TEXTUREADDRESSU:	// 44
			DeviceStates.RenderState[dwRenderStateType].State = dwRenderState;
			return SetD9SamplerState(0, D3DSAMP_ADDRESSU, dwRenderState);
		case D3DRENDERSTATE_TEXTUREADDRESSV:	// 45
			DeviceStates.RenderState[dwRenderStateType].State = dwRenderState;
			return SetD9SamplerState(0, D3DSAMP_ADDRESSV, dwRenderState);
		case D3DRENDERSTATE_MIPMAPLODBIAS:		// 46
			DeviceStates.RenderState[dwRenderStateType].State = dwRenderState;
			return SetD9SamplerState(0, D3DSAMP_MIPMAPLODBIAS, dwRenderState);
		case D3DRENDERSTATE_ZBIAS:				// 47
		{
			DeviceStates.RenderState[dwRenderStateType].State = dwRenderState;
			FLOAT Biased = static_cast<FLOAT>(dwRenderState) * -0.000005f;
			dwRenderState = *reinterpret_cast<const DWORD*>(&Biased);
			dwRenderStateType = D3DRS_DEPTHBIAS;
			break;
		}
		case D3DRENDERSTATE_ANISOTROPY:			// 49
			DeviceStates.RenderState[dwRenderStateType].State = dwRenderState;
			return SetD9SamplerState(0, D3DSAMP_MAXANISOTROPY, dwRenderState);
		case D3DRENDERSTATE_FLUSHBATCH:			// 50
			DeviceStates.RenderState[dwRenderStateType].State = dwRenderState;
			if (dwRenderState != FALSE)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DRENDERSTATE_FLUSHBATCH' not implemented! " << dwRenderState);
			}
			return D3D_OK;
		case D3DRENDERSTATE_TRANSLUCENTSORTINDEPENDENT:	// 51
			DeviceStates.RenderState[dwRenderStateType].State = dwRenderState;
			if (dwRenderState != FALSE)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DRENDERSTATE_TRANSLUCENTSORTINDEPENDENT' not implemented! " << dwRenderState);
			}
			return D3D_OK;
		case 61:
		case 62:
		case 63:
		case D3DRENDERSTATE_STIPPLEPATTERN00:	// 64
		case D3DRENDERSTATE_STIPPLEPATTERN01:	// 65
		case D3DRENDERSTATE_STIPPLEPATTERN02:	// 66
		case D3DRENDERSTATE_STIPPLEPATTERN03:	// 67
		case D3DRENDERSTATE_STIPPLEPATTERN04:	// 68
		case D3DRENDERSTATE_STIPPLEPATTERN05:	// 69
		case D3DRENDERSTATE_STIPPLEPATTERN06:	// 70
		case D3DRENDERSTATE_STIPPLEPATTERN07:	// 71
		case D3DRENDERSTATE_STIPPLEPATTERN08:	// 72
		case D3DRENDERSTATE_STIPPLEPATTERN09:	// 73
		case D3DRENDERSTATE_STIPPLEPATTERN10:	// 74
		case D3DRENDERSTATE_STIPPLEPATTERN11:	// 75
		case D3DRENDERSTATE_STIPPLEPATTERN12:	// 76
		case D3DRENDERSTATE_STIPPLEPATTERN13:	// 77
		case D3DRENDERSTATE_STIPPLEPATTERN14:	// 78
		case D3DRENDERSTATE_STIPPLEPATTERN15:	// 79
		case D3DRENDERSTATE_STIPPLEPATTERN16:	// 80
		case D3DRENDERSTATE_STIPPLEPATTERN17:	// 81
		case D3DRENDERSTATE_STIPPLEPATTERN18:	// 82
		case D3DRENDERSTATE_STIPPLEPATTERN19:	// 83
		case D3DRENDERSTATE_STIPPLEPATTERN20:	// 84
		case D3DRENDERSTATE_STIPPLEPATTERN21:	// 85
		case D3DRENDERSTATE_STIPPLEPATTERN22:	// 86
		case D3DRENDERSTATE_STIPPLEPATTERN23:	// 87
		case D3DRENDERSTATE_STIPPLEPATTERN24:	// 88
		case D3DRENDERSTATE_STIPPLEPATTERN25:	// 89
		case D3DRENDERSTATE_STIPPLEPATTERN26:	// 90
		case D3DRENDERSTATE_STIPPLEPATTERN27:	// 91
		case D3DRENDERSTATE_STIPPLEPATTERN28:	// 92
		case D3DRENDERSTATE_STIPPLEPATTERN29:	// 93
		case D3DRENDERSTATE_STIPPLEPATTERN30:	// 94
		case D3DRENDERSTATE_STIPPLEPATTERN31:	// 95
			DeviceStates.RenderState[dwRenderStateType].State = dwRenderState;
			if (dwRenderState != 0)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DRENDERSTATE_STIPPLEPATTERN00' not implemented! " << dwRenderState);
			}
			return D3D_OK;
		case D3DRENDERSTATE_EXTENTS:			// 138
			// ToDo: use this to enable/disable clip plane extents set by SetClipStatus()
			DeviceStates.RenderState[dwRenderStateType].State = dwRenderState;
			if (dwRenderState != FALSE)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DRENDERSTATE_EXTENTS' not implemented! " << dwRenderState);
			}
			return D3D_OK;
		case D3DRENDERSTATE_COLORKEYBLENDENABLE:// 144
			DeviceStates.RenderState[dwRenderStateType].State = dwRenderState;
			if (dwRenderState != FALSE)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DRENDERSTATE_COLORKEYBLENDENABLE' not implemented! " << dwRenderState);
			}
			return D3D_OK;
		}

		return SetD9RenderState(dwRenderStateType, dwRenderState);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	default:
		return DDERR_GENERIC;
	case 2:
		return GetProxyInterfaceV2()->SetRenderState(dwRenderStateType, dwRenderState);
	case 3:
		return GetProxyInterfaceV3()->SetRenderState(dwRenderStateType, dwRenderState);
	case 7:
		return GetProxyInterfaceV7()->SetRenderState(dwRenderStateType, dwRenderState);
	}
}

HRESULT m_IDirect3DDeviceX::GetLightState(D3DLIGHTSTATETYPE dwLightStateType, LPDWORD lpdwLightState)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpdwLightState)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: Light state called with nullptr: " << dwLightStateType);
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			*lpdwLightState = 0;
			return DDERR_INVALIDOBJECT;
		}

		if (dwLightStateType == 0
			|| (ClientDirectXVersion == 2 && dwLightStateType > 7)
			|| (ClientDirectXVersion == 3 && dwLightStateType > 8))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: unknown LightStateType: " << dwLightStateType);
			*lpdwLightState = 0;
			return DDERR_INVALIDPARAMS;
		}

		switch (dwLightStateType)
		{
		case D3DLIGHTSTATE_MATERIAL:
		case D3DLIGHTSTATE_AMBIENT:
		case D3DLIGHTSTATE_COLORMODEL:
		case D3DLIGHTSTATE_FOGMODE:
		case D3DLIGHTSTATE_FOGSTART:
		case D3DLIGHTSTATE_FOGEND:
		case D3DLIGHTSTATE_FOGDENSITY:
		case D3DLIGHTSTATE_COLORVERTEX:
			*lpdwLightState = DeviceStates.lsMap[dwLightStateType];
			return D3D_OK;
		default:
			break;
		}

		LOG_LIMIT(100, __FUNCTION__ << " Error: unknown LightStateType: " << dwLightStateType);
		*lpdwLightState = 0;
		return DDERR_INVALIDPARAMS;
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	default:
		return DDERR_GENERIC;
	case 2:
		return GetProxyInterfaceV2()->GetLightState(dwLightStateType, lpdwLightState);
	case 3:
		return GetProxyInterfaceV3()->GetLightState(dwLightStateType, lpdwLightState);
	}
}

HRESULT m_IDirect3DDeviceX::SetLightState(D3DLIGHTSTATETYPE dwLightStateType, DWORD dwLightState)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") " << dwLightStateType << " " << dwLightState;

	if (Config.Dd7to9)
	{
		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_INVALIDOBJECT;
		}

		if (dwLightStateType == 0
			|| (ClientDirectXVersion == 2 && dwLightStateType > 7)
			|| (ClientDirectXVersion == 3 && dwLightStateType > 8))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: unknown LightStateType: " << dwLightStateType);
			return DDERR_INVALIDPARAMS;
		}

		D3DRENDERSTATETYPE RenderState = (D3DRENDERSTATETYPE)0;
		switch (dwLightStateType)
		{
		case D3DLIGHTSTATE_MATERIAL:
			SetLightStateMap(dwLightStateType, dwLightState);
			return SetMaterialHandle(dwLightState);
		case D3DLIGHTSTATE_AMBIENT:
			RenderState = D3DRS_AMBIENT;
			SetLightStateMap(dwLightStateType, dwLightState);
			break;
		case D3DLIGHTSTATE_COLORMODEL:
			if (dwLightState != D3DCOLOR_RGB)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DLIGHTSTATE_COLORMODEL' not implemented! " << dwLightState);
			}
			SetLightStateMap(dwLightStateType, dwLightState);
			return D3D_OK;
		case D3DLIGHTSTATE_FOGMODE:
			RenderState = D3DRS_FOGTABLEMODE;
			SetLightStateMap(dwLightStateType, dwLightState);
			break;
		case D3DLIGHTSTATE_FOGSTART:
			RenderState = D3DRS_FOGSTART;
			SetLightStateMap(dwLightStateType, dwLightState);
			break;
		case D3DLIGHTSTATE_FOGEND:
			RenderState = D3DRS_FOGEND;
			SetLightStateMap(dwLightStateType, dwLightState);
			break;
		case D3DLIGHTSTATE_FOGDENSITY:
			RenderState = D3DRS_FOGDENSITY;
			SetLightStateMap(dwLightStateType, dwLightState);
			break;
		case D3DLIGHTSTATE_COLORVERTEX:
			RenderState = D3DRS_COLORVERTEX;
			SetLightStateMap(dwLightStateType, dwLightState);
			break;
		default:
			break;
		}

		return SetD9RenderState(RenderState, dwLightState);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	default:
		return DDERR_GENERIC;
	case 2:
		return GetProxyInterfaceV2()->SetLightState(dwLightStateType, dwLightState);
	case 3:
		return GetProxyInterfaceV3()->SetLightState(dwLightStateType, dwLightState);
	}
}

HRESULT m_IDirect3DDeviceX::SetTransform(D3DTRANSFORMSTATETYPE dtstTransformStateType, LPD3DMATRIX lpD3DMatrix)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpD3DMatrix)
		{
			return  DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_INVALIDOBJECT;
		}

		switch ((DWORD)dtstTransformStateType)
		{
		case D3DTRANSFORMSTATE_WORLD:
			dtstTransformStateType = D3DTS_WORLD;
			break;
		case D3DTRANSFORMSTATE_WORLD1:
			dtstTransformStateType = D3DTS_WORLD1;
			break;
		case D3DTRANSFORMSTATE_WORLD2:
			dtstTransformStateType = D3DTS_WORLD2;
			break;
		case D3DTRANSFORMSTATE_WORLD3:
			dtstTransformStateType = D3DTS_WORLD3;
			break;
		}

		HRESULT hr = SetD9Transform(dtstTransformStateType, lpD3DMatrix);

		if (SUCCEEDED(hr))
		{
#ifdef ENABLE_DEBUGOVERLAY
			if (Config.EnableImgui)
			{
				DOverlay.SetTransform(dtstTransformStateType, lpD3DMatrix);
			}
#endif
		}

		return hr;
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	default:
		return DDERR_GENERIC;
	case 2:
		return GetProxyInterfaceV2()->SetTransform(dtstTransformStateType, lpD3DMatrix);
	case 3:
		return GetProxyInterfaceV3()->SetTransform(dtstTransformStateType, lpD3DMatrix);
	case 7:
		return GetProxyInterfaceV7()->SetTransform(dtstTransformStateType, lpD3DMatrix);
	}
}

HRESULT m_IDirect3DDeviceX::GetTransform(D3DTRANSFORMSTATETYPE dtstTransformStateType, LPD3DMATRIX lpD3DMatrix)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpD3DMatrix)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			*lpD3DMatrix = {};
			return DDERR_INVALIDOBJECT;
		}

		switch ((DWORD)dtstTransformStateType)
		{
		case D3DTRANSFORMSTATE_WORLD:
			dtstTransformStateType = D3DTS_WORLD;
			break;
		case D3DTRANSFORMSTATE_WORLD1:
			dtstTransformStateType = D3DTS_WORLD1;
			break;
		case D3DTRANSFORMSTATE_WORLD2:
			dtstTransformStateType = D3DTS_WORLD2;
			break;
		case D3DTRANSFORMSTATE_WORLD3:
			dtstTransformStateType = D3DTS_WORLD3;
			break;
		}

		return GetD9Transform(dtstTransformStateType, lpD3DMatrix);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	default:
		return DDERR_GENERIC;
	case 2:
		return GetProxyInterfaceV2()->GetTransform(dtstTransformStateType, lpD3DMatrix);
	case 3:
		return GetProxyInterfaceV3()->GetTransform(dtstTransformStateType, lpD3DMatrix);
	case 7:
		return GetProxyInterfaceV7()->GetTransform(dtstTransformStateType, lpD3DMatrix);
	}
}

HRESULT m_IDirect3DDeviceX::MultiplyTransform(D3DTRANSFORMSTATETYPE dtstTransformStateType, LPD3DMATRIX lpD3DMatrix)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_INVALIDOBJECT;
		}

		switch ((DWORD)dtstTransformStateType)
		{
		case D3DTRANSFORMSTATE_WORLD:
			dtstTransformStateType = D3DTS_WORLD;
			break;
		case D3DTRANSFORMSTATE_WORLD1:
			dtstTransformStateType = D3DTS_WORLD1;
			break;
		case D3DTRANSFORMSTATE_WORLD2:
			dtstTransformStateType = D3DTS_WORLD2;
			break;
		case D3DTRANSFORMSTATE_WORLD3:
			dtstTransformStateType = D3DTS_WORLD3;
			break;
		}

		return D9MultiplyTransform(dtstTransformStateType, lpD3DMatrix);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	default:
		return DDERR_GENERIC;
	case 2:
		return GetProxyInterfaceV2()->MultiplyTransform(dtstTransformStateType, lpD3DMatrix);
	case 3:
		return GetProxyInterfaceV3()->MultiplyTransform(dtstTransformStateType, lpD3DMatrix);
	case 7:
		return GetProxyInterfaceV7()->MultiplyTransform(dtstTransformStateType, lpD3DMatrix);
	}
}

HRESULT m_IDirect3DDeviceX::DrawPrimitive(D3DPRIMITIVETYPE dptPrimitiveType, DWORD dwVertexTypeDesc, LPVOID lpVertices, DWORD dwVertexCount, DWORD dwFlags, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")" <<
		" VertexType = " << Logging::hex(dptPrimitiveType) <<
		" VertexDesc = " << Logging::hex(dwVertexTypeDesc) <<
		" Vertices = " << lpVertices <<
		" VertexCount = " << dwVertexCount <<
		" Flags = " << Logging::hex(dwFlags) <<
		" Version = " << DirectXVersion;

	if (Config.Dd7to9)
	{
		if (dwVertexCount == 0)
		{
			return D3D_OK; // Nothing to draw
		}

		if (!lpVertices)
		{
			return DDERR_INVALIDPARAMS;
		}

		if (DirectXVersion == 2)
		{
			if (dwVertexTypeDesc != D3DVT_VERTEX && dwVertexTypeDesc != D3DVT_LVERTEX && dwVertexTypeDesc != D3DVT_TLVERTEX)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: invalid Vertex type: " << dwVertexTypeDesc);
				return D3DERR_INVALIDVERTEXTYPE;
			}
			dwVertexTypeDesc = ConvertVertexTypeToFVF((D3DVERTEXTYPE)dwVertexTypeDesc);
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_INVALIDOBJECT;
		}

#ifdef ENABLE_PROFILING
		auto startTime = std::chrono::high_resolution_clock::now();
#endif

		ScopedCriticalSection ThreadLockDD(DdrawWrapper::GetDDCriticalSection());

		dwFlags = (dwFlags & D3DDP_FORCE_DWORD);

		// Update vertices for Direct3D9 (needs to be first)
		UpdateVertices(dwVertexTypeDesc, lpVertices, 0, dwVertexCount);

		// Set fixed function vertex type
		if (FAILED((*d3d9Device)->SetFVF(dwVertexTypeDesc)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: invalid FVF type: " << Logging::hex(dwVertexTypeDesc));
			return DDERR_INVALIDPARAMS;
		}

		// Handle dwFlags
		SetDrawStates(dwVertexTypeDesc, dwFlags, DirectXVersion);

		// Draw primitive UP
		HRESULT hr = (*d3d9Device)->DrawPrimitiveUP(dptPrimitiveType, GetNumberOfPrimitives(dptPrimitiveType, dwVertexCount), lpVertices, GetVertexStride(dwVertexTypeDesc));

		// Handle dwFlags
		RestoreDrawStates(dwVertexTypeDesc, dwFlags, DirectXVersion);

		if (FAILED(hr))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: 'DrawPrimitiveUP' call failed: " << (D3DERR)hr);
		}

#ifdef ENABLE_PROFILING
		Logging::Log() << __FUNCTION__ << " (" << this << ") hr = " << (D3DERR)hr << " Timing = " << Logging::GetTimeLapseInMS(startTime);
#endif

		return hr;
	}

	if (Config.DdrawUseNativeResolution)
	{
		ScaleVertices(dwVertexTypeDesc, lpVertices, dwVertexCount);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	default:
		return DDERR_GENERIC;
	case 2:
		return GetProxyInterfaceV2()->DrawPrimitive(dptPrimitiveType, (D3DVERTEXTYPE)dwVertexTypeDesc, lpVertices, dwVertexCount, dwFlags);
	case 3:
		return GetProxyInterfaceV3()->DrawPrimitive(dptPrimitiveType, dwVertexTypeDesc, lpVertices, dwVertexCount, dwFlags);
	case 7:
		return GetProxyInterfaceV7()->DrawPrimitive(dptPrimitiveType, dwVertexTypeDesc, lpVertices, dwVertexCount, dwFlags);
	}
}

HRESULT m_IDirect3DDeviceX::DrawIndexedPrimitive(D3DPRIMITIVETYPE dptPrimitiveType, DWORD dwVertexTypeDesc, LPVOID lpVertices, DWORD dwVertexCount, LPWORD lpwIndices, DWORD dwIndexCount, DWORD dwFlags, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")" <<
		" VertexType = " << Logging::hex(dptPrimitiveType) <<
		" VertexDesc = " << Logging::hex(dwVertexTypeDesc) <<
		" Vertices = " << lpVertices <<
		" VertexCount = " << dwVertexCount <<
		" Indices = " << lpwIndices <<
		" IndexCount = " << dwIndexCount <<
		" Flags = " << Logging::hex(dwFlags) <<
		" Version = " << DirectXVersion;

	if (Config.Dd7to9)
	{
		if (dwVertexCount == 0 || dwIndexCount == 0)
		{
			return D3D_OK; // Nothing to draw
		}

		if (!lpVertices || !lpwIndices)
		{
			return DDERR_INVALIDPARAMS;
		}

		if (DirectXVersion == 2)
		{
			if (dwVertexTypeDesc != D3DVT_VERTEX && dwVertexTypeDesc != D3DVT_LVERTEX && dwVertexTypeDesc != D3DVT_TLVERTEX)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: invalid Vertex type: " << dwVertexTypeDesc);
				return D3DERR_INVALIDVERTEXTYPE;
			}
			dwVertexTypeDesc = ConvertVertexTypeToFVF((D3DVERTEXTYPE)dwVertexTypeDesc);
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_INVALIDOBJECT;
		}

#ifdef ENABLE_PROFILING
		auto startTime = std::chrono::high_resolution_clock::now();
#endif

		ScopedCriticalSection ThreadLockDD(DdrawWrapper::GetDDCriticalSection());

		dwFlags = (dwFlags & D3DDP_FORCE_DWORD);

		// Update vertices for Direct3D9 (needs to be first)
		UpdateVertices(dwVertexTypeDesc, lpVertices, 0, dwVertexCount);

		// Set fixed function vertex type
		if (FAILED((*d3d9Device)->SetFVF(dwVertexTypeDesc)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: invalid FVF type: " << Logging::hex(dwVertexTypeDesc));
			return DDERR_INVALIDPARAMS;
		}

		// Handle dwFlags
		SetDrawStates(dwVertexTypeDesc, dwFlags, DirectXVersion);

		// Draw indexed primitive UP
		HRESULT hr = (*d3d9Device)->DrawIndexedPrimitiveUP(dptPrimitiveType, 0, dwVertexCount, GetNumberOfPrimitives(dptPrimitiveType, dwIndexCount), lpwIndices, D3DFMT_INDEX16, lpVertices, GetVertexStride(dwVertexTypeDesc));

		// Handle dwFlags
		RestoreDrawStates(dwVertexTypeDesc, dwFlags, DirectXVersion);

		if (FAILED(hr))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: 'DrawIndexedPrimitiveUP' call failed: " << (D3DERR)hr);
		}

#ifdef ENABLE_PROFILING
		Logging::Log() << __FUNCTION__ << " (" << this << ") hr = " << (D3DERR)hr << " Timing = " << Logging::GetTimeLapseInMS(startTime);
#endif

		return hr;
	}

	if (Config.DdrawUseNativeResolution)
	{
		ScaleVertices(dwVertexTypeDesc, lpVertices, dwVertexCount);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	default:
		return DDERR_GENERIC;
	case 2:
		return GetProxyInterfaceV2()->DrawIndexedPrimitive(dptPrimitiveType, (D3DVERTEXTYPE)dwVertexTypeDesc, lpVertices, dwVertexCount, lpwIndices, dwIndexCount, dwFlags);
	case 3:
		return GetProxyInterfaceV3()->DrawIndexedPrimitive(dptPrimitiveType, dwVertexTypeDesc, lpVertices, dwVertexCount, lpwIndices, dwIndexCount, dwFlags);
	case 7:
		return GetProxyInterfaceV7()->DrawIndexedPrimitive(dptPrimitiveType, dwVertexTypeDesc, lpVertices, dwVertexCount, lpwIndices, dwIndexCount, dwFlags);
	}
}

HRESULT m_IDirect3DDeviceX::SetClipStatus(LPD3DCLIPSTATUS lpD3DClipStatus)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpD3DClipStatus || !(lpD3DClipStatus->dwFlags & (D3DCLIPSTATUS_STATUS | D3DCLIPSTATUS_EXTENTS2 | D3DCLIPSTATUS_EXTENTS3)))
		{
			return DDERR_INVALIDPARAMS;
		}

		LOG_LIMIT(100, __FUNCTION__ << " Warning: clip status is not fully supported.");

		D3DClipStatus.dwFlags = lpD3DClipStatus->dwFlags & (D3DCLIPSTATUS_STATUS | D3DCLIPSTATUS_EXTENTS2 | D3DCLIPSTATUS_EXTENTS3);

		// Status
		if (lpD3DClipStatus->dwFlags & D3DCLIPSTATUS_STATUS)
		{
			D3DClipStatus.dwStatus = lpD3DClipStatus->dwStatus;
		}

		// Extents 2-D
		if (lpD3DClipStatus->dwFlags & (D3DCLIPSTATUS_EXTENTS2 | D3DCLIPSTATUS_EXTENTS3))
		{
			D3DClipStatus.minx = lpD3DClipStatus->minx;
			D3DClipStatus.maxx = lpD3DClipStatus->maxx;
			D3DClipStatus.miny = lpD3DClipStatus->miny;
			D3DClipStatus.maxy = lpD3DClipStatus->maxy;
		}

		// Extents 3-D
		if (lpD3DClipStatus->dwFlags & D3DCLIPSTATUS_EXTENTS3)
		{
			D3DClipStatus.minz = lpD3DClipStatus->minz;
			D3DClipStatus.maxz = lpD3DClipStatus->maxz;
		}

		return D3D_OK;
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	default:
		return DDERR_GENERIC;
	case 2:
		return GetProxyInterfaceV2()->SetClipStatus(lpD3DClipStatus);
	case 3:
		return GetProxyInterfaceV3()->SetClipStatus(lpD3DClipStatus);
	case 7:
		return GetProxyInterfaceV7()->SetClipStatus(lpD3DClipStatus);
	}
}

HRESULT m_IDirect3DDeviceX::GetClipStatus(LPD3DCLIPSTATUS lpD3DClipStatus)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpD3DClipStatus)
		{
			return DDERR_INVALIDPARAMS;
		}

		LOG_LIMIT(100, __FUNCTION__ << " Warning: clip status is not fully supported.");

		*lpD3DClipStatus = D3DClipStatus;

		return D3D_OK;
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	default:
		return DDERR_GENERIC;
	case 2:
		return GetProxyInterfaceV2()->GetClipStatus(lpD3DClipStatus);
	case 3:
		return GetProxyInterfaceV3()->GetClipStatus(lpD3DClipStatus);
	case 7:
		return GetProxyInterfaceV7()->GetClipStatus(lpD3DClipStatus);
	}
}

// ******************************
// IDirect3DDevice v3 functions
// ******************************

HRESULT m_IDirect3DDeviceX::DrawPrimitiveStrided(D3DPRIMITIVETYPE dptPrimitiveType, DWORD dwVertexTypeDesc, LPD3DDRAWPRIMITIVESTRIDEDDATA lpVertexArray, DWORD dwVertexCount, DWORD dwFlags, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (dwVertexCount == 0)
		{
			return D3D_OK; // Nothing to draw
		}

		if (!lpVertexArray)
		{
			return DDERR_INVALIDPARAMS;
		}

		if (DirectXVersion == 2)
		{
			if (dwVertexTypeDesc != D3DVT_VERTEX && dwVertexTypeDesc != D3DVT_LVERTEX && dwVertexTypeDesc != D3DVT_TLVERTEX)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: invalid Vertex type: " << dwVertexTypeDesc);
				return D3DERR_INVALIDVERTEXTYPE;
			}
			dwVertexTypeDesc = ConvertVertexTypeToFVF((D3DVERTEXTYPE)dwVertexTypeDesc);
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_INVALIDOBJECT;
		}

#ifdef ENABLE_PROFILING
		auto startTime = std::chrono::high_resolution_clock::now();
#endif

		ScopedCriticalSection ThreadLockDD(DdrawWrapper::GetDDCriticalSection());

		dwFlags = (dwFlags & D3DDP_FORCE_DWORD);

		// Update vertex desc type (FVF) before interleaving
		dwVertexTypeDesc = dwVertexTypeDesc == D3DFVF_LVERTEX ? D3DFVF_LVERTEX9 : dwVertexTypeDesc;

		// Update vertices for Direct3D9 (needs to be first)
		if (!InterleaveStridedVertexData(VertexCache, lpVertexArray, 0, dwVertexCount, dwVertexTypeDesc))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: invalid StridedVertexData!");
			return DDERR_INVALIDPARAMS;
		}

		// Set fixed function vertex type
		if (FAILED((*d3d9Device)->SetFVF(dwVertexTypeDesc)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: invalid FVF type: " << Logging::hex(dwVertexTypeDesc));
			return DDERR_INVALIDPARAMS;
		}

		// Handle dwFlags
		SetDrawStates(dwVertexTypeDesc, dwFlags, DirectXVersion);

		// Draw primitive UP
		HRESULT hr = (*d3d9Device)->DrawPrimitiveUP(dptPrimitiveType, GetNumberOfPrimitives(dptPrimitiveType, dwVertexCount), VertexCache.data(), GetVertexStride(dwVertexTypeDesc));

		// Handle dwFlags
		RestoreDrawStates(dwVertexTypeDesc, dwFlags, DirectXVersion);

		if (FAILED(hr))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: 'DrawPrimitiveUP' call failed: " << (D3DERR)hr);
		}

#ifdef ENABLE_PROFILING
		Logging::Log() << __FUNCTION__ << " (" << this << ") hr = " << (D3DERR)hr << " Timing = " << Logging::GetTimeLapseInMS(startTime);
#endif

		return hr;
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	case 2:
	default:
		return DDERR_GENERIC;
	case 3:
		return GetProxyInterfaceV3()->DrawPrimitiveStrided(dptPrimitiveType, dwVertexTypeDesc, lpVertexArray, dwVertexCount, dwFlags);
	case 7:
		if (DirectXVersion != 7)
		{
			// Handle dwFlags
			SetDrawStates(dwVertexTypeDesc, dwFlags, DirectXVersion);

			DWORD Flags = dwFlags & ~(D3DDP_DONOTCLIP | D3DDP_DONOTLIGHT | D3DDP_DONOTUPDATEEXTENTS);
			HRESULT hr = GetProxyInterfaceV7()->DrawPrimitiveStrided(dptPrimitiveType, dwVertexTypeDesc, lpVertexArray, dwVertexCount, Flags);

			// Handle dwFlags
			RestoreDrawStates(dwVertexTypeDesc, dwFlags, DirectXVersion);

			return hr;
		}
		else
		{
			return GetProxyInterfaceV7()->DrawPrimitiveStrided(dptPrimitiveType, dwVertexTypeDesc, lpVertexArray, dwVertexCount, dwFlags);
		}
	}
}

HRESULT m_IDirect3DDeviceX::DrawIndexedPrimitiveStrided(D3DPRIMITIVETYPE dptPrimitiveType, DWORD dwVertexTypeDesc, LPD3DDRAWPRIMITIVESTRIDEDDATA lpVertexArray, DWORD dwVertexCount, LPWORD lpwIndices, DWORD dwIndexCount, DWORD dwFlags, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (dwVertexCount == 0 || dwIndexCount == 0)
		{
			return D3D_OK; // Nothing to draw
		}

		if (!lpVertexArray || !lpwIndices)
		{
			return DDERR_INVALIDPARAMS;
		}

		if (DirectXVersion == 2)
		{
			if (dwVertexTypeDesc != D3DVT_VERTEX && dwVertexTypeDesc != D3DVT_LVERTEX && dwVertexTypeDesc != D3DVT_TLVERTEX)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: invalid Vertex type: " << dwVertexTypeDesc);
				return D3DERR_INVALIDVERTEXTYPE;
			}
			dwVertexTypeDesc = ConvertVertexTypeToFVF((D3DVERTEXTYPE)dwVertexTypeDesc);
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_INVALIDOBJECT;
		}

#ifdef ENABLE_PROFILING
		auto startTime = std::chrono::high_resolution_clock::now();
#endif

		ScopedCriticalSection ThreadLockDD(DdrawWrapper::GetDDCriticalSection());

		dwFlags = (dwFlags & D3DDP_FORCE_DWORD);

		// Update vertex desc type (FVF) before interleaving
		dwVertexTypeDesc = dwVertexTypeDesc == D3DFVF_LVERTEX ? D3DFVF_LVERTEX9 : dwVertexTypeDesc;

		// Set fixed function vertex type
		if (FAILED((*d3d9Device)->SetFVF(dwVertexTypeDesc)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: invalid FVF type: " << Logging::hex(dwVertexTypeDesc));
			return DDERR_INVALIDPARAMS;
		}

		// Update vertices for Direct3D9 (needs to be first)
		if (!InterleaveStridedVertexData(VertexCache, lpVertexArray, 0, dwVertexCount, dwVertexTypeDesc))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: invalid StridedVertexData!");
			return DDERR_INVALIDPARAMS;
		}

		// Handle dwFlags
		SetDrawStates(dwVertexTypeDesc, dwFlags, DirectXVersion);

		// Draw indexed primitive UP
		HRESULT hr = (*d3d9Device)->DrawIndexedPrimitiveUP(dptPrimitiveType, 0, dwVertexCount, GetNumberOfPrimitives(dptPrimitiveType, dwIndexCount), lpwIndices, D3DFMT_INDEX16, VertexCache.data(), GetVertexStride(dwVertexTypeDesc));

		// Handle dwFlags
		RestoreDrawStates(dwVertexTypeDesc, dwFlags, DirectXVersion);

		if (FAILED(hr))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: 'DrawIndexedPrimitiveUP' call failed: " << (D3DERR)hr);
		}

#ifdef ENABLE_PROFILING
		Logging::Log() << __FUNCTION__ << " (" << this << ") hr = " << (D3DERR)hr << " Timing = " << Logging::GetTimeLapseInMS(startTime);
#endif

		return hr;
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	case 2:
	default:
		return DDERR_GENERIC;
	case 3:
		return GetProxyInterfaceV3()->DrawIndexedPrimitiveStrided(dptPrimitiveType, dwVertexTypeDesc, lpVertexArray, dwVertexCount, lpwIndices, dwIndexCount, dwFlags);
	case 7:
		if (DirectXVersion != 7)
		{
			// Handle dwFlags
			SetDrawStates(dwVertexTypeDesc, dwFlags, DirectXVersion);

			DWORD Flags = dwFlags & ~(D3DDP_DONOTCLIP | D3DDP_DONOTLIGHT | D3DDP_DONOTUPDATEEXTENTS);
			HRESULT hr = GetProxyInterfaceV7()->DrawIndexedPrimitiveStrided(dptPrimitiveType, dwVertexTypeDesc, lpVertexArray, dwVertexCount, lpwIndices, dwIndexCount, Flags);

			// Handle dwFlags
			RestoreDrawStates(dwVertexTypeDesc, dwFlags, DirectXVersion);

			return hr;
		}
		else
		{
			return GetProxyInterfaceV7()->DrawIndexedPrimitiveStrided(dptPrimitiveType, dwVertexTypeDesc, lpVertexArray, dwVertexCount, lpwIndices, dwIndexCount, dwFlags);
		}
	}
}

HRESULT m_IDirect3DDeviceX::DrawPrimitiveVB(D3DPRIMITIVETYPE dptPrimitiveType, LPDIRECT3DVERTEXBUFFER7 lpd3dVertexBuffer, DWORD dwStartVertex, DWORD dwNumVertices, DWORD dwFlags, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")" <<
		" VertexType = " << Logging::hex(dptPrimitiveType) <<
		" VertexBuffer = " << lpd3dVertexBuffer <<
		" StartVertex = " << dwStartVertex <<
		" NumVertices = " << dwNumVertices <<
		" Flags = " << Logging::hex(dwFlags) <<
		" Version = " << DirectXVersion;

	if (Config.Dd7to9)
	{
		if (dwNumVertices == 0)
		{
			return D3D_OK; // Nothing to draw
		}

		if (!lpd3dVertexBuffer)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_INVALIDOBJECT;
		}

#ifdef ENABLE_PROFILING
		auto startTime = std::chrono::high_resolution_clock::now();
#endif

		ScopedCriticalSection ThreadLockDD(DdrawWrapper::GetDDCriticalSection());

		dwFlags = (dwFlags & D3DDP_FORCE_DWORD);

		m_IDirect3DVertexBufferX* pVertexBufferX = nullptr;
		lpd3dVertexBuffer->QueryInterface(IID_GetInterfaceX, (LPVOID*)&pVertexBufferX);
		if (!pVertexBufferX)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get vertex buffer wrapper!");
			return DDERR_GENERIC;
		}

		LPDIRECT3DVERTEXBUFFER9 d3d9VertexBuffer = pVertexBufferX->GetCurrentD9VertexBuffer();
		if (!d3d9VertexBuffer)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get d3d9 vertex buffer!");
			return DDERR_GENERIC;
		}

		DWORD FVF = pVertexBufferX->GetFVF9();

		// Set fixed function vertex type
		if (FAILED((*d3d9Device)->SetFVF(FVF)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: invalid FVF type: " << Logging::hex(FVF));
			return DDERR_INVALIDPARAMS;
		}

		// Set stream source
		(*d3d9Device)->SetStreamSource(0, d3d9VertexBuffer, 0, GetVertexStride(FVF));

		// Handle dwFlags
		SetDrawStates(FVF, dwFlags, DirectXVersion);

		// Draw primitive
		HRESULT hr = (*d3d9Device)->DrawPrimitive(dptPrimitiveType, dwStartVertex, GetNumberOfPrimitives(dptPrimitiveType, dwNumVertices));

		// Handle dwFlags
		RestoreDrawStates(FVF, dwFlags, DirectXVersion);

		if (FAILED(hr))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: 'DrawPrimitive' call failed: " << (D3DERR)hr);
		}

#ifdef ENABLE_PROFILING
		Logging::Log() << __FUNCTION__ << " (" << this << ") hr = " << (D3DERR)hr << " Timing = " << Logging::GetTimeLapseInMS(startTime);
#endif

		return hr;
	}

	if (lpd3dVertexBuffer)
	{
		lpd3dVertexBuffer->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpd3dVertexBuffer);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	case 2:
	default:
		return DDERR_GENERIC;
	case 3:
		return GetProxyInterfaceV3()->DrawPrimitiveVB(dptPrimitiveType, (LPDIRECT3DVERTEXBUFFER)lpd3dVertexBuffer, dwStartVertex, dwNumVertices, dwFlags);
	case 7:
		return GetProxyInterfaceV7()->DrawPrimitiveVB(dptPrimitiveType, lpd3dVertexBuffer, dwStartVertex, dwNumVertices, dwFlags);
	}
}

HRESULT m_IDirect3DDeviceX::DrawIndexedPrimitiveVB(D3DPRIMITIVETYPE dptPrimitiveType, LPDIRECT3DVERTEXBUFFER7 lpd3dVertexBuffer, DWORD dwStartVertex, DWORD dwNumVertices, LPWORD lpwIndices, DWORD dwIndexCount, DWORD dwFlags, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")" <<
		" VertexType = " << Logging::hex(dptPrimitiveType) <<
		" VertexBuffer = " << lpd3dVertexBuffer <<
		" StartVertex = " << dwStartVertex <<
		" NumVertices = " << dwNumVertices <<
		" Indices = " << lpwIndices <<
		" IndexCount = " << dwIndexCount <<
		" Flags = " << Logging::hex(dwFlags) <<
		" Version = " << DirectXVersion;

	if (Config.Dd7to9)
	{
		if (dwNumVertices == 0 || dwIndexCount == 0)
		{
			return D3D_OK; // Nothing to draw
		}

		if (!lpd3dVertexBuffer || !lpwIndices)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_INVALIDOBJECT;
		}

#ifdef ENABLE_PROFILING
		auto startTime = std::chrono::high_resolution_clock::now();
#endif

		ScopedCriticalSection ThreadLockDD(DdrawWrapper::GetDDCriticalSection());

		dwFlags = (dwFlags & D3DDP_FORCE_DWORD);

		m_IDirect3DVertexBufferX* pVertexBufferX = nullptr;
		lpd3dVertexBuffer->QueryInterface(IID_GetInterfaceX, (LPVOID*)&pVertexBufferX);
		if (!pVertexBufferX)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get vertex buffer wrapper!");
			return DDERR_INVALIDPARAMS;
		}

		LPDIRECT3DVERTEXBUFFER9 d3d9VertexBuffer = pVertexBufferX->GetCurrentD9VertexBuffer();
		if (!d3d9VertexBuffer)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get d3d9 vertex buffer!");
			return DDERR_GENERIC;
		}

		DWORD FVF = pVertexBufferX->GetFVF9();

		// Set fixed function vertex type
		if (FAILED((*d3d9Device)->SetFVF(FVF)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: invalid FVF type: " << Logging::hex(FVF));
			return DDERR_INVALIDPARAMS;
		}

		LPDIRECT3DINDEXBUFFER9 d3d9IndexBuffer = ddrawParent->GetIndexBuffer(lpwIndices, dwIndexCount);
		if (!d3d9IndexBuffer)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get d3d9 index buffer!");
			return DDERR_GENERIC;
		}

		// Set stream source
		(*d3d9Device)->SetStreamSource(0, d3d9VertexBuffer, 0, GetVertexStride(FVF));

		// Set Index data
		(*d3d9Device)->SetIndices(d3d9IndexBuffer);

		// Handle dwFlags
		SetDrawStates(FVF, dwFlags, DirectXVersion);

		// Draw primitive
		HRESULT hr = (*d3d9Device)->DrawIndexedPrimitive(dptPrimitiveType, dwStartVertex, 0, dwNumVertices, 0, GetNumberOfPrimitives(dptPrimitiveType, dwIndexCount));

		// Handle dwFlags
		RestoreDrawStates(FVF, dwFlags, DirectXVersion);

		if (FAILED(hr))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: 'DrawIndexedPrimitive' call failed: " << (D3DERR)hr);
		}

#ifdef ENABLE_PROFILING
		Logging::Log() << __FUNCTION__ << " (" << this << ") hr = " << (D3DERR)hr << " Timing = " << Logging::GetTimeLapseInMS(startTime);
#endif

		return hr;
	}

	if (lpd3dVertexBuffer)
	{
		lpd3dVertexBuffer->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpd3dVertexBuffer);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	case 2:
	default:
		return DDERR_GENERIC;
	case 3:
		return GetProxyInterfaceV3()->DrawIndexedPrimitiveVB(dptPrimitiveType, (LPDIRECT3DVERTEXBUFFER)lpd3dVertexBuffer, lpwIndices, dwIndexCount, dwFlags);
	case 7:
		return GetProxyInterfaceV7()->DrawIndexedPrimitiveVB(dptPrimitiveType, lpd3dVertexBuffer, dwStartVertex, dwNumVertices, lpwIndices, dwIndexCount, dwFlags);
	}
}

HRESULT m_IDirect3DDeviceX::ComputeSphereVisibility(LPD3DVECTOR lpCenters, LPD3DVALUE lpRadii, DWORD dwNumSpheres, DWORD dwFlags, LPDWORD lpdwReturnValues)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpCenters || !lpRadii || !dwNumSpheres || !lpdwReturnValues)
		{
			return DDERR_INVALIDPARAMS;
		}

		LOG_LIMIT(100, __FUNCTION__ << " Warning: function not fully implemented");

		// Sphere visibility is computed by back-transforming the viewing frustum to the model space, using the inverse of the combined world, view, or projection matrices.
		// If the combined matrix can't be inverted (if the determinant is 0), the method will fail, returning D3DERR_INVALIDMATRIX.
		for (UINT x = 0; x < dwNumSpheres; x++)
		{
			// If a sphere is completely visible, the corresponding entry in lpdwReturnValues is 0.
			lpdwReturnValues[x] = 0;	// Just return all is visible for now
		}

		return D3D_OK;
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	case 2:
	default:
		return DDERR_GENERIC;
	case 3:
		return GetProxyInterfaceV3()->ComputeSphereVisibility(lpCenters, lpRadii, dwNumSpheres, dwFlags, lpdwReturnValues);
	case 7:
		return GetProxyInterfaceV7()->ComputeSphereVisibility(lpCenters, lpRadii, dwNumSpheres, dwFlags, lpdwReturnValues);
	}
}

HRESULT m_IDirect3DDeviceX::GetTexture(DWORD dwStage, LPDIRECT3DTEXTURE2* lplpTexture)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lplpTexture || dwStage >= D3DHAL_TSS_MAXSTAGES)
		{
			return DDERR_INVALIDPARAMS;
		}
		*lplpTexture = nullptr;

		// Get surface stage
		ComPtr<IDirectDrawSurface7> pSurface;
		HRESULT hr = GetTexture(dwStage, pSurface.GetAddressOf());

		if (FAILED(hr))
		{
			return hr;
		}

		// Get surface wrapper
		m_IDirectDrawSurfaceX* pSurfaceX = nullptr;
		pSurface->QueryInterface(IID_GetInterfaceX, (LPVOID*)&pSurfaceX);
		if (!pSurfaceX)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get surface wrapper!");
			return DDERR_INVALIDPARAMS;
		}

		// Get attached texture from surface
		m_IDirect3DTextureX* pTextureX = pSurfaceX->GetAttachedTexture();
		if (!pTextureX)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get texture!");
			return DDERR_INVALIDPARAMS;
		}

		// Add ref to texture
		pTextureX->AddRef();

		*lplpTexture = (LPDIRECT3DTEXTURE2)pTextureX->GetWrapperInterfaceX(0);

		return D3D_OK;
	}

	HRESULT hr = GetProxyInterfaceV3()->GetTexture(dwStage, lplpTexture);

	if (SUCCEEDED(hr) && lplpTexture)
	{
		*lplpTexture = ProxyAddressLookupTable.FindAddress<m_IDirect3DTexture2>(*lplpTexture, 2);
	}

	return hr;
}

HRESULT m_IDirect3DDeviceX::SetTexture(DWORD dwStage, LPDIRECT3DTEXTURE2 lpTexture)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (dwStage >= D3DHAL_TSS_MAXSTAGES)
		{
			return DDERR_INVALIDPARAMS;
		}

		if (!lpTexture)
		{
			return SetTexture(dwStage, (LPDIRECTDRAWSURFACE7)nullptr);
		}

		m_IDirect3DTextureX* pTextureX = nullptr;
		lpTexture->QueryInterface(IID_GetInterfaceX, (LPVOID*)&pTextureX);
		if (!pTextureX)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get texture wrapper!");
			return DDERR_INVALIDPARAMS;
		}

		m_IDirectDrawSurfaceX* pSurfaceX = pTextureX->GetSurface();
		if (!pSurfaceX)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get surface!");
			return DDERR_INVALIDPARAMS;
		}

		return SetTexture(dwStage, (LPDIRECTDRAWSURFACE7)pSurfaceX->GetWrapperInterfaceX(0));
	}

	if (lpTexture)
	{
		lpTexture->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpTexture);
	}

	return GetProxyInterfaceV3()->SetTexture(dwStage, lpTexture);
}

HRESULT m_IDirect3DDeviceX::GetTextureStageState(DWORD dwStage, D3DTEXTURESTAGESTATETYPE dwState, LPDWORD lpdwValue)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpdwValue)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			*lpdwValue = 0;
			return DDERR_INVALIDOBJECT;
		}

		if (dwStage >= D3DHAL_TSS_MAXSTAGES || dwState == 0
			|| (ClientDirectXVersion == 3 && dwState > 23)
			|| (ClientDirectXVersion == 7 && dwState > 24))
		{
			if (dwStage < D3DHAL_TSS_MAXSTAGES && (DWORD)dwState < MaxTextureStageStates)
			{
				*lpdwValue = DeviceStates.TextureStageState[dwStage][dwState].State;
			}
			else
			{
				*lpdwValue = (DWORD)-1;
			}
			return D3D_OK;
		}

		// Special handling for stage 0 with some types because they are shared by a RenderTarget state
		if (dwStage == 0)
		{
			switch ((DWORD)dwState)
			{
			case D3DTSS_COLOROP:
			case D3DTSS_COLORARG1:
			case D3DTSS_COLORARG2:
			case D3DTSS_ALPHAOP:
			case D3DTSS_ALPHAARG1:
			case D3DTSS_ALPHAARG2:
			case D3DTSS_ADDRESSU:
			case D3DTSS_ADDRESSV:
			case D3DTSS_BORDERCOLOR:
			case D3DTSS_MAGFILTER:
			case D3DTSS_MINFILTER:
			case D3DTSS_MIPFILTER:
			case D3DTSS_MIPMAPLODBIAS:
			case D3DTSS_MAXANISOTROPY:
				*lpdwValue = DeviceStates.tsMap0[dwState];
				return D3D_OK;
			}
		}

		switch ((DWORD)dwState)
		{
		case D3DTSS_ADDRESS:
			*lpdwValue = DeviceStates.TextureStageState[dwStage][dwState].State;
			return D3D_OK;
		case D3DTSS_ADDRESSU:
			return GetD9SamplerState(dwStage, D3DSAMP_ADDRESSU, lpdwValue);
		case D3DTSS_ADDRESSV:
			return GetD9SamplerState(dwStage, D3DSAMP_ADDRESSV, lpdwValue);
		case D3DTSS_ADDRESSW:
			return GetD9SamplerState(dwStage, D3DSAMP_ADDRESSW, lpdwValue);
		case D3DTSS_BORDERCOLOR:
			return GetD9SamplerState(dwStage, D3DSAMP_BORDERCOLOR, lpdwValue);
		case D3DTSS_MAGFILTER:
			*lpdwValue = DeviceStates.TextureStageState[dwStage][dwState].State;
			return D3D_OK;
		case D3DTSS_MINFILTER:
			return GetD9SamplerState(dwStage, D3DSAMP_MINFILTER, lpdwValue);
		case D3DTSS_MIPFILTER:
			*lpdwValue = DeviceStates.TextureStageState[dwStage][dwState].State;
			return D3D_OK;
		case D3DTSS_MIPMAPLODBIAS:
			return GetD9SamplerState(dwStage, D3DSAMP_MIPMAPLODBIAS, lpdwValue);
		case D3DTSS_MAXMIPLEVEL:
			return GetD9SamplerState(dwStage, D3DSAMP_MAXMIPLEVEL, lpdwValue);
		case D3DTSS_MAXANISOTROPY:
			return GetD9SamplerState(dwStage, D3DSAMP_MAXANISOTROPY, lpdwValue);
		}

		if (!CheckTextureStageStateType(dwState))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: Texture Stage state type not implemented: " << dwState);
		}

		return GetD9TextureStageState(dwStage, dwState, lpdwValue);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	case 2:
	default:
		return DDERR_GENERIC;
	case 3:
		return GetProxyInterfaceV3()->GetTextureStageState(dwStage, dwState, lpdwValue);
	case 7:
		return GetProxyInterfaceV7()->GetTextureStageState(dwStage, dwState, lpdwValue);
	}
}

HRESULT m_IDirect3DDeviceX::SetTextureStageState(DWORD dwStage, D3DTEXTURESTAGESTATETYPE dwState, DWORD dwValue)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (dwStage >= D3DHAL_TSS_MAXSTAGES)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_INVALIDOBJECT;
		}

		if (dwStage >= D3DHAL_TSS_MAXSTAGES || dwState == 0
			|| (ClientDirectXVersion == 3 && dwState > 23)
			|| (ClientDirectXVersion == 7 && dwState > 24))
		{
			if (dwStage < D3DHAL_TSS_MAXSTAGES && (DWORD)dwState < MaxTextureStageStates)
			{
				DeviceStates.TextureStageState[dwStage][dwState].State = dwValue;
			}
			return D3D_OK;
		}

		// Special handling for stage 0 with some types because they are shared by a RenderTarget state
		if (dwStage == 0)
		{
			switch ((DWORD)dwState)
			{
			case D3DTSS_COLOROP:
			case D3DTSS_COLORARG1:
			case D3DTSS_COLORARG2:
			case D3DTSS_ALPHAOP:
			case D3DTSS_ALPHAARG1:
			case D3DTSS_ALPHAARG2:
			case D3DTSS_ADDRESSU:
			case D3DTSS_ADDRESSV:
			case D3DTSS_BORDERCOLOR:
			case D3DTSS_MAGFILTER:
			case D3DTSS_MINFILTER:
			case D3DTSS_MIPFILTER:
			case D3DTSS_MIPMAPLODBIAS:
			case D3DTSS_MAXANISOTROPY:
				SetTextureStageStateMap(dwState, dwValue);
			}
		}

		switch ((DWORD)dwState)
		{
		case D3DTSS_ADDRESS:
			DeviceStates.TextureStageState[dwStage][dwState].State = dwValue;
			SetD9SamplerState(dwStage, D3DSAMP_ADDRESSU, dwValue);
			return SetD9SamplerState(dwStage, D3DSAMP_ADDRESSV, dwValue);
		case D3DTSS_ADDRESSU:
			return SetD9SamplerState(dwStage, D3DSAMP_ADDRESSU, dwValue);
		case D3DTSS_ADDRESSV:
			return SetD9SamplerState(dwStage, D3DSAMP_ADDRESSV, dwValue);
		case D3DTSS_ADDRESSW:
			return SetD9SamplerState(dwStage, D3DSAMP_ADDRESSW, dwValue);
		case D3DTSS_BORDERCOLOR:
			return SetD9SamplerState(dwStage, D3DSAMP_BORDERCOLOR, dwValue);
		case D3DTSS_MAGFILTER:
			DeviceStates.TextureStageState[dwStage][dwState].State = dwValue;
			return SetD9SamplerState(dwStage, D3DSAMP_MAGFILTER, dwValue);
		case D3DTSS_MINFILTER:
			return SetD9SamplerState(dwStage, D3DSAMP_MINFILTER, dwValue);
		case D3DTSS_MIPFILTER:
			DeviceStates.TextureStageState[dwStage][dwState].State = dwValue;
			return SetD9SamplerState(dwStage, D3DSAMP_MIPFILTER, dwValue);
		case D3DTSS_MIPMAPLODBIAS:
			return SetD9SamplerState(dwStage, D3DSAMP_MIPMAPLODBIAS, dwValue);
		case D3DTSS_MAXMIPLEVEL:
			return SetD9SamplerState(dwStage, D3DSAMP_MAXMIPLEVEL, dwValue);
		case D3DTSS_MAXANISOTROPY:
			return SetD9SamplerState(dwStage, D3DSAMP_MAXANISOTROPY, dwValue);
		}

		if (!CheckTextureStageStateType(dwState))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: Texture Stage state type not implemented: " << dwState);
			return D3D_OK;	// Just return OK for now!
		}

		return SetD9TextureStageState(dwStage, dwState, dwValue);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	case 2:
	default:
		return DDERR_GENERIC;
	case 3:
		return GetProxyInterfaceV3()->SetTextureStageState(dwStage, dwState, dwValue);
	case 7:
		return GetProxyInterfaceV7()->SetTextureStageState(dwStage, dwState, dwValue);
	}
}

HRESULT m_IDirect3DDeviceX::ValidateDevice(LPDWORD lpdwPasses)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpdwPasses)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_INVALIDOBJECT;
		}

#ifdef ENABLE_PROFILING
		auto startTime = std::chrono::high_resolution_clock::now();
#endif

		ScopedCriticalSection ThreadLockDD(DdrawWrapper::GetDDCriticalSection());

		PrepDevice();

		DWORD FVF, Size;
		IDirect3DVertexBuffer9* vertexBuffer = ddrawParent->GetValidateDeviceVertexBuffer(FVF, Size);

		if (!vertexBuffer)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Failed to get vertex buffer!");
			return DDERR_GENERIC;
		}

		// Bind the vertex buffer to the device
		(*d3d9Device)->SetStreamSource(0, vertexBuffer, 0, Size);

		// Set a simple FVF (Flexible Vertex Format)
		(*d3d9Device)->SetFVF(FVF);

		// Call ValidateDevice
		HRESULT hr = (*d3d9Device)->ValidateDevice(lpdwPasses);

		if (FAILED(hr))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: ValidateDevice() function failed: " << (DDERR)hr);
		}

#ifdef ENABLE_PROFILING
		Logging::Log() << __FUNCTION__ << " (" << this << ") hr = " << (D3DERR)hr << " Timing = " << Logging::GetTimeLapseInMS(startTime);
#endif

		return hr;
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	case 2:
	default:
		return DDERR_GENERIC;
	case 3:
		return GetProxyInterfaceV3()->ValidateDevice(lpdwPasses);
	case 7:
		return GetProxyInterfaceV7()->ValidateDevice(lpdwPasses);
	}
}

// ******************************
// IDirect3DDevice v7 functions
// ******************************

HRESULT m_IDirect3DDeviceX::GetCaps(LPD3DDEVICEDESC7 lpD3DDevDesc)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpD3DDevDesc)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_INVALIDOBJECT;
		}

		ConvertDeviceDesc(*lpD3DDevDesc, Caps9);

		return D3D_OK;
	}

	return GetProxyInterfaceV7()->GetCaps(lpD3DDevDesc);
}

HRESULT m_IDirect3DDeviceX::EnumTextureFormats(LPD3DENUMPIXELFORMATSCALLBACK lpd3dEnumPixelProc, LPVOID lpArg)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpd3dEnumPixelProc)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, false)))
		{
			return DDERR_INVALIDOBJECT;
		}

		LPDIRECT3D9 d3d9Object = ddrawParent->GetDirectD9Object();

		if (!d3d9Object)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to get d3d9 object!");
			return DDERR_GENERIC;
		}

		// Get texture list
		std::vector<D3DFORMAT> TextureList = {
			D3DFMT_R5G6B5,
			D3DFMT_X1R5G5B5,
			D3DFMT_A1R5G5B5,
			D3DFMT_A4R4G4B4,
			//D3DFMT_R8G8B8,	// Requires emulation
			D3DFMT_X8R8G8B8,
			D3DFMT_A8R8G8B8,
			D3DFMT_V8U8,
			D3DFMT_X8L8V8U8,
			D3DFMT_L6V5U5,
			D3DFMT_DXT1,
			D3DFMT_DXT2,
			D3DFMT_DXT3,
			D3DFMT_DXT4,
			D3DFMT_DXT5,
			D3DFMT_P8,
			D3DFMT_L8,
			D3DFMT_A8,
			D3DFMT_A4L4,
			D3DFMT_A8L8 };

		// If textures are being trimmed
		if (Config.DdrawLimitTextureFormats)
		{
			// Trim texture list
			std::vector<D3DFORMAT> TrimTextureList = {
				D3DFMT_V8U8,       // May be trimmed if normal maps are unused
				D3DFMT_X8L8V8U8,   // Rare normal map format
				D3DFMT_L6V5U5,     // Uncommon format
				D3DFMT_DXT5,       // Newer texture format
				D3DFMT_P8,         // 8-bit palettized (Direct3D9 deprecated this)
				D3DFMT_A4L4 };     // Rare grayscale+alpha format

			// Remove trimmed texture from list
			for (auto it = TextureList.begin(); it != TextureList.end(); )
			{
				if (std::find(TrimTextureList.begin(), TrimTextureList.end(), *it) != TrimTextureList.end())
				{
					it = TextureList.erase(it); // Remove and update iterator
				}
				else
				{
					++it; // Move to next element
				}
			}
		}
		// Add FourCCs to texture list
		else
		{
			for (D3DFORMAT format : FourCCTypes)
			{
				TextureList.push_back(format);
			}
		}

		// Check for supported textures
		DDPIXELFORMAT ddpfPixelFormat = {};
		ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);

		bool IsDirectDraw8bit = (ddrawParent->GetDisplayBPP() == 8);

		for (D3DFORMAT format : TextureList)
		{
			if (!IsUnsupportedFormat(format) && ((format == D3DFMT_P8 && IsDirectDraw8bit) ||
				SUCCEEDED(d3d9Object->CheckDeviceFormat(ddrawParent->GetAdapterIndex(), D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, 0, D3DRTYPE_TEXTURE, format))))
			{
				SetPixelDisplayFormat(format, ddpfPixelFormat);
				if (lpd3dEnumPixelProc(&ddpfPixelFormat, lpArg) == DDENUMRET_CANCEL)
				{
					return D3D_OK;
				}
			}
		}

		return D3D_OK;
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
	case 2:
	default:
		return DDERR_GENERIC;
	case 3:
		return GetProxyInterfaceV3()->EnumTextureFormats(lpd3dEnumPixelProc, lpArg);
	case 7:
		return GetProxyInterfaceV7()->EnumTextureFormats(lpd3dEnumPixelProc, lpArg);
	}
}

HRESULT m_IDirect3DDeviceX::Clear(DWORD dwCount, LPD3DRECT lpRects, DWORD dwFlags, D3DCOLOR dwColor, D3DVALUE dvZ, DWORD dwStencil)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_INVALIDOBJECT;
		}

		ScopedCriticalSection ThreadLockDD(DdrawWrapper::GetDDCriticalSection());

		// Clear respects the current viewport
		(*d3d9Device)->SetViewport(&DeviceStates.Viewport.View);

		if (lpCurrentRenderTargetX)
		{
			lpCurrentRenderTargetX->PrepareRenderTarget();

			if (ddrawParent->GetRenderTargetSurface() != lpCurrentRenderTargetX)
			{
				ddrawParent->SetRenderTargetSurface(lpCurrentRenderTargetX);
			}
		}

		return (*d3d9Device)->Clear(dwCount, lpRects, dwFlags, dwColor, dvZ, dwStencil);
	}

	return GetProxyInterfaceV7()->Clear(dwCount, lpRects, dwFlags, dwColor, dvZ, dwStencil);
}

HRESULT m_IDirect3DDeviceX::SetViewport(LPD3DVIEWPORT7 lpViewport)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpViewport)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_INVALIDOBJECT;
		}

		// Clear viewport scaling
		DeviceStates.Viewport.UseViewportScale = false;

		return SetD9Viewport((D3DVIEWPORT9*)lpViewport);
	}

	D3DVIEWPORT7 Viewport7;
	if (Config.DdrawUseNativeResolution && lpViewport)
	{
		ConvertViewport(Viewport7, *lpViewport);
		Viewport7.dwX = (LONG)(Viewport7.dwX * ScaleDDWidthRatio) + ScaleDDPadX;
		Viewport7.dwY = (LONG)(Viewport7.dwY * ScaleDDHeightRatio) + ScaleDDPadY;
		Viewport7.dwWidth = (LONG)(Viewport7.dwWidth * ScaleDDWidthRatio);
		Viewport7.dwHeight = (LONG)(Viewport7.dwHeight * ScaleDDHeightRatio);
		lpViewport = &Viewport7;
	}

	return GetProxyInterfaceV7()->SetViewport(lpViewport);
}

HRESULT m_IDirect3DDeviceX::GetViewport(LPD3DVIEWPORT7 lpViewport)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpViewport)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			*lpViewport = {};
			return DDERR_INVALIDOBJECT;
		}

		return GetD9Viewport((D3DVIEWPORT9*)lpViewport);
	}

	return GetProxyInterfaceV7()->GetViewport(lpViewport);
}

HRESULT m_IDirect3DDeviceX::SetMaterial(LPD3DMATERIAL7 lpMaterial)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpMaterial)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_INVALIDOBJECT;
		}

		return SetD9Material((D3DMATERIAL9*)lpMaterial);
	}

	return GetProxyInterfaceV7()->SetMaterial(lpMaterial);
}

HRESULT m_IDirect3DDeviceX::GetMaterial(LPD3DMATERIAL7 lpMaterial)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpMaterial)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_INVALIDOBJECT;
		}

		return GetD9Material((D3DMATERIAL9*)lpMaterial);
	}

	return GetProxyInterfaceV7()->GetMaterial(lpMaterial);
}

HRESULT m_IDirect3DDeviceX::SetLight(DWORD dwLightIndex, LPD3DLIGHT7 lpLight)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpLight)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: called with nullptr: " << lpLight);
			return DDERR_INVALIDPARAMS;
		}

		if (lpLight->dltType == D3DLIGHT_PARALLELPOINT || lpLight->dltType == D3DLIGHT_GLSPOT)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: Light Type: " << lpLight->dltType << " Not Implemented");
			return D3D_OK;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_INVALIDOBJECT;
		}

		HRESULT hr = SetD9Light(dwLightIndex, reinterpret_cast<D3DLIGHT9*>(lpLight));

		if (SUCCEEDED(hr))
		{
#ifdef ENABLE_DEBUGOVERLAY
			if (Config.EnableImgui)
			{
				DOverlay.SetLight(dwLightIndex, lpLight);
			}
#endif
		}

		return hr;
	}

	return GetProxyInterfaceV7()->SetLight(dwLightIndex, lpLight);
}

HRESULT m_IDirect3DDeviceX::GetLight(DWORD dwLightIndex, LPD3DLIGHT7 lpLight)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpLight)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_INVALIDOBJECT;
		}

		return GetD9Light(dwLightIndex, reinterpret_cast<D3DLIGHT9*>(lpLight));
	}

	return GetProxyInterfaceV7()->GetLight(dwLightIndex, lpLight);
}

HRESULT m_IDirect3DDeviceX::BeginStateBlock()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_INVALIDOBJECT;
		}

		if (IsRecordingState)
		{
			return DDERR_GENERIC;
		}

		ScopedCriticalSection ThreadLockDD(DdrawWrapper::GetDDCriticalSection());

		PrepDevice();

		HRESULT hr = (*d3d9Device)->BeginStateBlock();

		if (SUCCEEDED(hr))
		{
			IsRecordingState = true;
		}

		return hr;
	}

	return GetProxyInterfaceV7()->BeginStateBlock();
}

HRESULT m_IDirect3DDeviceX::EndStateBlock(LPDWORD lpdwBlockHandle)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpdwBlockHandle)
		{
			return DDERR_INVALIDPARAMS;
		}
		*lpdwBlockHandle = NULL;

		if (!IsRecordingState)
		{
			return DDERR_GENERIC;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_INVALIDOBJECT;
		}

		ScopedCriticalSection ThreadLockDD(DdrawWrapper::GetDDCriticalSection());

		HRESULT hr = (*d3d9Device)->EndStateBlock(reinterpret_cast<IDirect3DStateBlock9**>(lpdwBlockHandle));

		if (SUCCEEDED(hr))
		{
			IsRecordingState = false;
			StateBlockTokens.insert(*lpdwBlockHandle);
		}

		return hr;
	}

	return GetProxyInterfaceV7()->EndStateBlock(lpdwBlockHandle);
}

HRESULT m_IDirect3DDeviceX::PreLoad(LPDIRECTDRAWSURFACE7 lpddsTexture)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Textures are loaded as managed in Direct3D9, so there is no need to manualy preload textures
		return D3D_OK;
	}

	if (lpddsTexture)
	{
		lpddsTexture->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpddsTexture);
	}

	return GetProxyInterfaceV7()->PreLoad(lpddsTexture);
}

HRESULT m_IDirect3DDeviceX::GetTexture(DWORD dwStage, LPDIRECTDRAWSURFACE7* lplpTexture)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lplpTexture || dwStage >= D3DHAL_TSS_MAXSTAGES)
		{
			return DDERR_INVALIDPARAMS;
		}
		*lplpTexture = nullptr;

		HRESULT hr = DDERR_GENERIC;

		if (AttachedTexture[dwStage])
		{
			AttachedTexture[dwStage]->AddRef();

			*lplpTexture = AttachedTexture[dwStage];

			hr = D3D_OK;
		}

		return hr;
	}

	HRESULT hr = GetProxyInterfaceV7()->GetTexture(dwStage, lplpTexture);

	if (SUCCEEDED(hr) && lplpTexture)
	{
		*lplpTexture = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(*lplpTexture, 7);
	}

	return hr;
}

HRESULT m_IDirect3DDeviceX::SetTexture(DWORD dwStage, LPDIRECTDRAWSURFACE7 lpSurface)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (dwStage >= D3DHAL_TSS_MAXSTAGES)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_INVALIDOBJECT;
		}

		m_IDirectDrawSurfaceX* lpDDSrcSurfaceX = nullptr;

		if (lpSurface)
		{
			lpSurface->QueryInterface(IID_GetInterfaceX, (LPVOID*)&lpDDSrcSurfaceX);
			if (!lpDDSrcSurfaceX)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: could not get surface wrapper!");
				return DDERR_INVALIDPARAMS;
			}

			IDirect3DTexture9* pTexture9 = lpDDSrcSurfaceX->GetD3d9Texture();
			if (!pTexture9)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: could not get texture!");
				return DDERR_INVALIDPARAMS;
			}

			if (lpCurrentRenderTargetX && lpCurrentRenderTargetX->IsPalette() && !lpDDSrcSurfaceX->IsPalette())
			{
				LOG_LIMIT(100, __FUNCTION__ << " Warning: setting non-palette texture on a paletted render target!");
			}
		}

		AttachedTexture[dwStage] = lpSurface;
		CurrentTextureSurfaceX[dwStage] = lpDDSrcSurfaceX;

		return D3D_OK;
	}

	if (lpSurface)
	{
		lpSurface->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpSurface);
	}

	return GetProxyInterfaceV7()->SetTexture(dwStage, lpSurface);
}

HRESULT m_IDirect3DDeviceX::ApplyStateBlock(DWORD dwBlockHandle)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (dwBlockHandle == 0)
		{
			return DDERR_INVALIDPARAMS;
		}

		if (IsRecordingState)
		{
			return DDERR_GENERIC;
		}

		if (StateBlockTokens.find(dwBlockHandle) == StateBlockTokens.end())
		{
			return D3D_OK;
		}

		ScopedCriticalSection ThreadLockDD(DdrawWrapper::GetDDCriticalSection());

		PrepDevice();

		HRESULT hr = reinterpret_cast<IDirect3DStateBlock9*>(dwBlockHandle)->Apply();

		if (SUCCEEDED(hr))
		{
			CollectStates();
		}

		return hr;
	}

	return GetProxyInterfaceV7()->ApplyStateBlock(dwBlockHandle);
}

HRESULT m_IDirect3DDeviceX::CaptureStateBlock(DWORD dwBlockHandle)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (dwBlockHandle == 0)
		{
			return DDERR_INVALIDPARAMS;
		}

		if (IsRecordingState)
		{
			return DDERR_GENERIC;
		}

		if (StateBlockTokens.find(dwBlockHandle) == StateBlockTokens.end())
		{
			return D3D_OK;
		}

		ScopedCriticalSection ThreadLockDD(DdrawWrapper::GetDDCriticalSection());

		PrepDevice();

		return reinterpret_cast<IDirect3DStateBlock9*>(dwBlockHandle)->Capture();
	}

	return GetProxyInterfaceV7()->CaptureStateBlock(dwBlockHandle);
}

HRESULT m_IDirect3DDeviceX::DeleteStateBlock(DWORD dwBlockHandle)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (dwBlockHandle == 0)
		{
			return DDERR_INVALIDPARAMS;
		}

		if (IsRecordingState)
		{
			return DDERR_GENERIC;
		}

		if (StateBlockTokens.find(dwBlockHandle) == StateBlockTokens.end())
		{
			return D3D_OK;
		}

		reinterpret_cast<IDirect3DStateBlock9*>(dwBlockHandle)->Release();

		StateBlockTokens.erase(dwBlockHandle);

		return D3D_OK;
	}

	return GetProxyInterfaceV7()->DeleteStateBlock(dwBlockHandle);
}

HRESULT m_IDirect3DDeviceX::CreateStateBlock(D3DSTATEBLOCKTYPE d3dsbtype, LPDWORD lpdwBlockHandle)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpdwBlockHandle)
		{
			return DDERR_INVALIDPARAMS;
		}
		*lpdwBlockHandle = NULL;

		if (IsRecordingState)
		{
			return DDERR_GENERIC;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_INVALIDOBJECT;
		}

		ScopedCriticalSection ThreadLockDD(DdrawWrapper::GetDDCriticalSection());

		PrepDevice();

		HRESULT hr = (*d3d9Device)->CreateStateBlock(d3dsbtype, reinterpret_cast<IDirect3DStateBlock9**>(lpdwBlockHandle));

		if (SUCCEEDED(hr))
		{
			StateBlockTokens.insert(*lpdwBlockHandle);
		}

		return hr;
	}

	return GetProxyInterfaceV7()->CreateStateBlock(d3dsbtype, lpdwBlockHandle);
}

HRESULT m_IDirect3DDeviceX::Load(LPDIRECTDRAWSURFACE7 lpDestTex, LPPOINT lpDestPoint, LPDIRECTDRAWSURFACE7 lpSrcTex, LPRECT lprcSrcRect, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpDestTex || !lpSrcTex)
		{
			return  DDERR_INVALIDPARAMS;
		}

		m_IDirectDrawSurfaceX* pDestSurfaceX = nullptr;
		lpDestTex->QueryInterface(IID_GetInterfaceX, (LPVOID*)&pDestSurfaceX);
		if (!pDestSurfaceX)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get surface!");
			return DDERR_GENERIC;
		}

		return pDestSurfaceX->Load(lpDestTex, lpDestPoint, lpSrcTex, lprcSrcRect, dwFlags);
	}

	if (lpDestTex)
	{
		lpDestTex->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDestTex);
	}
	if (lpSrcTex)
	{
		lpSrcTex->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpSrcTex);
	}

	return GetProxyInterfaceV7()->Load(lpDestTex, lpDestPoint, lpSrcTex, lprcSrcRect, dwFlags);
}

HRESULT m_IDirect3DDeviceX::LightEnable(DWORD dwLightIndex, BOOL bEnable)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_INVALIDOBJECT;
		}

		HRESULT hr = D9LightEnable(dwLightIndex, bEnable);

		if (SUCCEEDED(hr))
		{
#ifdef ENABLE_DEBUGOVERLAY
			if (Config.EnableImgui)
			{
				DOverlay.LightEnable(dwLightIndex, bEnable);
			}
#endif
		}

		return hr;
	}

	return GetProxyInterfaceV7()->LightEnable(dwLightIndex, bEnable);
}

HRESULT m_IDirect3DDeviceX::GetLightEnable(DWORD dwLightIndex, BOOL* pbEnable)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!pbEnable)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_INVALIDOBJECT;
		}

		return GetD9LightEnable(dwLightIndex, pbEnable);
	}

	return GetProxyInterfaceV7()->GetLightEnable(dwLightIndex, pbEnable);
}

HRESULT m_IDirect3DDeviceX::SetClipPlane(DWORD dwIndex, D3DVALUE* pPlaneEquation)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_INVALIDOBJECT;
		}

		return SetD9ClipPlane(dwIndex, pPlaneEquation);
	}

	return GetProxyInterfaceV7()->SetClipPlane(dwIndex, pPlaneEquation);
}

HRESULT m_IDirect3DDeviceX::GetClipPlane(DWORD dwIndex, D3DVALUE* pPlaneEquation)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!pPlaneEquation)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true)))
		{
			return DDERR_INVALIDOBJECT;
		}

		return GetD9ClipPlane(dwIndex, pPlaneEquation);
	}

	return GetProxyInterfaceV7()->GetClipPlane(dwIndex, pPlaneEquation);
}

HRESULT m_IDirect3DDeviceX::GetInfo(DWORD dwDevInfoID, LPVOID pDevInfoStruct, DWORD dwSize)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!pDevInfoStruct || dwSize == 0)
		{
			return DDERR_GENERIC;
		}

#ifdef _DEBUG
		// Fill device info structures
		switch (dwDevInfoID)
		{
		case D3DDEVINFOID_TEXTUREMANAGER:
		case D3DDEVINFOID_D3DTEXTUREMANAGER:
			if (dwSize == sizeof(D3DDEVINFO_TEXTUREMANAGER))
			{
				// Simulate a default texture manager structure for a good video card
				D3DDEVINFO_TEXTUREMANAGER* pTexManagerInfo = (D3DDEVINFO_TEXTUREMANAGER*)pDevInfoStruct;
				pTexManagerInfo->bThrashing = FALSE;
				pTexManagerInfo->dwNumEvicts = 0;
				pTexManagerInfo->dwNumVidCreates = 0;
				pTexManagerInfo->dwNumTexturesUsed = 0;
				pTexManagerInfo->dwNumUsedTexInVid = 0;
				pTexManagerInfo->dwWorkingSet = 0;
				pTexManagerInfo->dwWorkingSetBytes = 0;
				pTexManagerInfo->dwTotalManaged = 0;
				pTexManagerInfo->dwTotalBytes = 0;
				pTexManagerInfo->dwLastPri = 0;
				break;
			}
			return DDERR_GENERIC;

		case D3DDEVINFOID_TEXTURING:
			if (dwSize == sizeof(D3DDEVINFO_TEXTURING))
			{
				// Simulate a default texturing activity structure for a good video card
				D3DDEVINFO_TEXTURING* pTexturingInfo = (D3DDEVINFO_TEXTURING*)pDevInfoStruct;
				pTexturingInfo->dwNumLoads = 0;
				pTexturingInfo->dwApproxBytesLoaded = 0;
				pTexturingInfo->dwNumPreLoads = 0;
				pTexturingInfo->dwNumSet = 0;
				pTexturingInfo->dwNumCreates = 0;
				pTexturingInfo->dwNumDestroys = 0;
				pTexturingInfo->dwNumSetPriorities = 0;
				pTexturingInfo->dwNumSetLODs = 0;
				pTexturingInfo->dwNumLocks = 0;
				pTexturingInfo->dwNumGetDCs = 0;
				break;
			}
			return DDERR_GENERIC;

		default:
			Logging::LogDebug() << __FUNCTION__ << " Error: Unknown DevInfoID: " << dwDevInfoID;
			return DDERR_GENERIC;
		}
#endif

		// This method is intended to be used for performance tracking and debugging during product development (on the debug version of DirectX). 
		// The method can succeed, returning S_FALSE, without retrieving device data.
		// This occurs when the retail version of the DirectX runtime is installed on the host system.
		return S_FALSE;
	}

	return GetProxyInterfaceV7()->GetInfo(dwDevInfoID, pDevInfoStruct, dwSize);
}

// ******************************
// Helper functions
// ******************************

void m_IDirect3DDeviceX::InitInterface(DWORD DirectXVersion)
{
	if (D3DInterface)
	{
		D3DInterface->AddD3DDevice(this);
	}

	if (Config.Dd7to9)
	{
		if (ddrawParent)
		{
			d3d9Device = ddrawParent->GetDirectD9Device();

			if (CurrentRenderTarget)
			{
				m_IDirectDrawSurfaceX* lpDDSrcSurfaceX = nullptr;

				CurrentRenderTarget->QueryInterface(IID_GetInterfaceX, (LPVOID*)&lpDDSrcSurfaceX);
				if (lpDDSrcSurfaceX)
				{
					CurrentRenderTarget->AddRef();

					lpCurrentRenderTargetX = lpDDSrcSurfaceX;

					ddrawParent->SetRenderTargetSurface(lpCurrentRenderTargetX);
				}
			}
		}

		AddRef(DirectXVersion);
	}
}

void m_IDirect3DDeviceX::ReleaseInterface()
{
	if (Config.Exiting)
	{
		return;
	}

	if (CurrentRenderTarget)
	{
		lpCurrentRenderTargetX = nullptr;

		CurrentRenderTarget->Release();
		CurrentRenderTarget = nullptr;
	}

	if (D3DInterface)
	{
		D3DInterface->ClearD3DDevice(this);
	}

	// Don't delete wrapper interface
	SaveInterfaceAddress(WrapperInterface);
	SaveInterfaceAddress(WrapperInterface2);
	SaveInterfaceAddress(WrapperInterface3);
	SaveInterfaceAddress(WrapperInterface7);

	// Clear ExecuteBuffers
	for (auto& entry : ExecuteBufferList)
	{
		entry->ClearD3DDevice();
	}

	// Clear device from veiwports
	for (auto& entry : AttachedViewports)
	{
		m_IDirect3DViewportX* lpViewportX = nullptr;
		if (SUCCEEDED(entry->QueryInterface(IID_GetInterfaceX, (LPVOID*)&lpViewportX)))
		{
			lpViewportX->ClearD3DDevice(this);
		}
	}

	ReleaseAllStateBlocks();
}

HRESULT m_IDirect3DDeviceX::CheckInterface(char *FunctionName, bool CheckD3DDevice)
{
	// Check ddrawParent device
	if (!ddrawParent)
	{
		LOG_LIMIT(100, FunctionName << " Error: no ddraw parent!");
		return DDERR_INVALIDOBJECT;
	}

	// Check d3d9 device
	if (CheckD3DDevice)
	{
		if (!ddrawParent->CheckD9Device(FunctionName) || !d3d9Device || !*d3d9Device)
		{
			LOG_LIMIT(100, FunctionName << " Error: d3d9 device not setup!");
			return DDERR_INVALIDOBJECT;
		}
		if (bSetDefaults)
		{
			SetDefaults();
		}
	}

	return D3D_OK;
}

void* m_IDirect3DDeviceX::GetWrapperInterfaceX(DWORD DirectXVersion)
{
	switch (DirectXVersion)
	{
	case 0:
		if (WrapperInterface7) return WrapperInterface7;
		if (WrapperInterface3) return WrapperInterface3;
		if (WrapperInterface2) return WrapperInterface2;
		if (WrapperInterface) return WrapperInterface;
		break;
	case 1:
		return GetInterfaceAddress(WrapperInterface, (LPDIRECT3DDEVICE)ProxyInterface, this);
	case 2:
		return GetInterfaceAddress(WrapperInterface2, (LPDIRECT3DDEVICE2)ProxyInterface, this);
	case 3:
		return GetInterfaceAddress(WrapperInterface3, (LPDIRECT3DDEVICE3)ProxyInterface, this);
	case 7:
		return GetInterfaceAddress(WrapperInterface7, (LPDIRECT3DDEVICE7)ProxyInterface, this);
	}
	LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
	return nullptr;
}

LPDIRECT3DDEVICE9* m_IDirect3DDeviceX::GetD3d9Device()
{
	CheckInterface(__FUNCTION__, true);

	return d3d9Device;
}

HRESULT m_IDirect3DDeviceX::SetViewport(LPD3DVIEWPORT lpViewport)
{
	if (!lpViewport)
	{
		return DDERR_INVALIDPARAMS;
	}

	D3DVIEWPORT7 Viewport7 = {};
	ConvertViewport(Viewport7, *lpViewport);

	HRESULT hr = SetViewport(&Viewport7);

	if (SUCCEEDED(hr))
	{
		DeviceStates.Viewport.UseViewportScale = true;
		DeviceStates.Viewport.ViewportScale = *lpViewport;

		// Set transform
		D3DMATRIX Matrix = {};
		if (SUCCEEDED(GetD9Transform(D3DTS_PROJECTION, &Matrix)))
		{
			SetD9Transform(D3DTS_PROJECTION, &Matrix);
		}
	}

	return hr;
}

HRESULT m_IDirect3DDeviceX::SetViewport(LPD3DVIEWPORT2 lpViewport)
{
	if (!lpViewport)
	{
		return DDERR_INVALIDPARAMS;
	}

	if (lpViewport->dvClipWidth != 0 || lpViewport->dvClipHeight != 0 || lpViewport->dvClipX != 0 || lpViewport->dvClipY != 0)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Warning: 'clip volume' Not Implemented: " <<
			lpViewport->dvClipWidth << "x" << lpViewport->dvClipHeight << " X: " << lpViewport->dvClipX << " Y: " << lpViewport->dvClipY);
	}

	D3DVIEWPORT7 Viewport7 = {};
	ConvertViewport(Viewport7, *lpViewport);

	return SetViewport(&Viewport7);
}

bool m_IDirect3DDeviceX::DeleteAttachedViewport(LPDIRECT3DVIEWPORT3 ViewportX)
{
	auto it = std::find_if(AttachedViewports.begin(), AttachedViewports.end(),
		[=](auto pViewport) -> bool { return pViewport == ViewportX; });

	if (it != std::end(AttachedViewports))
	{
		AttachedViewports.erase(it);
		return true;
	}
	return false;
}

void m_IDirect3DDeviceX::ClearTextureHandle(D3DTEXTUREHANDLE tHandle)
{
	if (tHandle)
	{
		TextureHandleMap.erase(tHandle);

		// If texture handle is set then clear it
		if (rsTextureHandle == tHandle)
		{
			SetTexture(0, (LPDIRECT3DTEXTURE2)nullptr);
		}
	}
}

HRESULT m_IDirect3DDeviceX::SetTextureHandle(D3DTEXTUREHANDLE& tHandle, m_IDirect3DTextureX* pTextureX)
{
	if (!tHandle || !pTextureX)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: NULL pointer found! " << pTextureX << " -> " << tHandle);
		return DDERR_INVALIDPARAMS;
	}

	// Ensure that the handle is unique
	while (GetTexture(tHandle))
	{
		tHandle += 4;
	}

	TextureHandleMap[tHandle] = pTextureX;

	return D3D_OK;
}

void m_IDirect3DDeviceX::ClearLight(m_IDirect3DLight* lpLight)
{
	// Find handle associated with Light
	auto it = LightIndexMap.begin();
	while (it != LightIndexMap.end())
	{
		if (it->second == lpLight)
		{
			DWORD Index = it->first;

			// Remove entry from map
			it = LightIndexMap.erase(it);

			// Disable light
			D9LightEnable(Index, FALSE);

			// Clear device states
			{
				auto entry = DeviceStates.Light.find(Index);
				if (entry != DeviceStates.Light.end())
				{
					DeviceStates.Light.erase(entry);
				}
			}

			// Clear batch state light
			{
				auto entry = BatchStates.Light.find(Index);
				if (entry != BatchStates.Light.end())
				{
					BatchStates.Light.erase(entry);
				}
			}

			// Clear batch state light enable
			{
				auto entry = BatchStates.LightEnable.find(Index);
				if (entry != BatchStates.LightEnable.end())
				{
					BatchStates.LightEnable.erase(entry);
				}
			}
		}
		else
		{
			++it;
		}
	}
}

HRESULT m_IDirect3DDeviceX::SetLight(m_IDirect3DLight* lpLightInterface, LPD3DLIGHT lpLight)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!lpLightInterface || !lpLight || (lpLight->dwSize != sizeof(D3DLIGHT) && lpLight->dwSize != sizeof(D3DLIGHT2)))
	{
		return DDERR_INVALIDPARAMS;
	}

	D3DLIGHT7 Light7;

	// ToDo: the dvAttenuation members are interpreted differently in D3DLIGHT2 than they were for D3DLIGHT.

	ConvertLight(Light7, *lpLight);

	DWORD dwLightIndex = MaxActiveLights;

	// Check if Light exists in the map
	for (auto& entry : LightIndexMap)
	{
		if (entry.second == lpLightInterface)
		{
			dwLightIndex = entry.first;
			break;
		}
	}

	// Create index and add light to the map
	if (dwLightIndex == MaxActiveLights)
	{
		for (BYTE x = 0; x < MaxActiveLights; x++)
		{
			bool Flag = true;
			for (auto& entry : LightIndexMap)
			{
				if (entry.first == x)
				{
					Flag = false;
					break;
				}
			}
			for (auto& entry : DeviceStates.Light)
			{
				if (entry.first == x)
				{
					Flag = false;
					break;
				}
			}
			if (x != 0 && Flag)
			{
				dwLightIndex = x;
				break;
			}
		}
	}

	if (dwLightIndex >= MaxActiveLights)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Failed to find an available Light Index");
		return DDERR_INVALIDPARAMS;
	}

	// Add light to index map
	LightIndexMap[dwLightIndex] = lpLightInterface;

	HRESULT hr = SetLight(dwLightIndex, &Light7);

	if (SUCCEEDED(hr))
	{
		if (lpLight->dwSize == sizeof(D3DLIGHT2))
		{
			LPD3DLIGHT2 lpLight2 = reinterpret_cast<LPD3DLIGHT2>(lpLight);

			if (lpLight2->dwFlags & D3DLIGHT_ACTIVE)
			{
				LightEnable(dwLightIndex, TRUE);
			}
			else
			{
				LightEnable(dwLightIndex, FALSE);
			}
		}
	}

	return hr;
}

HRESULT m_IDirect3DDeviceX::GetLightEnable(m_IDirect3DLight* lpLightInterface, BOOL* pbEnable)
{
	if (!lpLightInterface || !pbEnable)
	{
		return DDERR_INVALIDPARAMS;
	}

	DWORD dwLightIndex = 0;

	// Check if Light exists in the map
	for (auto& entry : LightIndexMap)
	{
		if (entry.second == lpLightInterface)
		{
			dwLightIndex = entry.first;
			break;
		}
	}

	if (dwLightIndex == 0)
	{
		return DDERR_INVALIDPARAMS;
	}

	return GetLightEnable(dwLightIndex, pbEnable);
}

void m_IDirect3DDeviceX::ClearMaterialHandle(D3DMATERIALHANDLE mHandle)
{
	if (mHandle)
	{
		TextureHandleMap.erase(mHandle);

		// If material handle is set then clear it
		if (lsMaterialHandle == mHandle)
		{
			SetMaterialHandle(NULL);
		}
	}
}

HRESULT m_IDirect3DDeviceX::SetMaterialHandle(D3DMATERIALHANDLE& mHandle, m_IDirect3DMaterialX* lpMaterial)
{
	if (!mHandle || !lpMaterial)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: NULL pointer found! " << lpMaterial << " -> " << mHandle);
		return DDERR_GENERIC;
	}

	// Ensure that the handle is unique
	while (GetMaterial(mHandle))
	{
		mHandle += 4;
	}

	MaterialHandleMap[mHandle] = lpMaterial;

	return D3D_OK;
}

HRESULT m_IDirect3DDeviceX::SetMaterial(LPD3DMATERIAL lpMaterial)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!lpMaterial)
	{
		return DDERR_INVALIDPARAMS;
	}

	D3DMATERIAL7 Material7;

	ConvertMaterial(Material7, *lpMaterial);

	HRESULT hr = SetMaterial(&Material7);

	if (FAILED(hr))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Failed to set material: " << (D3DERR)hr);
		return hr;
	}

	if (lpMaterial->dwRampSize)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Warning: RampSize Not Implemented: " << lpMaterial->dwRampSize);
	}

	if (lpMaterial->hTexture)
	{
		SetTextureHandle(lpMaterial->hTexture);
	}

	return D3D_OK;
}

void m_IDirect3DDeviceX::ClearSurface(m_IDirectDrawSurfaceX* lpSurfaceX)
{
	if (lpCurrentRenderTargetX == lpSurfaceX)
	{
		CurrentRenderTarget = nullptr;
		lpCurrentRenderTargetX = nullptr;
		LOG_LIMIT(100, __FUNCTION__ << " Warning: clearing current render target!");
	}
	for (UINT x = 0; x < D3DHAL_TSS_MAXSTAGES; x++)
	{
		if (CurrentTextureSurfaceX[x] == lpSurfaceX)
		{
			SetTexture(x, (LPDIRECTDRAWSURFACE7)nullptr);
			AttachedTexture[x] = nullptr;
			CurrentTextureSurfaceX[x] = nullptr;
		}
	}
}

void m_IDirect3DDeviceX::AddExecuteBuffer(m_IDirect3DExecuteBuffer* lpExecuteBuffer)
{
	if (!lpExecuteBuffer)
	{
		return;
	}

	ExecuteBufferList.push_back(lpExecuteBuffer);
}

void m_IDirect3DDeviceX::ClearExecuteBuffer(m_IDirect3DExecuteBuffer* lpExecuteBuffer)
{
	// Find and remove the buffer from the list
	auto it = std::find(ExecuteBufferList.begin(), ExecuteBufferList.end(), lpExecuteBuffer);
	if (it != ExecuteBufferList.end())
	{
		ExecuteBufferList.erase(it);
	}
}

void m_IDirect3DDeviceX::CopyConvertExecuteVertex(BYTE*& DestVertex, DWORD& DestVertexCount, BYTE* SrcVertex, DWORD SrcIndex, DWORD VertexTypeDesc)
{
	// Primitive structures and related defines. Vertex offsets are to types D3DVERTEX, D3DLVERTEX, or D3DTLVERTEX.
	if (VertexTypeDesc == D3DFVF_VERTEX)
	{
		DestVertexCount++;
		*((D3DVERTEX*)DestVertex) = ((D3DVERTEX*)SrcVertex)[SrcIndex];
		DestVertex += sizeof(D3DVERTEX);
		return;
	}
	else if (VertexTypeDesc == D3DFVF_LVERTEX)
	{
		DestVertexCount++;
		*((D3DLVERTEX*)DestVertex) = ((D3DLVERTEX*)SrcVertex)[SrcIndex];
		DestVertex += sizeof(D3DLVERTEX);
		return;
	}
	else if (VertexTypeDesc == D3DFVF_TLVERTEX)
	{
		DestVertexCount++;
		*((D3DTLVERTEX*)DestVertex) = ((D3DTLVERTEX*)SrcVertex)[SrcIndex];
		DestVertex += sizeof(D3DTLVERTEX);
		return;
	}
}

HRESULT m_IDirect3DDeviceX::DrawExecutePoint(D3DPOINT* point, WORD pointCount, DWORD vertexIndexCount, BYTE* vertexBuffer, DWORD VertexTypeDesc)
{
	// Define vertices and setup vector
	std::vector<BYTE, aligned_allocator<BYTE, 4>> vertices;
	vertices.resize(sizeof(D3DTLVERTEX) * pointCount);
	BYTE* verticesData = vertices.data();
	DWORD verticesCount = 0;

	// Add vertices to vector
	for (DWORD i = 0; i < pointCount; i++)
	{
		if ((DWORD)point[i].wFirst < vertexIndexCount)
		{
			DWORD count = min(point[i].wCount, vertexIndexCount - point[i].wFirst);

			for (DWORD x = 0; x < count; x++)
			{
				CopyConvertExecuteVertex(verticesData, verticesCount, vertexBuffer, point[i].wFirst + x, VertexTypeDesc);
			}
		}
	}

	if (verticesCount)
	{
		// Pass the vertex data to the rendering pipeline
		DrawPrimitive(D3DPT_POINTLIST, VertexTypeDesc, vertices.data(), verticesCount, 0, 1);
	}

	return D3D_OK;
}

HRESULT m_IDirect3DDeviceX::DrawExecuteLine(D3DLINE* line, WORD lineCount, DWORD vertexIndexCount, BYTE* vertexBuffer, DWORD VertexTypeDesc)
{
	// Define vertices and setup vector
	std::vector<BYTE, aligned_allocator<BYTE, 4>> vertices;
	vertices.resize(sizeof(D3DTLVERTEX) * lineCount * 2);
	BYTE* verticesData = vertices.data();
	DWORD verticesCount = 0;

	for (DWORD i = 0; i < lineCount; i++)
	{
		if (line[i].v1 < vertexIndexCount && line[i].v2 < vertexIndexCount)
		{
			CopyConvertExecuteVertex(verticesData, verticesCount, vertexBuffer, line[i].v1, VertexTypeDesc);
			CopyConvertExecuteVertex(verticesData, verticesCount, vertexBuffer, line[i].v2, VertexTypeDesc);
		}
	}

	if (verticesCount)
	{
		// Pass the vertex data to the rendering pipeline
		DrawPrimitive(D3DPT_LINELIST, VertexTypeDesc, vertices.data(), verticesCount, 0, 1);
	}

	return D3D_OK;
}

HRESULT m_IDirect3DDeviceX::DrawExecuteTriangle(D3DTRIANGLE* triangle, WORD triangleCount, DWORD vertexIndexCount, BYTE* vertexBuffer, DWORD VertexTypeDesc)
{
	// Compute buffer size
	DWORD BufferSize;
	{
		bool LastRecord = false;
		DWORD Count = 0, MaxCount = 0;
		for (DWORD i = 0; i < triangleCount; i++)
		{
			bool IsStartRecord = (triangle[i].wFlags & 0x1F) < D3DTRIFLAG_STARTFLAT(30);
			if (IsStartRecord != LastRecord)
			{
				MaxCount = max(Count, MaxCount);
				Count = (IsStartRecord) ? 3 : 4;
			}
			else
			{
				Count += (IsStartRecord) ? 3 : 1;
			}
			LastRecord = IsStartRecord;
		}
		BufferSize = sizeof(D3DTLVERTEX) * max(Count, MaxCount);
	}

	std::vector<BYTE, aligned_allocator<BYTE, 4>> vertices;
	vertices.resize(BufferSize);
	BYTE* verticesData = vertices.data();
	DWORD verticesCount = 0;

	D3DPRIMITIVETYPE PrimitiveType = D3DPT_TRIANGLELIST;

	LONG LastCullMode = D3DTRIFLAG_START;
	LONG CullRecordCount = 0;

	for (DWORD i = 0; i < triangleCount; i++)
	{
		// Flags for this triangle
		WORD TriFlags = (triangle[i].wFlags & 0x1F);

		// START loads all three vertices
		if (TriFlags < D3DTRIFLAG_STARTFLAT(30))
		{
			if (triangle[i].v1 < vertexIndexCount && triangle[i].v2 < vertexIndexCount && triangle[i].v3 < vertexIndexCount)
			{
				PrimitiveType = D3DPT_TRIANGLELIST;

				CopyConvertExecuteVertex(verticesData, verticesCount, vertexBuffer, triangle[i].v1, VertexTypeDesc);
				CopyConvertExecuteVertex(verticesData, verticesCount, vertexBuffer, triangle[i].v2, VertexTypeDesc);
				CopyConvertExecuteVertex(verticesData, verticesCount, vertexBuffer, triangle[i].v3, VertexTypeDesc);

				LastCullMode = D3DTRIFLAG_START;
				CullRecordCount = TriFlags;
			}
		}
		// EVEN and ODD load just v3 with even or odd culling
		else if (TriFlags == D3DTRIFLAG_EVEN || TriFlags == D3DTRIFLAG_ODD)
		{
			// Set primative type
			if (LastCullMode == D3DTRIFLAG_START)
			{
				// Even cull modes indicates a triangle fan
				if (TriFlags == D3DTRIFLAG_EVEN)
				{
					PrimitiveType = D3DPT_TRIANGLEFAN;
				}
				// Odd or mismatching cull modes indicates a triangle strip
				else
				{
					PrimitiveType = D3DPT_TRIANGLESTRIP;
				}
			}
			// The primative type doesn't mismatch past cull mode
			else if ((TriFlags == LastCullMode && PrimitiveType == D3DPT_TRIANGLESTRIP) ||
				(TriFlags != LastCullMode && PrimitiveType == D3DPT_TRIANGLEFAN))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Warning: vertex cull mode mismatch detected!");
			}

			if (triangle[i].v3 < vertexIndexCount)
			{
				CopyConvertExecuteVertex(verticesData, verticesCount, vertexBuffer, triangle[i].v3, VertexTypeDesc);
			}

			LastCullMode = TriFlags;
			CullRecordCount--;
		}

		// Check next records
		bool AtEndOfList = !(i + 1U < triangleCount);
		LONG NextRecord = (i + 1U < triangleCount) ? ((triangle[i + 1].wFlags & 0x1F) < 30 ? D3DTRIFLAG_START : D3DTRIFLAG_EVEN) : 0;
		LONG NextNextRecord = (i + 2U < triangleCount) ? ((triangle[i + 2].wFlags & 0x1F) < 30 ? D3DTRIFLAG_START : D3DTRIFLAG_EVEN) : 0;

		// Draw primitaves once at the end of the list
		if (verticesCount &&								// There primatives to draw
			(AtEndOfList ||									// There are no more records, or
				(NextRecord == D3DTRIFLAG_START &&			// Next record is a new START
					(LastCullMode != D3DTRIFLAG_START || NextNextRecord != D3DTRIFLAG_START))))
		{
			if (CullRecordCount > 0)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Warning: drawing before all records have been culled: " << CullRecordCount);
			}

			// Pass the vertex data to the rendering pipeline
			DrawPrimitive(PrimitiveType, VertexTypeDesc, vertices.data(), verticesCount, 0, 1);

			// Reset variables for next list
			verticesCount = 0;
			verticesData = vertices.data();
		}
	}

	return D3D_OK;
}

void m_IDirect3DDeviceX::ClearViewport(m_IDirect3DViewportX* lpViewportX)
{
	if (lpViewportX == lpCurrentViewportX)
	{
		lpCurrentViewport = nullptr;
		lpCurrentViewportX = nullptr;
	}
}

void m_IDirect3DDeviceX::SetD3D(m_IDirect3DX* lpD3D)
{
	if (!lpD3D)
	{
		return;
	}

	if (D3DInterface && D3DInterface != lpD3D)
	{
		Logging::Log() << __FUNCTION__ << " Warning: Direct3D interface has already been created!";
	}

	D3DInterface = lpD3D;
}

void m_IDirect3DDeviceX::ClearD3D(m_IDirect3DX* lpD3D)
{
	if (lpD3D != D3DInterface)
	{
		Logging::Log() << __FUNCTION__ << " Warning: released Direct3D interface does not match cached one!";
	}

	D3DInterface = nullptr;
}

HRESULT m_IDirect3DDeviceX::SetTextureHandle(DWORD TexHandle)
{
	if (TexHandle == NULL)
	{
		rsTextureHandle = NULL;
		return SetTexture(0, (LPDIRECT3DTEXTURE2)nullptr);
	}
	m_IDirect3DTextureX* pTextureX = GetTexture(TexHandle);
	if (pTextureX)
	{
		IDirect3DTexture2* lpTexture = (IDirect3DTexture2*)pTextureX->GetWrapperInterfaceX(0);
		if (!lpTexture)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get texture address!");
			return D3D_OK;
		}

		rsTextureHandle = TexHandle;
		return SetTexture(0, lpTexture);
	}
	else
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: could not get texture handle!");
		rsTextureHandle = NULL;
		return SetTexture(0, (LPDIRECT3DTEXTURE2)nullptr);
	}
	return D3D_OK;
}

HRESULT m_IDirect3DDeviceX::SetMaterialHandle(DWORD MatHandle)
{
	lsMaterialHandle = NULL;

	D3DMATERIAL Material = {};
	Material.dwSize = sizeof(D3DMATERIAL);

	if (MatHandle)
	{
		m_IDirect3DMaterialX* pMaterialX = GetMaterial(MatHandle);
		if (pMaterialX)
		{
			if (FAILED(pMaterialX->GetMaterial(&Material)))
			{
				return DDERR_INVALIDPARAMS;
			}
		}
		else
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get material handle!");
			return D3D_OK;
		}
	}

	lsMaterialHandle = MatHandle;

	D3DMATERIAL7 Material7;

	ConvertMaterial(Material7, Material);

	SetMaterial(&Material7);

	if (Material.hTexture)
	{
		SetTextureHandle(Material.hTexture);
	}

	return D3D_OK;
}

HRESULT m_IDirect3DDeviceX::GetD9RenderState(D3DRENDERSTATETYPE State, LPDWORD lpValue) const
{
	if (!lpValue)
	{
		return DDERR_INVALIDPARAMS;
	}
	if ((UINT)State >= D3D_MAXRENDERSTATES)
	{
		*lpValue = (DWORD)-1;
		return DDERR_INVALIDPARAMS;
	}

	if (DeviceStates.RenderState[State].Set)
	{
		*lpValue = DeviceStates.RenderState[State].State;
	}
	else
	{
		*lpValue = DefaultRenderState[State];
	}

	return D3D_OK;
}

void m_IDirect3DDeviceX::SetRenderStateMap(D3DRENDERSTATETYPE State, DWORD Value)
{
	DeviceStates.rsMap[State] = Value;
}

HRESULT m_IDirect3DDeviceX::SetD9RenderState(D3DRENDERSTATETYPE State, DWORD Value)
{
	if ((UINT)State >= D3D_MAXRENDERSTATES)
	{
		return DDERR_INVALIDPARAMS;
	}

	BatchStates.RenderState[State] = Value;

	DeviceStates.RenderState[State].Set = (DefaultRenderState[State] != Value);
	DeviceStates.RenderState[State].State = Value;

	return D3D_OK;
}

HRESULT m_IDirect3DDeviceX::GetD9TextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, LPDWORD lpValue) const
{
	if (!lpValue)
	{
		return DDERR_INVALIDPARAMS;
	}
	if (Stage >= D3DHAL_TSS_MAXSTAGES || (UINT)Type >= MaxTextureStageStates)
	{
		*lpValue = (DWORD)-1;
		return DDERR_INVALIDPARAMS;
	}

	if (DeviceStates.TextureStageState[Stage][Type].Set)
	{
		*lpValue = DeviceStates.TextureStageState[Stage][Type].State;
	}
	else
	{
		*lpValue = DefaultTextureStageState[Stage][Type];
	}

	return D3D_OK;
}

void m_IDirect3DDeviceX::SetTextureStageStateMap(D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
	DeviceStates.tsMap0[Type] = Value;
}

HRESULT m_IDirect3DDeviceX::SetD9TextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
	if (Stage >= D3DHAL_TSS_MAXSTAGES || (UINT)Type >= MaxTextureStageStates)
	{
		return DDERR_INVALIDPARAMS;
	}

	BatchStates.TextureStageState[Stage][Type] = Value;

	DeviceStates.TextureStageState[Stage][Type].Set = (DefaultTextureStageState[Stage][Type] != Value);
	DeviceStates.TextureStageState[Stage][Type].State = Value;

	return D3D_OK;
}

HRESULT m_IDirect3DDeviceX::GetD9SamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, LPDWORD lpValue) const
{
	if (!lpValue)
	{
		return DDERR_INVALIDPARAMS;
	}
	if (Sampler >= D3DHAL_TSS_MAXSTAGES || (UINT)Type >= D3DHAL_TEXTURESTATEBUF_SIZE)
	{
		*lpValue = (DWORD)-1;
		return DDERR_INVALIDPARAMS;
	}

	if (DeviceStates.SamplerState[Sampler][Type].Set)
	{
		*lpValue = DeviceStates.SamplerState[Sampler][Type].State;
	}
	else
	{
		*lpValue = DefaultSamplerState[Sampler][Type];
	}

	return D3D_OK;
}

HRESULT m_IDirect3DDeviceX::SetD9SamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value)
{
	if (Sampler >= D3DHAL_TSS_MAXSTAGES || (UINT)Type >= D3DHAL_TEXTURESTATEBUF_SIZE)
	{
		return DDERR_INVALIDPARAMS;
	}

	BatchStates.SamplerState[Sampler][Type] = FixSamplerState(Type, Value);

	DeviceStates.SamplerState[Sampler][Type].Set = (DefaultSamplerState[Sampler][Type] != Value);
	DeviceStates.SamplerState[Sampler][Type].State = Value;

	return D3D_OK;
}

void m_IDirect3DDeviceX::SetLightStateMap(D3DLIGHTSTATETYPE State, DWORD Value)
{
	DeviceStates.lsMap[State] = Value;
}

HRESULT m_IDirect3DDeviceX::GetD9Light(DWORD Index, D3DLIGHT9* lpLight) const
{
	if (!lpLight)
	{
		return DDERR_INVALIDPARAMS;
	}

	auto it = DeviceStates.Light.find(Index);
	if (it != DeviceStates.Light.end() && it->second.Set)
	{
		*lpLight = it->second.Light;
		return D3D_OK;
	}

	return DDERR_INVALIDPARAMS;
}

HRESULT m_IDirect3DDeviceX::SetD9Light(DWORD Index, const D3DLIGHT9* lpLight)
{
	if (!lpLight)
	{
		return DDERR_INVALIDPARAMS;
	}

	BatchStates.Light[Index] = FixLight(*(D3DLIGHT9*)lpLight);

	DeviceStates.Light[Index].Set = true;
	DeviceStates.Light[Index].Light = *lpLight;

	return D3D_OK;
}

HRESULT m_IDirect3DDeviceX::GetD9LightEnable(DWORD Index, LPBOOL lpEnable) const
{
	if (!lpEnable)
	{
		return DDERR_INVALIDPARAMS;
	}

	auto it = DeviceStates.Light.find(Index);
	if (it != DeviceStates.Light.end())
	{
		*lpEnable = it->second.Enable;
	}
	else
	{
		*lpEnable = FALSE;
	}

	return D3D_OK;
}

HRESULT m_IDirect3DDeviceX::D9LightEnable(DWORD Index, BOOL Enable)
{
	BatchStates.LightEnable[Index] = Enable;

	DeviceStates.Light[Index].Enable = Enable;

	return D3D_OK;
}

HRESULT m_IDirect3DDeviceX::GetD9ClipPlane(DWORD Index, float* lpPlane) const
{
	if (!lpPlane)
	{
		return DDERR_INVALIDPARAMS;
	}
	if (Index >= MaxClipPlaneIndex)
	{
		*(FLOAT4*)lpPlane = {};
		return DDERR_INVALIDPARAMS;
	}

	if (DeviceStates.ClipPlane[Index].Set)
	{
		*(FLOAT4*)lpPlane = DeviceStates.ClipPlane[Index].Plane;
	}
	else
	{
		*(FLOAT4*)lpPlane = *(FLOAT4*)&DefaultClipPlane;
	}

	return D3D_OK;
}

HRESULT m_IDirect3DDeviceX::SetD9ClipPlane(DWORD Index, const float* lpPlane)
{
	if (!lpPlane || Index >= MaxClipPlaneIndex)
	{
		return DDERR_INVALIDPARAMS;
	}

	BatchStates.ClipPlane[Index] = *(FLOAT4*)lpPlane;

	DeviceStates.ClipPlane[Index].Set = true;
	DeviceStates.ClipPlane[Index].Plane = *(FLOAT4*)lpPlane;

	return D3D_OK;
}

HRESULT m_IDirect3DDeviceX::GetD9Viewport(D3DVIEWPORT9* lpViewport) const
{
	if (!lpViewport)
	{
		return DDERR_INVALIDPARAMS;
	}

	if (DeviceStates.Viewport.Set)
	{
		*lpViewport = DeviceStates.Viewport.View;
	}
	else
	{
		*lpViewport = DefaultViewport;
	}

	return D3D_OK;
}

HRESULT m_IDirect3DDeviceX::SetD9Viewport(const D3DVIEWPORT9* lpViewport)
{
	if (!lpViewport)
	{
		return DDERR_INVALIDPARAMS;
	}

	if (lpCurrentRenderTargetX)
	{
		DDSURFACEDESC2 Desc2 = {};
		Desc2.dwSize = sizeof(Desc2);
		if (SUCCEEDED(lpCurrentRenderTargetX->GetSurfaceDesc2(&Desc2, 0, 7)))
		{
			if (lpViewport->X + lpViewport->Width > Desc2.dwWidth || lpViewport->Y + lpViewport->Height > Desc2.dwHeight)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: invalid viewport size: " << lpViewport->X << "x" << lpViewport->Y << " " <<
					lpViewport->Width << "x" << lpViewport->Height << " " << Desc2.dwWidth << "x" << Desc2.dwHeight);
				return DDERR_INVALIDPARAMS;
			}
		}
	}

	DeviceStates.Viewport.Set = true;
	DeviceStates.Viewport.View = *lpViewport;

	return D3D_OK;
}

HRESULT m_IDirect3DDeviceX::GetD9Material(D3DMATERIAL9* lpMaterial) const
{
	if (!lpMaterial)
	{
		return DDERR_INVALIDPARAMS;
	}

	if (DeviceStates.Material.Set)
	{
		*lpMaterial = DeviceStates.Material.Material;
	}
	else
	{
		*lpMaterial = DefaultMaterial;
	}

	return D3D_OK;
}

HRESULT m_IDirect3DDeviceX::SetD9Material(const D3DMATERIAL9* lpMaterial)
{
	if (!lpMaterial)
	{
		return DDERR_INVALIDPARAMS;
	}

	BatchStates.Material.Set = true;

	DeviceStates.Material.Set = true;
	DeviceStates.Material.Material = *lpMaterial;

	return D3D_OK;
}

HRESULT m_IDirect3DDeviceX::GetD9Transform(D3DTRANSFORMSTATETYPE State, D3DMATRIX* lpMatrix) const
{
	if (!lpMatrix)
	{
		return DDERR_INVALIDPARAMS;
	}
	if (!IsValidTransformState(State))
	{
		*lpMatrix = {};
		return DDERR_INVALIDPARAMS;
	}

	auto it = DeviceStates.Matrix.find(State);
	if (it != DeviceStates.Matrix.end())
	{
		*lpMatrix = it->second;
	}
	else
	{
		*lpMatrix = DefaultIdentityMatrix;
	}

	return D3D_OK;
}

HRESULT m_IDirect3DDeviceX::SetD9Transform(D3DTRANSFORMSTATETYPE State, const D3DMATRIX* lpMatrix)
{
	if (!lpMatrix || !IsValidTransformState(State))
	{
		return DDERR_INVALIDPARAMS;
	}

	BatchStates.Matrix[State] = FixMatrix(*lpMatrix, State, DeviceStates.Viewport.ViewportScale, DeviceStates.Viewport.UseViewportScale);

	DeviceStates.Matrix[State] = *lpMatrix;

	return D3D_OK;
}

HRESULT m_IDirect3DDeviceX::D9MultiplyTransform(D3DTRANSFORMSTATETYPE State, const D3DMATRIX* lpMatrix)
{
	if (!lpMatrix)
	{
		return DDERR_INVALIDPARAMS;
	}

	D3DMATRIX Matrix;
	HRESULT hr = GetD9Transform(State, &Matrix);

	if (SUCCEEDED(hr))
	{
		D3DMATRIX result = {};
		D3DXMatrixMultiply(&result, lpMatrix, &Matrix);

		BatchStates.Matrix[State] = FixMatrix(*lpMatrix, State, DeviceStates.Viewport.ViewportScale, DeviceStates.Viewport.UseViewportScale);

		DeviceStates.Matrix[State] = result;
	}

	return hr;
}

void m_IDirect3DDeviceX::PrepDevice()
{
	if (ddrawParent && (RequiresStateRestore || ddrawParent->GetLastDrawDevice() != (DWORD)this))
	{
		RestoreStates();
	}

	// Set batched states
	if (Config.Dd7to9)
	{
		for (auto& entry : BatchStates.RenderState)
		{
			(*d3d9Device)->SetRenderState(entry.first, entry.second);
		}
		BatchStates.RenderState.clear();
		for (UINT x = 0; x < D3DHAL_TSS_MAXSTAGES; x++)
		{
			for (auto& entry : BatchStates.TextureStageState[x])
			{
				(*d3d9Device)->SetTextureStageState(x, entry.first, entry.second);
			}
			BatchStates.TextureStageState[x].clear();
			for (auto& entry : BatchStates.SamplerState[x])
			{
				(*d3d9Device)->SetSamplerState(x, entry.first, entry.second);
			}
			BatchStates.SamplerState[x].clear();
		}
		for (auto& entry : BatchStates.Light)
		{
			(*d3d9Device)->SetLight(entry.first, &entry.second);
		}
		BatchStates.Light.clear();
		for (auto& entry : BatchStates.LightEnable)
		{
			(*d3d9Device)->LightEnable(entry.first, entry.second);
		}
		BatchStates.LightEnable.clear();
		for (auto& entry : BatchStates.ClipPlane)
		{
			(*d3d9Device)->SetClipPlane(entry.first, reinterpret_cast<float*>(&entry.second));
		}
		BatchStates.ClipPlane.clear();
		if (BatchStates.Material.Set)
		{
			(*d3d9Device)->SetMaterial(&DeviceStates.Material.Material);
			BatchStates.Material.Set = false;
		}
		for (auto& entry : BatchStates.Matrix)
		{
			(*d3d9Device)->SetTransform(entry.first, &entry.second);
		}
	}
}

HRESULT m_IDirect3DDeviceX::RestoreStates()
{
	if (!d3d9Device || !*d3d9Device)
	{
		RequiresStateRestore = true;
		Logging::Log() << __FUNCTION__ " Error: Failed to restore the device state!";
		return DDERR_GENERIC;
	}

	// Reset vars
	RequiresStateRestore = false;
	ddrawParent->SetLastDrawDevice((DWORD)this);

	// Reset device state
	ddrawParent->ApplyStateBlock();

	// Restore render states
	for (UINT x = 0; x < D3D_MAXRENDERSTATES; x++)
	{
		if (DeviceStates.RenderState[x].Set)
		{
			(*d3d9Device)->SetRenderState((D3DRENDERSTATETYPE)x, DeviceStates.RenderState[x].State);
		}
	}

	// Restore texture states
	for (UINT y = 0; y < D3DHAL_TSS_MAXSTAGES; y++)
	{
		for (UINT x = 0; x < MaxTextureStageStates; x++)
		{
			if (DeviceStates.TextureStageState[y][x].Set)
			{
				(*d3d9Device)->SetTextureStageState(y, (D3DTEXTURESTAGESTATETYPE)x, DeviceStates.TextureStageState[y][x].State);
			}
		}
	}

	// Restore sampler states
	for (UINT y = 0; y < D3DHAL_TSS_MAXSTAGES; y++)
	{
		for (UINT x = 0; x < D3DHAL_TEXTURESTATEBUF_SIZE; x++)
		{
			if (DeviceStates.SamplerState[y][x].Set)
			{
				(*d3d9Device)->SetSamplerState(y, (D3DSAMPLERSTATETYPE)x, FixSamplerState((D3DSAMPLERSTATETYPE)x, DeviceStates.SamplerState[y][x].State));
			}
		}
	}

	// Restore lights
	for (auto& entry : DeviceStates.Light)
	{
		if (entry.second.Set)
		{
			D3DLIGHT9 Light = FixLight(entry.second.Light);
			(*d3d9Device)->SetLight(entry.first, &Light);
		}
		if (entry.second.Enable)
		{
			(*d3d9Device)->LightEnable(entry.first, entry.second.Enable);
		}
	}

	// Restore clip planes
	for (UINT x = 0; x < MaxClipPlaneIndex; x++)
	{
		if (DeviceStates.ClipPlane[x].Set)
		{
			(*d3d9Device)->SetClipPlane(x, reinterpret_cast<float*>(&DeviceStates.ClipPlane[x].Plane));
		}
	}

	// Restore viewport
	if (DeviceStates.Viewport.Set)
	{
		(*d3d9Device)->SetViewport(&DeviceStates.Viewport.View);
	}

	// Restore material
	if (DeviceStates.Material.Set)
	{
		(*d3d9Device)->SetMaterial(&DeviceStates.Material.Material);
	}

	// Restore transform
	for (auto& entry : DeviceStates.Matrix)
	{
		D3DMATRIX Matrix = FixMatrix(entry.second, entry.first, DeviceStates.Viewport.ViewportScale, DeviceStates.Viewport.UseViewportScale);
		(*d3d9Device)->SetTransform(entry.first, &Matrix);
	}

	return D3D_OK;
}

void m_IDirect3DDeviceX::CollectStates()
{
	// Restore render states
	for (UINT x = 0; x < D3D_MAXRENDERSTATES; x++)
	{
		if (DeviceStates.RenderState[x].Set)
		{
			(*d3d9Device)->GetRenderState((D3DRENDERSTATETYPE)x, &DeviceStates.RenderState[x].State);
		}
	}

	// Restore texture states
	for (UINT y = 0; y < D3DHAL_TSS_MAXSTAGES; y++)
	{
		for (UINT x = 0; x < MaxTextureStageStates; x++)
		{
			if (DeviceStates.TextureStageState[y][x].Set)
			{
				(*d3d9Device)->GetTextureStageState(y, (D3DTEXTURESTAGESTATETYPE)x, &DeviceStates.TextureStageState[y][x].State);
			}
		}
	}

	// Restore sampler states
	for (UINT y = 0; y < D3DHAL_TSS_MAXSTAGES; y++)
	{
		for (UINT x = 0; x < D3DHAL_TEXTURESTATEBUF_SIZE; x++)
		{
			if (DeviceStates.SamplerState[y][x].Set)
			{
				(*d3d9Device)->GetSamplerState(y, (D3DSAMPLERSTATETYPE)x, &DeviceStates.SamplerState[y][x].State);
			}
		}
	}

	/*/ Restore lights (ToDo: figure out how to undo fixed up lights)
	for (auto& entry : DeviceStates.Light)
	{
		if (entry.second.Set)
		{
			(*d3d9Device)->GetLight(entry.first, &entry.second.Light);
		}
		(*d3d9Device)->GetLightEnable(entry.first, &entry.second.Enable);
	}*/

	// Restore clip planes
	for (UINT x = 0; x < MaxClipPlaneIndex; x++)
	{
		if (DeviceStates.ClipPlane[x].Set)
		{
			(*d3d9Device)->GetClipPlane(x, DeviceStates.ClipPlane[x].Plane.Plane);
		}
	}

	// Restore viewport
	if (DeviceStates.Viewport.Set)
	{
		(*d3d9Device)->GetViewport(&DeviceStates.Viewport.View);
	}

	// Restore material
	if (DeviceStates.Material.Set)
	{
		(*d3d9Device)->GetMaterial(&DeviceStates.Material.Material);
	}

	/*/ Restore transform (ToDo: figure out how to undo fixed up matrix)
	for (auto& entry : DeviceStates.Matrix)
	{
		(*d3d9Device)->GetTransform(entry.first, &entry.second);
	}*/
}

void m_IDirect3DDeviceX::BeforeResetDevice()
{
	RequiresStateRestore = true;

	if (IsRecordingState)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Warning: StateBlock is in recording state!");
		DWORD dwBlockHandle = NULL;
		if (SUCCEEDED(EndStateBlock(&dwBlockHandle)))
		{
			DeleteStateBlock(dwBlockHandle);
		}
	}
}

void m_IDirect3DDeviceX::AfterResetDevice()
{
	// Get defaults
	(*d3d9Device)->GetViewport(&DefaultViewport);

	// If viewport isn't set then set to default
	if (!DeviceStates.Viewport.Set)
	{
		DeviceStates.Viewport.View = DefaultViewport;
	}
}

void m_IDirect3DDeviceX::ClearDdraw()
{
	ReleaseAllStateBlocks();
	ddrawParent = nullptr;
	colorkeyPixelShader = nullptr;
	fixupVertexShader = nullptr;
	d3d9Device = nullptr;
	RequiresStateRestore = true;
}

void m_IDirect3DDeviceX::ReleaseAllStateBlocks()
{
	while (!StateBlockTokens.empty())
	{
		DWORD Token = *StateBlockTokens.begin();
		if (FAILED(DeleteStateBlock(Token)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to delete all StateBlocks");
			break;
		}
	}
}

void m_IDirect3DDeviceX::SetDefaults()
{
	// Reset defaults flag
	bSetDefaults = false;
	RequiresStateRestore = true;

	// Render states
	if (ClientDirectXVersion == 1)
	{
		DeviceStates.RenderState[D3DRENDERSTATE_COLORKEYENABLE].State = TRUE;
	}
	else
	{
		for (auto State : { 1, 4, 5, 6, 10, 11, 30, 31, 32, 33, 39, 41, 43, 46, 47 })
		{
			DeviceStates.RenderState[State].State = FALSE;
		}
		for (UINT State = 64; State < 96; State++)
		{
			DeviceStates.RenderState[State].State = FALSE;
		}
		for (auto State : { 3, 4, 17, 18, 44, 45, 49 })
		{
			DeviceStates.RenderState[State].State = TRUE;
		}
		DeviceStates.RenderState[D3DRENDERSTATE_ROP2].State = R2_COPYPEN;
		DeviceStates.RenderState[D3DRENDERSTATE_TEXTUREMAPBLEND].State = D3DTBLEND_MODULATE;
		DeviceStates.RenderState[D3DRENDERSTATE_TEXTUREPERSPECTIVE].State = (ClientDirectXVersion == 2 ? FALSE : TRUE);

		if (ClientDirectXVersion != 7)
		{
			DeviceStates.RenderState[D3DRENDERSTATE_NONE].State = 0;
			DeviceStates.RenderState[D3DRENDERSTATE_ANTIALIAS].State = 0;
			DeviceStates.RenderState[D3DRENDERSTATE_EDGEANTIALIAS].State = 0;
		}
		else
		{
			DeviceStates.RenderState[D3DRENDERSTATE_ANTIALIAS].State = (DWORD)-1;
			DeviceStates.RenderState[D3DRENDERSTATE_EXTENTS].State = 0;
			DeviceStates.RenderState[D3DRENDERSTATE_COLORKEYBLENDENABLE].State = 0;
		}

		if (ClientDirectXVersion > 2)
		{
			for (UINT x = 0; x < D3DHAL_TSS_MAXSTAGES; x++)
			{
				for (auto State : { 12, 14, 16, 18 })
				{
					DeviceStates.TextureStageState[x][State].State = TRUE;
				}
			}
		}

		// Set default maps
		DeviceStates.rsMap[D3DRENDERSTATE_FOGEND] = 0x3F800000;	// 1.0f
		DeviceStates.rsMap[D3DRENDERSTATE_FOGDENSITY] = 0x3F800000;	// 1.0f
		DeviceStates.rsMap[D3DRENDERSTATE_AMBIENT] = (ClientDirectXVersion == 7 ? FALSE : (DWORD)-1);
		DeviceStates.rsMap[D3DRENDERSTATE_FOGVERTEXMODE] = (ClientDirectXVersion == 7 ? FALSE : (DWORD)-1);
		DeviceStates.rsMap[D3DRENDERSTATE_COLORVERTEX] = (ClientDirectXVersion == 7 ? TRUE : (DWORD)-1);
		DeviceStates.rsMap[D3DRS_DEPTHBIAS] = (DWORD)-1;

		DeviceStates.tsMap0[D3DTSS_COLOROP] = 4;
		DeviceStates.tsMap0[D3DTSS_COLORARG1] = 2;
		DeviceStates.tsMap0[D3DTSS_COLORARG2] = 1;
		DeviceStates.tsMap0[D3DTSS_ALPHAOP] = 2;
		DeviceStates.tsMap0[D3DTSS_ALPHAARG1] = 2;
		DeviceStates.tsMap0[D3DTSS_ALPHAARG2] = 1;
		DeviceStates.tsMap0[D3DTSS_ADDRESSU] = 1;
		DeviceStates.tsMap0[D3DTSS_ADDRESSV] = 1;
		DeviceStates.tsMap0[D3DTSS_MAGFILTER] = 1;
		DeviceStates.tsMap0[D3DTSS_MINFILTER] = 1;
		DeviceStates.tsMap0[D3DTSS_MIPFILTER] = 1;
		DeviceStates.tsMap0[D3DTSS_MAXANISOTROPY] = 1;

		if (ClientDirectXVersion == 3)
		{
			DeviceStates.lsMap[D3DLIGHTSTATE_COLORVERTEX] = TRUE;
		}
	}

	// Set DirectDraw defaults
	for (UINT x = 1; x < D3DHAL_TSS_MAXSTAGES; x++)
	{
		SetD9TextureStageState(x, D3DTSS_TEXCOORDINDEX, 0);
	}

	if (ClientDirectXVersion < 3)
	{
		SetD9RenderState(D3DRS_SPECULARENABLE, TRUE);
	}

	{
		float Value = 1.0f;
		SetD9RenderState(D3DRS_FOGSTART, *(DWORD*)&Value);
		Value = 100.0f;
		SetD9RenderState(D3DRS_FOGEND, *(DWORD*)&Value);
		Value = 1.0f;
		SetD9RenderState(D3DRS_FOGDENSITY, *(DWORD*)&Value);
		SetD9RenderState(D3DRS_COLORVERTEX, TRUE);
	}

	// Get default structures
	(*d3d9Device)->GetDeviceCaps(&Caps9);
	ddrawParent->GetDefaultViewport(&DefaultViewport);

	// Set defaults
	DeviceStates.Viewport.View = DefaultViewport;
}

void m_IDirect3DDeviceX::SetDrawStates(DWORD dwVertexTypeDesc, DWORD& dwFlags, DWORD DirectXVersion)
{
	PrepDevice();

	// Handle dwFlags
	if (DirectXVersion < 7)
	{
		// dwFlags (D3DDP_WAIT) can be ignored safely

		if (dwFlags & D3DDP_DONOTCLIP)
		{
			(*d3d9Device)->GetRenderState(D3DRS_CLIPPING, &TempStates.rsClipping);
			(*d3d9Device)->SetRenderState(D3DRS_CLIPPING, FALSE);
		}
		if ((dwFlags & D3DDP_DONOTLIGHT) || !(dwVertexTypeDesc & D3DFVF_NORMAL))
		{
			(*d3d9Device)->GetRenderState(D3DRS_LIGHTING, &TempStates.rsLighting);
			(*d3d9Device)->SetRenderState(D3DRS_LIGHTING, FALSE);
		}
		if (dwFlags & D3DDP_DONOTUPDATEEXTENTS)
		{
			// ToDo: fix Extents see SetRenderState() implementation
			//GetRenderState(D3DRENDERSTATE_EXTENTS, &TempStates.rsExtents);
			//SetRenderState(D3DRENDERSTATE_EXTENTS, FALSE);
		}
	}

	// Prepare render target surface
	if (lpCurrentRenderTargetX)
	{
		lpCurrentRenderTargetX->PrepareRenderTarget();

		if (ddrawParent->GetRenderTargetSurface() != lpCurrentRenderTargetX)
		{
			ddrawParent->SetRenderTargetSurface(lpCurrentRenderTargetX);
		}

		if (DeviceStates.RenderState[D3DRS_ZENABLE].Set)
		{
			(*d3d9Device)->SetRenderState(D3DRS_ZENABLE, DeviceStates.RenderState[D3DRS_ZENABLE].State);
		}
	}

	// Need to always set viewport
	(*d3d9Device)->SetViewport(&DeviceStates.Viewport.View);

	// Handle texture wrapping
	if (rsWrapChanged)
	{
		DWORD RenderState = (DeviceStates.RenderState[D3DRENDERSTATE_WRAPU].State ? D3DWRAP_U : 0) | (DeviceStates.RenderState[D3DRENDERSTATE_WRAPV].State ? D3DWRAP_V : 0);
		SetD9RenderState(D3DRS_WRAP0, RenderState);
		rsWrapChanged = false;
	}

	// Handle anti-aliasing
	if (rsAntiAliasChanged)
	{
		DWORD rsAntiAlias = DeviceStates.RenderState[D3DRENDERSTATE_ANTIALIAS].State;
		BOOL AntiAliasEnabled = (bool)((D3DANTIALIASMODE)rsAntiAlias == D3DANTIALIAS_SORTDEPENDENT || (D3DANTIALIASMODE)rsAntiAlias == D3DANTIALIAS_SORTINDEPENDENT);
		SetD9RenderState(D3DRS_MULTISAMPLEANTIALIAS, AntiAliasEnabled);
		rsAntiAliasChanged = false;
	}

	if (Config.DdrawFixByteAlignment > 1)
	{
		for (UINT x = 0; x < D3DHAL_TSS_MAXSTAGES; x++)
		{
			if (CurrentTextureSurfaceX[x] && CurrentTextureSurfaceX[x]->GetWasBitAlignLocked())
			{
				(*d3d9Device)->GetSamplerState(x, D3DSAMP_MINFILTER, &TempStates.ssMinFilter[x]);
				(*d3d9Device)->GetSamplerState(x, D3DSAMP_MAGFILTER, &TempStates.ssMagFilter[x]);

				(*d3d9Device)->SetSamplerState(x, D3DSAMP_MINFILTER, Config.DdrawFixByteAlignment == 2 ? D3DTEXF_POINT : D3DTEXF_LINEAR);
				(*d3d9Device)->SetSamplerState(x, D3DSAMP_MAGFILTER, Config.DdrawFixByteAlignment == 2 ? D3DTEXF_POINT : D3DTEXF_LINEAR);
			}
		}
	}
	for (UINT x = 0; x < D3DHAL_TSS_MAXSTAGES; x++)
	{
		// Set textures
		if (CurrentTextureSurfaceX[x])
		{
			IDirect3DTexture9* pTexture9 = CurrentTextureSurfaceX[x]->GetD3d9Texture();
			if (pTexture9)
			{
				(*d3d9Device)->SetTexture(x, pTexture9);
			}
			// Generate MipMap levels
			if (DeviceStates.TextureStageState[x][D3DTSS_MIPFILTER].State != D3DTEXF_NONE && !CurrentTextureSurfaceX[x]->IsMipMapGenerated())
			{
				CurrentTextureSurfaceX[x]->GenerateMipMapLevels();
			}
		}
		else
		{
			(*d3d9Device)->SetTexture(x, nullptr);
		}
	}
	if (DeviceStates.RenderState[D3DRENDERSTATE_COLORKEYENABLE].State)
	{
		// Check for color key alpha texture
		for (UINT x = 0; x < D3DHAL_TSS_MAXSTAGES; x++)
		{
			if (CurrentTextureSurfaceX[x] && CurrentTextureSurfaceX[x]->IsColorKeyTexture() && CurrentTextureSurfaceX[x]->GetD3d9DrawTexture())
			{
				dwFlags |= D3DDP_DXW_ALPHACOLORKEY;
				(*d3d9Device)->SetTexture(x, CurrentTextureSurfaceX[x]->GetD3d9DrawTexture());
			}
		}
		if (dwFlags & D3DDP_DXW_ALPHACOLORKEY)
		{
			(*d3d9Device)->GetRenderState(D3DRS_ALPHATESTENABLE, &TempStates.rsAlphaTestEnable);
			(*d3d9Device)->GetRenderState(D3DRS_ALPHAFUNC, &TempStates.rsAlphaFunc);
			(*d3d9Device)->GetRenderState(D3DRS_ALPHAREF, &TempStates.rsAlphaRef);

			(*d3d9Device)->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
			(*d3d9Device)->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
			(*d3d9Device)->SetRenderState(D3DRS_ALPHAREF, (DWORD)0x01);
		}
	}
	if ((dwFlags & D3DDP_DXW_COLORKEYENABLE) && ddrawParent)
	{
		if (!colorkeyPixelShader || !*colorkeyPixelShader)
		{
			colorkeyPixelShader = ddrawParent->GetColorKeyShader();
		}
		if (colorkeyPixelShader && *colorkeyPixelShader)
		{
			(*d3d9Device)->SetPixelShader(*colorkeyPixelShader);
			(*d3d9Device)->SetPixelShaderConstantF(0, TempStates.lowColorKey, 1);
			(*d3d9Device)->SetPixelShaderConstantF(1, TempStates.highColorKey, 1);
		}
	}
	/*if ((dwVertexTypeDesc & D3DFVF_XYZRHW) && d3d9Device && *d3d9Device && ddrawParent)
	{
		if (!fixupVertexShader || !*fixupVertexShader)
		{
			fixupVertexShader = ddrawParent->GetVertexFixupShader();
		}
		if (fixupVertexShader && *fixupVertexShader)
		{
			// Set pixel center offset (-0.5 if AlternatePixelCenter is enabled)
			float pixelOffset[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			if (Config.DdrawAlternatePixelCenter)
			{
				pixelOffset[0] = -0.5f;
				pixelOffset[1] = -0.5f;
			}
			(*d3d9Device)->SetVertexShaderConstantF(1, pixelOffset, 1);

			// Set vertex shader
			(*d3d9Device)->SetVertexShader(*fixupVertexShader);
		}
	}*/
}

void m_IDirect3DDeviceX::RestoreDrawStates(DWORD dwVertexTypeDesc, DWORD dwFlags, DWORD DirectXVersion)
{
	// Handle dwFlags
	if (DirectXVersion < 7)
	{
		if (dwFlags & D3DDP_DONOTCLIP)
		{
			(*d3d9Device)->SetRenderState(D3DRS_CLIPPING, TempStates.rsClipping);
		}
		if ((dwFlags & D3DDP_DONOTLIGHT) || !(dwVertexTypeDesc & D3DFVF_NORMAL))
		{
			(*d3d9Device)->SetRenderState(D3DRS_LIGHTING, TempStates.rsLighting);
		}
		if (dwFlags & D3DDP_DONOTUPDATEEXTENTS)
		{
			// ToDo: fix Extents see SetRenderState() implementation
			//SetRenderState(D3DRENDERSTATE_EXTENTS, TempStates.rsExtents);
		}
	}
	if (Config.DdrawFixByteAlignment > 1)
	{
		for (UINT x = 0; x < D3DHAL_TSS_MAXSTAGES; x++)
		{
			if (CurrentTextureSurfaceX[x] && CurrentTextureSurfaceX[x]->GetWasBitAlignLocked())
			{
				(*d3d9Device)->SetSamplerState(x, D3DSAMP_MINFILTER, TempStates.ssMinFilter[x]);
				(*d3d9Device)->SetSamplerState(x, D3DSAMP_MAGFILTER, TempStates.ssMagFilter[x]);
			}
		}
	}
	if (dwFlags & D3DDP_DXW_ALPHACOLORKEY)
	{
		(*d3d9Device)->SetRenderState(D3DRS_ALPHATESTENABLE, TempStates.rsAlphaTestEnable);
		(*d3d9Device)->SetRenderState(D3DRS_ALPHAFUNC, TempStates.rsAlphaFunc);
		(*d3d9Device)->SetRenderState(D3DRS_ALPHAREF, TempStates.rsAlphaRef);
	}
	if (dwFlags & D3DDP_DXW_COLORKEYENABLE)
	{
		(*d3d9Device)->SetPixelShader(nullptr);
	}
	/*if ((dwVertexTypeDesc & D3DFVF_XYZRHW) && d3d9Device && *d3d9Device)
	{
		(*d3d9Device)->SetVertexShader(nullptr);
	}*/
}

bool m_IDirect3DDeviceX::IsLightInUse(m_IDirect3DLight* pLightX)
{
	if (ClientDirectXVersion != 7)
	{
		if (lpCurrentViewportX && lpCurrentViewportX->IsLightAttached(pLightX))
		{
			return true;
		}
	}

	return false;
}

void m_IDirect3DDeviceX::GetEnabledLightList(std::vector<DXLIGHT7>& AttachedLightList)
{
	if (ClientDirectXVersion == 7)
	{
		for (auto& entry : DeviceStates.Light)
		{
			if (entry.second.Set && entry.second.Enable)
			{
				DXLIGHT7 DxLight7 = {};
				*reinterpret_cast<D3DLIGHT9*>(&DxLight7) = entry.second.Light;
				DxLight7.dwLightVersion = 7;
				DxLight7.dwFlags = 0;

				AttachedLightList.push_back(DxLight7);
			}
		}
	}
	else if (lpCurrentViewportX)
	{
		lpCurrentViewportX->GetEnabledLightList(AttachedLightList, this);
	}
}

void m_IDirect3DDeviceX::ScaleVertices(DWORD dwVertexTypeDesc, LPVOID& lpVertices, DWORD dwVertexCount)
{
	if (dwVertexTypeDesc == 3)
	{
		VertexCache.resize(dwVertexCount * sizeof(D3DTLVERTEX));
		memcpy(VertexCache.data(), lpVertices, dwVertexCount * sizeof(D3DTLVERTEX));
		D3DTLVERTEX* pVert = (D3DTLVERTEX*)VertexCache.data();

		for (DWORD x = 0; x < dwVertexCount; x++)
		{
			pVert[x].sx = (D3DVALUE)(pVert[x].sx * ScaleDDWidthRatio) + ScaleDDPadX;
			pVert[x].sy = (D3DVALUE)(pVert[x].sy * ScaleDDHeightRatio) + ScaleDDPadY;
		}

		lpVertices = pVert;
	}
}

void m_IDirect3DDeviceX::UpdateVertices(DWORD& dwVertexTypeDesc, LPVOID& lpVertices, DWORD dwVertexStart, DWORD dwNumVertices)
{
	if (dwVertexTypeDesc == D3DFVF_LVERTEX)
	{
		VertexCache.resize((dwVertexStart + dwNumVertices) * sizeof(DXLVERTEX9));
		ConvertLVertex(reinterpret_cast<DXLVERTEX9*>(VertexCache.data() + (dwVertexStart * sizeof(DXLVERTEX9))),
			reinterpret_cast<DXLVERTEX7*>((DWORD)lpVertices + (dwVertexStart * sizeof(DXLVERTEX7))),
			dwNumVertices);

		dwVertexTypeDesc = D3DFVF_LVERTEX9;
		lpVertices = VertexCache.data();
	}
	else if (Config.DdrawClampVertexZDepth && (dwVertexTypeDesc & D3DFVF_XYZRHW))
	{
		DWORD stride = GetVertexStride(dwVertexTypeDesc);
		VertexCache.resize((dwVertexStart + dwNumVertices) * stride);
		memcpy(reinterpret_cast<void*>(VertexCache.data() + (dwVertexStart * stride)),
			reinterpret_cast<void*>((DWORD)lpVertices + (dwVertexStart * stride)),
			dwNumVertices * stride);

		ClampVertices(VertexCache.data(), stride, dwNumVertices);

		lpVertices = VertexCache.data();
	}
}

bool m_IDirect3DDeviceX::InterleaveStridedVertexData(std::vector<BYTE, aligned_allocator<BYTE, 4>>& outputBuffer, const D3DDRAWPRIMITIVESTRIDEDDATA* sd, const DWORD dwVertexStart, const DWORD dwNumVertices, const DWORD dwVertexTypeDesc)
{
	if (!sd)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: missing D3DDRAWPRIMITIVESTRIDEDDATA!");
		return false;
	}

	DWORD Stride = GetVertexStride(dwVertexTypeDesc);

	bool hasPosition = (dwVertexTypeDesc & D3DFVF_POSITION_MASK);
	bool hasReserved = (dwVertexTypeDesc & D3DFVF_RESERVED1);
	bool hasNormal = (dwVertexTypeDesc & D3DFVF_NORMAL);
	bool hasDiffuse = (dwVertexTypeDesc & D3DFVF_DIFFUSE);
	bool hasSpecular = (dwVertexTypeDesc & D3DFVF_SPECULAR);
	DWORD texCount = D3DFVF_TEXCOUNT(dwVertexTypeDesc);

	if (texCount > D3DDP_MAXTEXCOORD)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: texCount " << texCount << " exceeds D3DDP_MAXTEXCOORD!");
		return false;
	}

	UINT posStride = GetVertexPositionStride(dwVertexTypeDesc);
	UINT texStride[D3DDP_MAXTEXCOORD] = {};

	// Check data and compute stride
	if (hasPosition)
	{
		if (!sd->position.lpvData)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: position data missing! FVF: " << Logging::hex(dwVertexTypeDesc));
			return false;
		}
		if (sd->position.dwStride != posStride)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: position stride does not match: " << posStride << " -> " << sd->position.dwStride << " FVF: " << Logging::hex(dwVertexTypeDesc));
			return false;
		}
	}
	if (hasNormal)
	{
		if (!sd->normal.lpvData)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: normal data missing! FVF: " << Logging::hex(dwVertexTypeDesc));
			return false;
		}
		if (sd->normal.dwStride != sizeof(D3DXVECTOR3))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: normal stride does not match: " << sizeof(D3DXVECTOR3) << " -> " << sd->normal.dwStride << " FVF: " << Logging::hex(dwVertexTypeDesc));
			return false;
		}
	}
	if (hasDiffuse)
	{
		if (!sd->diffuse.lpvData)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: diffuse data missing! FVF: " << Logging::hex(dwVertexTypeDesc));
			return false;
		}
		if (sd->diffuse.dwStride != sizeof(D3DCOLOR))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: diffuse stride does not match: " << sizeof(D3DCOLOR) << " -> " << sd->diffuse.dwStride << " FVF: " << Logging::hex(dwVertexTypeDesc));
			return false;
		}
	}
	if (hasSpecular)
	{
		if (!sd->specular.lpvData)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: specular data missing! FVF: " << Logging::hex(dwVertexTypeDesc));
			return false;
		}
		if (sd->specular.dwStride != sizeof(D3DCOLOR))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: specular stride does not match: " << sizeof(D3DCOLOR) << " -> " << sd->specular.dwStride << " FVF: " << Logging::hex(dwVertexTypeDesc));
			return false;
		}
	}
	for (DWORD t = 0; t < texCount; ++t)
	{
		if (!sd->textureCoords[t].lpvData)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: textureCoords " << t << " data missing! FVF: " << Logging::hex(dwVertexTypeDesc));
			return false;
		}
		texStride[t] = GetTexStride(dwVertexTypeDesc, t);
		if (sd->textureCoords[t].dwStride != texStride[t])
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: specular stride does not match: " << texStride[t] << " -> " << sd->textureCoords[t].dwStride << " FVF: " << Logging::hex(dwVertexTypeDesc));
			return false;
		}
	}

	outputBuffer.resize((dwVertexStart + dwNumVertices) * Stride);

	BYTE* cursor = outputBuffer.data() + dwVertexStart * Stride;
	BYTE* posCursor = reinterpret_cast<BYTE*>(sd->position.lpvData) + dwVertexStart * sd->position.dwStride;
	const D3DXVECTOR3* normalCursor = reinterpret_cast<D3DXVECTOR3*>(sd->normal.lpvData) + dwVertexStart * sd->normal.dwStride;
	const D3DCOLOR* diffCursor = reinterpret_cast<D3DCOLOR*>(sd->diffuse.lpvData) + dwVertexStart * sd->diffuse.dwStride;
	const D3DCOLOR* specCursor = reinterpret_cast<D3DCOLOR*>(sd->specular.lpvData) + dwVertexStart * sd->specular.dwStride;
	BYTE* texCursor[D3DDP_MAXTEXCOORD] = {};
	for (DWORD t = 0; t < texCount; ++t)
	{
		texCursor[t] = reinterpret_cast<BYTE*>(sd->textureCoords[t].lpvData) + dwVertexStart * sd->textureCoords[t].dwStride;
	}

	if (dwVertexTypeDesc == (D3DFVF_XYZ | D3DFVF_DIFFUSE))
	{
		for (DWORD i = 0; i < dwNumVertices; ++i)
		{
			// Position
			*(D3DXVECTOR3*)cursor = ((D3DXVECTOR3*)posCursor)[i];
			cursor += sizeof(D3DXVECTOR3);

			// Diffuse
			*(D3DCOLOR*)cursor = diffCursor[i];
			cursor += sizeof(D3DCOLOR);
		}
	}
	else if (dwVertexTypeDesc == (D3DFVF_XYZ | D3DFVF_TEX1) && texCursor[0])
	{
		for (DWORD i = 0; i < dwNumVertices; ++i)
		{
			// Position
			*(D3DXVECTOR3*)cursor = ((D3DXVECTOR3*)posCursor)[i];
			cursor += sizeof(D3DXVECTOR3);

			// Texture
			*(D3DXVECTOR2*)cursor = ((D3DXVECTOR2*)texCursor[0])[i];
			cursor += sizeof(D3DXVECTOR2);
		}
	}
	else if (dwVertexTypeDesc == (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1) && texCursor[0])
	{
		for (DWORD i = 0; i < dwNumVertices; ++i)
		{
			// Position
			*(D3DXVECTOR3*)cursor = ((D3DXVECTOR3*)posCursor)[i];
			cursor += sizeof(D3DXVECTOR3);

			// Normal
			*(D3DXVECTOR3*)cursor = normalCursor[i];
			cursor += sizeof(D3DXVECTOR3);

			// Texture
			*(D3DXVECTOR2*)cursor = ((D3DXVECTOR2*)texCursor[0])[i];
			cursor += sizeof(D3DXVECTOR2);
		}
	}
	else if (dwVertexTypeDesc == (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1) && texCursor[0])
	{
		for (DWORD i = 0; i < dwNumVertices; ++i)
		{
			// Position
			*(D3DXVECTOR3*)cursor = ((D3DXVECTOR3*)posCursor)[i];
			cursor += sizeof(D3DXVECTOR3);

			// Diffuse
			*(D3DCOLOR*)cursor = diffCursor[i];
			cursor += sizeof(D3DCOLOR);

			// Texture
			*(D3DXVECTOR2*)cursor = ((D3DXVECTOR2*)texCursor[0])[i];
			cursor += sizeof(D3DXVECTOR2);
		}
	}
	else if (dwVertexTypeDesc == (D3DFVF_XYZRHW | D3DFVF_TEX1) && texCursor[0])
	{
		for (DWORD i = 0; i < dwNumVertices; ++i)
		{
			// Position
			*(D3DXVECTOR4*)cursor = ((D3DXVECTOR4*)posCursor)[i];
			cursor += sizeof(D3DXVECTOR4);

			// Texture
			*(D3DXVECTOR2*)cursor = ((D3DXVECTOR2*)texCursor[0])[i];
			cursor += sizeof(D3DXVECTOR2);
		}
	}
	else if (dwVertexTypeDesc == (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1) && texCursor[0])
	{
		for (DWORD i = 0; i < dwNumVertices; ++i)
		{
			// Position
			*(D3DXVECTOR4*)cursor = ((D3DXVECTOR4*)posCursor)[i];
			cursor += sizeof(D3DXVECTOR4);

			// Diffuse
			*(D3DCOLOR*)cursor = diffCursor[i];
			cursor += sizeof(D3DCOLOR);

			// Texture
			*(D3DXVECTOR2*)cursor = ((D3DXVECTOR2*)texCursor[0])[i];
			cursor += sizeof(D3DXVECTOR2);
		}
	}
	else if (dwVertexTypeDesc == (D3DFVF_XYZ | D3DFVF_RESERVED1 | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1) && texCursor[0])
	{
		for (DWORD i = 0; i < dwNumVertices; ++i)
		{
			// Position
			*(D3DXVECTOR3*)cursor = ((D3DXVECTOR3*)posCursor)[i];
			cursor += sizeof(D3DXVECTOR3);

			// Reserved
			cursor += sizeof(DWORD);

			// Diffuse
			*(D3DCOLOR*)cursor = diffCursor[i];
			cursor += sizeof(D3DCOLOR);

			// Specular
			*(D3DCOLOR*)cursor = specCursor[i];
			cursor += sizeof(D3DCOLOR);

			// Texture
			*(D3DXVECTOR2*)cursor = ((D3DXVECTOR2*)texCursor[0])[i];
			cursor += sizeof(D3DXVECTOR2);
		}
	}
	else if (dwVertexTypeDesc == (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1) && texCursor[0])
	{
		for (DWORD i = 0; i < dwNumVertices; ++i)
		{
			// Position
			*(D3DXVECTOR3*)cursor = ((D3DXVECTOR3*)posCursor)[i];
			cursor += sizeof(D3DXVECTOR3);

			// Diffuse
			*(D3DCOLOR*)cursor = diffCursor[i];
			cursor += sizeof(D3DCOLOR);

			// Specular
			*(D3DCOLOR*)cursor = specCursor[i];
			cursor += sizeof(D3DCOLOR);

			// Texture
			*(D3DXVECTOR2*)cursor = ((D3DXVECTOR2*)texCursor[0])[i];
			cursor += sizeof(D3DXVECTOR2);
		}
	}
	else if (dwVertexTypeDesc == (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1) && texCursor[0])
	{
		for (DWORD i = 0; i < dwNumVertices; ++i)
		{
			// Position
			*(D3DXVECTOR3*)cursor = ((D3DXVECTOR3*)posCursor)[i];
			cursor += sizeof(D3DXVECTOR3);

			// Normal
			*(D3DXVECTOR3*)cursor = normalCursor[i];
			cursor += sizeof(D3DXVECTOR3);

			// Diffuse
			*(D3DCOLOR*)cursor = diffCursor[i];
			cursor += sizeof(D3DCOLOR);

			// Specular
			*(D3DCOLOR*)cursor = specCursor[i];
			cursor += sizeof(D3DCOLOR);

			// Texture
			*(D3DXVECTOR2*)cursor = ((D3DXVECTOR2*)texCursor[0])[i];
			cursor += sizeof(D3DXVECTOR2);
		}
	}
	else
	{
		LOG_LIMIT(100, __FUNCTION__ << " Warning: Non-optimized vertex interleaving. FVF: " << Logging::hex(dwVertexTypeDesc));

		for (DWORD i = 0; i < dwNumVertices; ++i)
		{
			if (hasPosition)
			{
				memcpy(cursor, posCursor, posStride);
				cursor += posStride;
				posCursor += sd->position.dwStride;
			}

			if (hasReserved)
			{
				cursor += sizeof(DWORD);
			}

			if (hasNormal)
			{
				*(D3DXVECTOR3*)cursor = normalCursor[i];
				cursor += sizeof(D3DXVECTOR3);
			}

			if (hasDiffuse)
			{
				*(D3DCOLOR*)cursor = diffCursor[i];
				cursor += sizeof(D3DCOLOR);
			}

			if (hasSpecular)
			{
				*(D3DCOLOR*)cursor = specCursor[i];
				cursor += sizeof(D3DCOLOR);
			}

			for (DWORD t = 0; t < texCount; ++t)
			{
				memcpy(cursor, texCursor[t], texStride[t]);
				cursor += texStride[t];
				texCursor[t] += sd->textureCoords[t].dwStride;
			}
		}
	}

	return true;
}
