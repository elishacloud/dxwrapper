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

#include "ddraw.h"

HRESULT m_IDirect3DVertexBufferX::QueryInterface(REFIID riid, LPVOID FAR * ppvObj, DWORD DirectXVersion)
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

	if (DirectXVersion != 1 && DirectXVersion != 7)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
		return E_NOINTERFACE;
	}

	DWORD DxVersion = (CheckWrapperType(riid) && (Config.Dd7to9 || Config.ConvertToDirect3D7)) ? GetGUIDVersion(riid) : DirectXVersion;

	if (riid == GetWrapperType(DxVersion) || riid == IID_IUnknown)
	{
		*ppvObj = GetWrapperInterfaceX(DxVersion);

		AddRef(DxVersion);

		return D3D_OK;
	}

	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, GetWrapperType(DxVersion));
}

void *m_IDirect3DVertexBufferX::GetWrapperInterfaceX(DWORD DirectXVersion)
{
	switch (DirectXVersion)
	{
	case 1:
		if (!WrapperInterface)
		{
			WrapperInterface = new m_IDirect3DVertexBuffer((LPDIRECT3DVERTEXBUFFER)ProxyInterface, this);
		}
		return WrapperInterface;
	case 7:
		if (!WrapperInterface7)
		{
			WrapperInterface7 = new m_IDirect3DVertexBuffer7((LPDIRECT3DVERTEXBUFFER7)ProxyInterface, this);
		}
		return WrapperInterface7;
	default:
		LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
		return nullptr;
	}
}

ULONG m_IDirect3DVertexBufferX::AddRef(DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") v" << DirectXVersion;

	if (Config.Dd7to9)
	{
		switch (DirectXVersion)
		{
		case 1:
			return InterlockedIncrement(&RefCount1);
		case 7:
			return InterlockedIncrement(&RefCount7);
		default:
			LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
			return 0;
		}
	}

	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DVertexBufferX::Release(DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") v" << DirectXVersion;

	ULONG ref;

	if (Config.Dd7to9)
	{
		switch (DirectXVersion)
		{
		case 1:
			ref = (InterlockedCompareExchange(&RefCount1, 0, 0)) ? InterlockedDecrement(&RefCount1) : 0;
			break;
		case 7:
			ref = (InterlockedCompareExchange(&RefCount7, 0, 0)) ? InterlockedDecrement(&RefCount7) : 0;
			break;
		default:
			LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
			ref = 0;
		}

		if (InterlockedCompareExchange(&RefCount1, 0, 0) + InterlockedCompareExchange(&RefCount7, 0, 0) == 0)
		{
			delete this;
		}
	}
	else
	{
		ref = ProxyInterface->Release();

		if (ref == 0)
		{
			delete this;
		}
	}

	return ref;
}

HRESULT m_IDirect3DVertexBufferX::Lock(DWORD dwFlags, LPVOID* lplpData, LPDWORD lpdwSize)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lplpData)
		{
			return DDERR_INVALIDPARAMS;
		}

		*lplpData = nullptr;

		if (lpdwSize)
		{
			*lpdwSize = 0;
		}

		if (LastLockAddr)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: locking vertex buffer when buffer is already locked!");
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, false, true)))
		{
			return DDERR_GENERIC;
		}

		// Non-implemented dwFlags:
		// DDLOCK_WAIT & DDLOCK_SURFACEMEMORYPTR can be ignored safely
		// DDLOCK_WRITEONLY should be specified at create time. The presence of the D3DUSAGE_WRITEONLY flag in Usage indicates that the vertex buffer memory is used only for write operations.

		DWORD Flags = dwFlags & (DDLOCK_NOSYSLOCK | DDLOCK_READONLY | DDLOCK_DISCARDCONTENTS);

		void* pData = nullptr;
		HRESULT hr = d3d9VertexBuffer->Lock(0, 0, &pData, Flags);

		if (FAILED(hr))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock vertex buffer: " << (D3DERR)hr);
			return hr;
		}

		LastLockAddr = pData;
		LastLockFlags = Flags;

		// Handle D3DFVF_LVERTEX
		if (VBDesc.dwFVF == D3DFVF_LVERTEX)
		{
			*lplpData = VertexData.data();

			if (lpdwSize)
			{
				*lpdwSize = VertexData.size();
			}

			// Should not need to copy from vertex buffer??
			//ConvertVertices((D3DLVERTEX*)VertexData.data(), (D3DLVERTEX9*)pData, VBDesc.dwNumVertices);
		}
		else
		{
			*lplpData = pData;

			if (lpdwSize)
			{
				*lpdwSize = d3d9VBDesc.Size;
			}
		}

		return D3D_OK;
	}

	return ProxyInterface->Lock(dwFlags, lplpData, lpdwSize);
}

HRESULT m_IDirect3DVertexBufferX::Unlock()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, false, true)))
		{
			return DDERR_GENERIC;
		}

		// Handle D3DFVF_LVERTEX
		if (VBDesc.dwFVF == D3DFVF_LVERTEX && LastLockAddr && !(LastLockFlags & DDLOCK_READONLY))
		{
			ConvertVertices((D3DLVERTEX9*)LastLockAddr, (D3DLVERTEX*)VertexData.data(), VBDesc.dwNumVertices);
		}

		HRESULT hr = d3d9VertexBuffer->Unlock();

		if (FAILED(hr))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to unlock vertex buffer: " << (D3DERR)hr);
			return hr;
		}
		
		LastLockAddr = nullptr;

		return D3D_OK;
	}

	return ProxyInterface->Unlock();
}

HRESULT m_IDirect3DVertexBufferX::ProcessVertices(DWORD dwVertexOp, DWORD dwDestIndex, DWORD dwCount, LPDIRECT3DVERTEXBUFFER7 lpSrcBuffer, DWORD dwSrcIndex, LPDIRECT3DDEVICE7 lpD3DDevice, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Always include the D3DVOP_TRANSFORM flag in the dwVertexOp parameter. If you do not, the method fails, returning DDERR_INVALIDPARAMS.
		if (!lpSrcBuffer || !(dwVertexOp & D3DVOP_TRANSFORM))
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true, true)))
		{
			return DDERR_GENERIC;
		}

		// Handle dwFlags
		// D3DVOP_TRANSFORM is inherently handled by ProcessVertices() as it performs vertex transformations based on the current world, view, and projection matrices.
		if (dwVertexOp & D3DVOP_CLIP)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DVOP_CLIP' not handled!");
		}
		if (dwVertexOp & D3DVOP_LIGHT)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DVOP_LIGHT' not handled!");
		}
		if (dwVertexOp & D3DVOP_EXTENTS)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DVOP_EXTENTS' not handled!");
		}

		// ToDo: Validate vertex buffer
		m_IDirect3DVertexBufferX* pSrcVertexBufferX = nullptr;
		lpSrcBuffer->QueryInterface(IID_GetInterfaceX, (LPVOID*)&pSrcVertexBufferX);

		if (!pSrcVertexBufferX)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get source vertex buffer wrapper!");
			return DDERR_GENERIC;
		}

		LPDIRECT3DVERTEXBUFFER9 d3d9SrcVertexBuffer = pSrcVertexBufferX->GetCurrentD9VertexBuffer();
		if (!d3d9SrcVertexBuffer)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get d3d9 source vertex buffer!");
			return DDERR_GENERIC;
		}
		LPDIRECT3DVERTEXBUFFER9 d3d9DestVertexBuffer = d3d9VertexBuffer;

		// Get and verify FVF
		DWORD SrcFVF = pSrcVertexBufferX->GetFVF9();
		DWORD DestFVF = GetFVF9();
		if (FAILED((*d3d9Device)->SetFVF(SrcFVF)) || FAILED((*d3d9Device)->SetFVF(DestFVF)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not set vertex declaration: " << Logging::hex(SrcFVF) << " -> " << Logging::hex(DestFVF));
			return D3DERR_INVALIDVERTEXTYPE;
		}
		UINT SrcStride = GetVertexStride(SrcFVF);
		UINT DestStride = GetVertexStride(DestFVF);

		// Check the dwDestIndex, dwSrcIndex and dwCount to make sure they won't cause an overload
		DWORD SrcNumVertices = pSrcVertexBufferX->VBDesc.dwNumVertices;
		DWORD DestNumVertices = VBDesc.dwNumVertices;
		if (dwSrcIndex > SrcNumVertices || dwDestIndex > DestNumVertices)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: vertex index is too large: " <<
				SrcNumVertices << " -> " << dwSrcIndex << " " <<
				DestNumVertices << " -> " << dwDestIndex);
			return D3DERR_INVALIDVERTEXTYPE;
		}
		dwCount = min(dwCount, SrcNumVertices - dwSrcIndex);
		dwCount = min(dwCount, DestNumVertices - dwDestIndex);

		D3DMATRIX matWorldViewProj = {};
		{
			D3DMATRIX matWorld, matView, matProj, matWorldView = {};
			if (FAILED((*d3d9Device)->GetTransform(D3DTS_WORLD, &matWorld)) ||
				FAILED((*d3d9Device)->GetTransform(D3DTS_VIEW, &matView)) ||
				FAILED((*d3d9Device)->GetTransform(D3DTS_PROJECTION, &matProj)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to get world, view or projection matrix!");
				return DDERR_GENERIC;
			}

			// Multiply the world, view and projection matrices
			D3DXMatrixMultiply(&matWorldViewProj, D3DXMatrixMultiply(&matWorldView, &matWorld, &matView), &matProj);
		}

		void* pSrcVertices = nullptr;
		void* pDestVertices = nullptr;

		HRESULT hr = D3D_OK;

		do {
			// Lock the source vertex buffer
			if (FAILED(d3d9SrcVertexBuffer->Lock(0, 0, &pSrcVertices, D3DLOCK_READONLY)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock source vertex");
				hr = DDERR_GENERIC;
				break;
			}

			// Lock the destination vertex buffer
			if (FAILED(d3d9DestVertexBuffer->Lock(0, 0, &pDestVertices, 0)))
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock destination vertex");
				hr = DDERR_GENERIC;
				break;
			}

			bool CopyRHW = (((SrcFVF & D3DFVF_POSITION_MASK_9) == D3DFVF_XYZRHW && (DestFVF & D3DFVF_POSITION_MASK_9) == D3DFVF_XYZRHW) ||
				((SrcFVF & D3DFVF_POSITION_MASK_9) == D3DFVF_XYZW && (DestFVF & D3DFVF_POSITION_MASK_9) == D3DFVF_XYZW));

			BYTE* pSrcVertex = (BYTE*)pSrcVertices + (dwSrcIndex * SrcStride);
			BYTE* pDestVertex = (BYTE*)pDestVertices + (dwDestIndex * DestStride);

			// Copy only position data
			if ((dwFlags & D3DPV_DONOTCOPYDATA) && !CopyRHW)
			{
				for (UINT i = 0; i < dwCount; ++i)
				{
					// Apply the transformation to the position
					D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(pDestVertex), reinterpret_cast<D3DXVECTOR3*>(pSrcVertex), &matWorldViewProj);

					// Move to the next vertex
					pSrcVertex = pSrcVertex + SrcStride;
					pDestVertex = pDestVertex + DestStride;
				}
			}
			// Copy position and RHW/W data
			else if (dwFlags & D3DPV_DONOTCOPYDATA)
			{
				for (UINT i = 0; i < dwCount; ++i)
				{
					// Apply the transformation to the position
					D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(pDestVertex), reinterpret_cast<D3DXVECTOR3*>(pSrcVertex), &matWorldViewProj);

					// Copy RHW/W data
					*(float*)(pDestVertex + 3 * sizeof(float)) = *(float*)(pSrcVertex + 3 * sizeof(float));

					// Move to the next vertex
					pSrcVertex = pSrcVertex + SrcStride;
					pDestVertex = pDestVertex + DestStride;
				}
			}
			// Copy all data
			else if (SrcFVF == DestFVF)
			{
				memcpy(pDestVertex, pSrcVertex, DestStride * dwCount);

				for (UINT i = 0; i < dwCount; ++i)
				{
					// Apply the transformation to the position
					D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(pDestVertex), reinterpret_cast<D3DXVECTOR3*>(pSrcVertex), &matWorldViewProj);

					// Move to the next vertex
					pSrcVertex = pSrcVertex + SrcStride;
					pDestVertex = pDestVertex + DestStride;
				}
			}
			// Copy all data converting vertices
			else
			{
				for (UINT i = 0; i < dwCount; ++i)
				{
					// Convert and copy all vertex data
					ConvertVertex(pDestVertex, DestFVF, pSrcVertex, SrcFVF);

					// Apply the transformation to the position
					D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(pDestVertex), reinterpret_cast<D3DXVECTOR3*>(pSrcVertex), &matWorldViewProj);

					// Move to the next vertex
					pSrcVertex = pSrcVertex + SrcStride;
					pDestVertex = pDestVertex + DestStride;
				}
			}

			// Handle D3DFVF_LVERTEX
			if (VBDesc.dwFVF == D3DFVF_LVERTEX)
			{
				BYTE* pDestData = VertexData.data() + (dwDestIndex * LVERTEX_SIZE);
				BYTE* pSrcData = (BYTE*)pDestVertices + (dwDestIndex * DestStride);		// Destination vertex is the source for this copy

				ConvertVertices((D3DLVERTEX*)pDestData, (D3DLVERTEX9*)pSrcData, dwCount);
			}

		} while (false);

		if (pSrcVertices)
		{
			d3d9SrcVertexBuffer->Unlock();
		}
		if (pDestVertices)
		{
			d3d9DestVertexBuffer->Unlock();
		}

		return hr;
	}

	if (lpSrcBuffer)
	{
		lpSrcBuffer->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpSrcBuffer);
	}
	if (lpD3DDevice)
	{
		lpD3DDevice->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpD3DDevice);
	}

	return ProxyInterface->ProcessVertices(dwVertexOp, dwDestIndex, dwCount, lpSrcBuffer, dwSrcIndex, lpD3DDevice, dwFlags);
}

HRESULT m_IDirect3DVertexBufferX::GetVertexBufferDesc(LPD3DVERTEXBUFFERDESC lpVBDesc)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpVBDesc)
		{
			return DDERR_INVALIDPARAMS;
		}

		if (VBDesc.dwSize != sizeof(D3DVERTEXBUFFERDESC))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: invalid vertex buffer desc: " << VBDesc);
			return DDERR_GENERIC;
		}

		lpVBDesc->dwSize = sizeof(D3DVERTEXBUFFERDESC);
		lpVBDesc->dwCaps = VBDesc.dwCaps;
		lpVBDesc->dwFVF = VBDesc.dwFVF;
		lpVBDesc->dwNumVertices = VBDesc.dwNumVertices;

		return D3D_OK;
	}

	return ProxyInterface->GetVertexBufferDesc(lpVBDesc);
}

HRESULT m_IDirect3DVertexBufferX::Optimize(LPDIRECT3DDEVICE7 lpD3DDevice, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// The Optimize function doesn't exist in Direct3D9 because it manages vertex buffer optimizations internally
		return D3D_OK;
	}

	if (lpD3DDevice)
	{
		lpD3DDevice->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpD3DDevice);
	}

	return ProxyInterface->Optimize(lpD3DDevice, dwFlags);
}

HRESULT m_IDirect3DVertexBufferX::ProcessVerticesStrided(DWORD dwVertexOp, DWORD dwDestIndex, DWORD dwCount, LPD3DDRAWPRIMITIVESTRIDEDDATA lpVertexArray, DWORD dwSrcIndex, LPDIRECT3DDEVICE7 lpD3DDevice, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	if (lpD3DDevice)
	{
		lpD3DDevice->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpD3DDevice);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
		return DDERR_GENERIC;
	case 7:
		return ProxyInterface->ProcessVerticesStrided(dwVertexOp, dwDestIndex, dwCount, lpVertexArray, dwSrcIndex, lpD3DDevice, dwFlags);
	default:
		return DDERR_GENERIC;
	}
}

/************************/
/*** Helper functions ***/
/************************/

void m_IDirect3DVertexBufferX::InitVertexBuffer(DWORD DirectXVersion)
{
	if (!Config.Dd7to9)
	{
		return;
	}

	if (ddrawParent)
	{
		ddrawParent->AddVertexBufferToVector(this);

		d3d9Device = ddrawParent->GetDirectD9Device();
	}

	AddRef(DirectXVersion);
}

void m_IDirect3DVertexBufferX::ReleaseVertexBuffer()
{
	if (WrapperInterface)
	{
		WrapperInterface->DeleteMe();
	}
	if (WrapperInterface7)
	{
		WrapperInterface7->DeleteMe();
	}

	ReleaseD9Buffers(false, false);

	if (ddrawParent && !Config.Exiting)
	{
		ddrawParent->RemoveVertexBufferFromVector(this);
	}
}

HRESULT m_IDirect3DVertexBufferX::CheckInterface(char* FunctionName, bool CheckD3DDevice, bool CheckD3DVertexBuffer)
{
	// Check for ddraw parent
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
	}

	// Check for vertex buffer, if not then create it
	if (CheckD3DVertexBuffer && !d3d9VertexBuffer)
	{
		// Create d3d9 vertex buffer
		if (FAILED(CreateD3D9VertexBuffer()))
		{
			LOG_LIMIT(100, FunctionName << " Error: could not create d3d9 vertex buffer!");
			return DDERR_GENERIC;
		}
	}

	return D3D_OK;
}

HRESULT m_IDirect3DVertexBufferX::CreateD3D9VertexBuffer()
{
	// Release existing buffer
	ReleaseD3D9VertexBuffer();

	// Check for device interface
	if (FAILED(CheckInterface(__FUNCTION__, true, false)))
	{
		return DDERR_GENERIC;
	}

	// ToDo: implement D3DVBCAPS_OPTIMIZED

	d3d9VBDesc.FVF = (VBDesc.dwFVF == D3DFVF_LVERTEX) ? D3DFVF_LVERTEX9 : VBDesc.dwFVF;
	d3d9VBDesc.Size = GetVertexStride(d3d9VBDesc.FVF) * VBDesc.dwNumVertices;
	d3d9VBDesc.Usage = D3DUSAGE_DYNAMIC |
		((VBDesc.dwCaps & D3DVBCAPS_WRITEONLY) ? D3DUSAGE_WRITEONLY : 0) |
		((VBDesc.dwCaps & D3DVBCAPS_DONOTCLIP) ? D3DUSAGE_DONOTCLIP : 0);
	d3d9VBDesc.Pool = (VBDesc.dwCaps & D3DVBCAPS_SYSTEMMEMORY) ? D3DPOOL_SYSTEMMEM : D3DPOOL_DEFAULT;

	HRESULT hr = (*d3d9Device)->CreateVertexBuffer(d3d9VBDesc.Size, d3d9VBDesc.Usage, d3d9VBDesc.FVF, d3d9VBDesc.Pool, &d3d9VertexBuffer, nullptr);
	if (FAILED(hr))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create vertex buffer: " << (D3DERR)hr <<
			" Length: " << d3d9VBDesc.Size << " Usage: " << Logging::hex(d3d9VBDesc.Usage) << " FVF: " << Logging::hex(d3d9VBDesc.FVF) << " Pool: " << Logging::hex(d3d9VBDesc.Pool) <<
			VBDesc);
		return DDERR_GENERIC;
	}

	if (VBDesc.dwFVF == D3DFVF_LVERTEX)
	{
		VertexData.resize(LVERTEX_SIZE * VBDesc.dwNumVertices);
	}
	else if (VertexData.size() == d3d9VBDesc.Size)
	{
		// ToDo: restore vertex buffer data
	}

	LastLockAddr = nullptr;

	return D3D_OK;
}

void m_IDirect3DVertexBufferX::ReleaseD3D9VertexBuffer()
{
	// Release vertex buffer
	if (d3d9VertexBuffer)
	{
		ULONG ref = d3d9VertexBuffer->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " (" << this << ")" << " Error: there is still a reference to 'd3d9VertexBuffer' " << ref;
		}
		d3d9VertexBuffer = nullptr;
	}
}

LPDIRECT3DINDEXBUFFER9 m_IDirect3DVertexBufferX::SetupIndexBuffer(LPWORD lpwIndices, DWORD dwIndexCount)
{
	if (!lpwIndices)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: nullptr Indices!");
		return nullptr;
	}

	// Check for device interface
	if (FAILED(CheckInterface(__FUNCTION__, true, false)))
	{
		return nullptr;
	}

	DWORD NewIndexSize = dwIndexCount * sizeof(WORD);

	HRESULT hr = D3D_OK;
	if (!d3d9IndexBuffer || NewIndexSize > IndexBufferSize)
	{
		ReleaseD3D9IndexBuffer();
		hr = (*d3d9Device)->CreateIndexBuffer(NewIndexSize, ((VBDesc.dwCaps & D3DVBCAPS_DONOTCLIP) ? D3DUSAGE_DONOTCLIP : 0), D3DFMT_INDEX16, D3DPOOL_SYSTEMMEM, &d3d9IndexBuffer, nullptr);
	}

	if (FAILED(hr))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create index buffer: " << (D3DERR)hr << " Size: " << NewIndexSize);
		return nullptr;
	}

	if (NewIndexSize > IndexBufferSize)
	{
		IndexBufferSize = NewIndexSize;
	}

	void* pData = nullptr;
	hr = d3d9IndexBuffer->Lock(0, NewIndexSize, &pData, 0);

	if (FAILED(hr))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock index buffer: " << (D3DERR)hr);
		return nullptr;
	}

	memcpy(pData, lpwIndices, NewIndexSize);

	d3d9IndexBuffer->Unlock();

	return d3d9IndexBuffer;
}

void m_IDirect3DVertexBufferX::ReleaseD3D9IndexBuffer()
{
	// Release index buffer
	if (d3d9IndexBuffer)
	{
		ULONG ref = d3d9IndexBuffer->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " (" << this << ")" << " Error: there is still a reference to 'd3d9IndexBuffer' " << ref;
		}
		d3d9IndexBuffer = nullptr;
		IndexBufferSize = 0;
	}
}

void m_IDirect3DVertexBufferX::ReleaseD9Buffers(bool BackupData, bool ResetBuffer)
{
	if (BackupData && VBDesc.dwFVF != D3DFVF_LVERTEX)
	{
		// ToDo: backup vertex buffer data
		//VertexData.resize(d3d9VBDesc.Size);
	}

	if (!ResetBuffer || d3d9VBDesc.Pool == D3DPOOL_DEFAULT)
	{
		ReleaseD3D9VertexBuffer();
	}
	if (!ResetBuffer)
	{
		ReleaseD3D9IndexBuffer();
	}
}
