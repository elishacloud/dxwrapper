/**
* Copyright (C) 2022 Elisha Riedlinger
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

	if (riid == IID_GetRealInterface)
	{
		*ppvObj = ProxyInterface;
		return DD_OK;
	}
	if (riid == IID_GetInterfaceX)
	{
		*ppvObj = this;
		return DD_OK;
	}

	if (DirectXVersion != 1 && DirectXVersion != 7)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
		return DDERR_GENERIC;
	}

	DWORD DxVersion = (CheckWrapperType(riid) && (Config.Dd7to9 || Config.ConvertToDirect3D7)) ? GetGUIDVersion(riid) : DirectXVersion;

	if (riid == GetWrapperType(DxVersion) || riid == IID_IUnknown)
	{
		*ppvObj = GetWrapperInterfaceX(DxVersion);

		AddRef(DxVersion);

		return DD_OK;
	}

	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, GetWrapperType(DxVersion));
}

void *m_IDirect3DVertexBufferX::GetWrapperInterfaceX(DWORD DirectXVersion)
{
	switch (DirectXVersion)
	{
	case 1:
		return WrapperInterface;
	case 7:
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
			*lplpData = &VertexData[0];

			if (lpdwSize)
			{
				*lpdwSize = VertexData.size();
			}

			// Should not need to copy from vertex buffer??
			//ConvertVertices((D3DLVERTEX*)&VertexData[0], (D3DLVERTEX9*)pData, VBDesc.dwNumVertices);
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
			ConvertVertices((D3DLVERTEX9*)LastLockAddr, (D3DLVERTEX*)&VertexData[0], VBDesc.dwNumVertices);
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
		if (!lpSrcBuffer || !lpD3DDevice || !(dwFlags & D3DVOP_TRANSFORM))
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true, true)))
		{
			return DDERR_GENERIC;
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

		DWORD FVF = pSrcVertexBufferX->GetFVF9();

		// Set fixed function vertex type
		if (FAILED((*d3d9Device)->SetFVF(FVF)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: invalid FVF type: " << Logging::hex(FVF));
			return D3DERR_INVALIDVERTEXTYPE;
		}

		// Set stream source
		(*d3d9Device)->SetStreamSource(0, d3d9SrcVertexBuffer, 0, GetVertexStride(FVF));

		// Handle dwFlags
		DWORD rsClipping = 0, rsLighting = 0, rsExtents = 0;
		(*d3d9Device)->GetRenderState(D3DRENDERSTATE_CLIPPING, &rsClipping);
		(*d3d9Device)->GetRenderState(D3DRENDERSTATE_LIGHTING, &rsLighting);
		(*d3d9Device)->GetRenderState(D3DRENDERSTATE_EXTENTS, &rsExtents);
		(*d3d9Device)->SetRenderState(D3DRENDERSTATE_CLIPPING, (dwVertexOp & D3DVOP_CLIP));
		(*d3d9Device)->SetRenderState(D3DRENDERSTATE_LIGHTING, (dwVertexOp & D3DVOP_LIGHT));
		(*d3d9Device)->SetRenderState(D3DRENDERSTATE_EXTENTS, (dwVertexOp & D3DVOP_EXTENTS));

		// Process vertices
		HRESULT hr = (*d3d9Device)->ProcessVertices(dwSrcIndex, dwDestIndex, dwCount, d3d9VertexBuffer, nullptr, dwFlags);

		// Reset render state
		(*d3d9Device)->SetRenderState(D3DRENDERSTATE_CLIPPING, rsClipping);
		(*d3d9Device)->SetRenderState(D3DRENDERSTATE_LIGHTING, rsLighting);
		(*d3d9Device)->SetRenderState(D3DRENDERSTATE_EXTENTS, rsExtents);

		if (FAILED(hr))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: 'ProcessVertices' call failed: " << (D3DERR)hr);
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
		LOG_LIMIT(100, __FUNCTION__ << " Error: Not Implemented");
		return DDERR_UNSUPPORTED;
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
	WrapperInterface = new m_IDirect3DVertexBuffer((LPDIRECT3DVERTEXBUFFER)ProxyInterface, this);
	WrapperInterface7 = new m_IDirect3DVertexBuffer7((LPDIRECT3DVERTEXBUFFER7)ProxyInterface, this);

	if (!Config.Dd7to9)
	{
		return;
	}

	if (ddrawParent)
	{
		ddrawParent->AddVertexBufferToVector(this);

		d3d9Device = ddrawParent->GetDirect3D9Device();
	}

	AddRef(DirectXVersion);
}

void m_IDirect3DVertexBufferX::ReleaseVertexBuffer()
{
	WrapperInterface->DeleteMe();
	WrapperInterface7->DeleteMe();

	ReleaseD9Buffers(false);

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
		return DDERR_GENERIC;
	}

	// Check d3d9 device
	if (CheckD3DDevice)
	{
		if (!ddrawParent->CheckD3D9Device() || !d3d9Device || !*d3d9Device)
		{
			LOG_LIMIT(100, FunctionName << " Error: d3d9 device not setup!");
			return DDERR_GENERIC;
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

	return DD_OK;
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

	HRESULT hr = DD_OK;
	if (!d3d9IndexBuffer || NewIndexSize > IndexBufferSize)
	{
		ReleaseD3D9IndexBuffer();
		hr = (*d3d9Device)->CreateIndexBuffer(NewIndexSize, ((VBDesc.dwCaps & D3DVBCAPS_DONOTCLIP) ? D3DUSAGE_DONOTCLIP : 0), D3DFMT_INDEX16, D3DPOOL_SYSTEMMEM, &d3d9IndexBuffer, nullptr);
	}

	if (FAILED(hr))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create index buffer: " << (D3DERR)hr);
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
	}
}

void m_IDirect3DVertexBufferX::ReleaseD9Buffers(bool BackupData)
{
	if (BackupData && VBDesc.dwFVF != D3DFVF_LVERTEX)
	{
		// ToDo: backup vertex buffer data
		//VertexData.resize(d3d9VBDesc.Size);
	}

	ReleaseD3D9VertexBuffer();
	ReleaseD3D9IndexBuffer();
}
