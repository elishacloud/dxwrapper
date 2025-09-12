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

// ******************************
// IUnknown functions
// ******************************

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

	DWORD DxVersion = (CheckWrapperType(riid) && Config.Dd7to9) ? GetGUIDVersion(riid) : DirectXVersion;

	if (riid == GetWrapperType(DxVersion) || riid == IID_IUnknown)
	{
		*ppvObj = GetWrapperInterfaceX(DxVersion);

		AddRef(DxVersion);

		return D3D_OK;
	}

	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, GetWrapperType(DirectXVersion));
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

	if (Config.Dd7to9)
	{
		ULONG ref;

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
// IDirect3DVertexBuffer v1 functions
// ******************************

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
		// DDLOCK_WAIT, DDLOCK_SURFACEMEMORYPTR and DDLOCK_WRITEONLY can be ignored safely

		DWORD Flags = (dwFlags & (DDLOCK_READONLY | DDLOCK_DISCARDCONTENTS | DDLOCK_NOSYSLOCK));

		// Handle emulated readonly
		if (IsVBEmulated && (Flags & D3DLOCK_READONLY))
		{
			LastLockAddr = nullptr;
			LastLockFlags = Flags;

			*lplpData = VertexData.data();

			if (lpdwSize)
			{
				*lpdwSize = VB.Size;
			}

			return D3D_OK;
		}

		// Lock vertex
		void* pData = nullptr;
		HRESULT hr = d3d9VertexBuffer->Lock(0, 0, &pData, Flags);
		if (FAILED(hr) && (Flags & D3DLOCK_NOSYSLOCK))
		{
			hr = d3d9VertexBuffer->Lock(0, 0, &pData, Flags & ~D3DLOCK_NOSYSLOCK);
		}

		if (FAILED(hr))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock vertex buffer: " << (D3DERR)hr);
			return hr;
		}

		LastLockAddr = pData;
		LastLockFlags = Flags;

		// Handle emulated vertex
		if (IsVBEmulated)
		{
			*lplpData = VertexData.data();

			if (lpdwSize)
			{
				*lpdwSize = VB.Size;
			}

			if (dwFlags & DDLOCK_DISCARDCONTENTS)
			{
				ZeroMemory(VertexData.data(), VB.Size);
			}
		}
		else
		{
			*lplpData = pData;

			if (lpdwSize)
			{
				*lpdwSize = VB.Size;
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

		// Handle emulated readonly
		if (IsVBEmulated && (LastLockFlags & D3DLOCK_READONLY))
		{
			LastLockAddr = nullptr;
			LastLockFlags = 0;

			return D3D_OK;
		}

		// Handle emulated vertex
		if (IsVBEmulated && LastLockAddr)
		{
			if (VB.Desc.dwFVF == D3DFVF_LVERTEX)
			{
				ConvertLVertex((DXLVERTEX9*)LastLockAddr, (DXLVERTEX7*)VertexData.data(), VB.Desc.dwNumVertices);
			}
			else
			{
				DWORD stride = GetVertexStride(VB.Desc.dwFVF);
				memcpy(LastLockAddr, VertexData.data(), VB.Desc.dwNumVertices * stride);

				if (Config.DdrawClampVertexZDepth && (VB.Desc.dwFVF & D3DFVF_XYZRHW))
				{
					ClampVertices(VertexData.data(), stride, VB.Desc.dwNumVertices);
				}
			}
		}

		HRESULT hr = d3d9VertexBuffer->Unlock();

		if (FAILED(hr))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to unlock vertex buffer: " << (D3DERR)hr);
			return hr;
		}
		
		LastLockAddr = nullptr;
		LastLockFlags = 0;

		return D3D_OK;
	}

	return ProxyInterface->Unlock();
}

HRESULT m_IDirect3DVertexBufferX::ProcessVertices(DWORD dwVertexOp, DWORD dwDestIndex, DWORD dwCount, LPDIRECT3DVERTEXBUFFER7 lpSrcBuffer, DWORD dwSrcIndex, LPDIRECT3DDEVICE7 lpD3DDevice, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Check if there are no vertices to process
		if (dwCount == 0)
		{
			return D3D_OK;
		}

		// Always include the D3DVOP_TRANSFORM flag in the dwVertexOp parameter. If you do not, the method fails, returning DDERR_INVALIDPARAMS.
		if (!lpSrcBuffer || !lpD3DDevice || !(dwVertexOp & D3DVOP_TRANSFORM))
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true, true)))
		{
			return DDERR_GENERIC;
		}

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

#ifdef ENABLE_PROFILING
		auto startTime = std::chrono::high_resolution_clock::now();
#endif

		// Get FVF
		DWORD dwSrcVertexTypeDesc = pSrcVertexBufferX->VB.Desc.dwFVF;

		void* pSrcVertices = nullptr;

		// Lock the source vertex buffer
		if (FAILED(pSrcVertexBufferX->Lock(D3DLOCK_READONLY, &pSrcVertices, 0)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock source vertex");
			return DDERR_GENERIC;
		}

		// Check the dwSrcIndex and dwCount to make sure they won't cause an overload
		DWORD SrcNumVertices = pSrcVertexBufferX->VB.Desc.dwNumVertices;
		if (dwSrcIndex > SrcNumVertices)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: source vertex index is too large: " <<
				SrcNumVertices << " -> " << dwSrcIndex);
			return D3DERR_INVALIDVERTEXTYPE;
		}
		dwCount = min(dwCount, SrcNumVertices - dwSrcIndex);

		HRESULT hr = ProcessVerticesUP(dwVertexOp, dwDestIndex, dwCount, pSrcVertices, dwSrcVertexTypeDesc, dwSrcIndex, lpD3DDevice, dwFlags);

		// Unlock the source vertex buffer
		if (pSrcVertices)
		{
			pSrcVertexBufferX->Unlock();
		}

#ifdef ENABLE_PROFILING
		Logging::Log() << __FUNCTION__ << " (" << this << ") hr = " << (D3DERR)hr << " Timing = " << Logging::GetTimeLapseInMS(startTime);
#endif

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

		if (VB.Desc.dwSize != sizeof(D3DVERTEXBUFFERDESC))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: invalid vertex buffer desc: " << VB.Desc);
			return DDERR_GENERIC;
		}

		lpVBDesc->dwSize = sizeof(D3DVERTEXBUFFERDESC);
		lpVBDesc->dwCaps = VB.Desc.dwCaps;
		lpVBDesc->dwFVF = VB.Desc.dwFVF;
		lpVBDesc->dwNumVertices = VB.Desc.dwNumVertices;

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

// ******************************
// IDirect3DVertexBuffer v7 functions
// ******************************

HRESULT m_IDirect3DVertexBufferX::ProcessVerticesStrided(DWORD dwVertexOp, DWORD dwDestIndex, DWORD dwCount, LPD3DDRAWPRIMITIVESTRIDEDDATA lpVertexArray, DWORD dwSrcIndex, LPDIRECT3DDEVICE7 lpD3DDevice, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Check if there are no vertices to process
		if (dwCount == 0)
		{
			return D3D_OK;
		}

		// Always include the D3DVOP_TRANSFORM flag in the dwVertexOp parameter. If you do not, the method fails, returning DDERR_INVALIDPARAMS.
		if (!lpVertexArray || !lpD3DDevice || !(dwVertexOp & D3DVOP_TRANSFORM))
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true, true)))
		{
			return DDERR_GENERIC;
		}

#ifdef ENABLE_PROFILING
		auto startTime = std::chrono::high_resolution_clock::now();
#endif

		// Setup vars
		DWORD dwVertexTypeDesc = VB.Desc.dwFVF;
		std::vector<BYTE, aligned_allocator<BYTE, 4>> VertexCache;

		// Process strided data
		if (!InterleaveStridedVertexData(VertexCache, lpVertexArray, dwSrcIndex, dwCount, dwVertexTypeDesc))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: invalid StridedVertexData!");
			return DDERR_INVALIDPARAMS;
		}

		HRESULT hr = ProcessVerticesUP(dwVertexOp, dwDestIndex, dwCount, VertexCache.data(), dwVertexTypeDesc, dwSrcIndex, lpD3DDevice, dwFlags);

#ifdef ENABLE_PROFILING
		Logging::Log() << __FUNCTION__ << " (" << this << ") hr = " << (D3DERR)hr << " Timing = " << Logging::GetTimeLapseInMS(startTime);
#endif

		return hr;
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

void m_IDirect3DVertexBufferX::InitInterface(DWORD DirectXVersion)
{
	if (ddrawParent)
	{
		ddrawParent->AddVertexBuffer(this);
	}

	if (D3DInterface)
	{
		D3DInterface->AddVertexBuffer(this);
	}

	if (Config.Dd7to9)
	{
		if (ddrawParent)
		{
			d3d9Device = ddrawParent->GetDirectD9Device();
		}

		AddRef(DirectXVersion);
	}
}

void m_IDirect3DVertexBufferX::ReleaseInterface()
{
	if (Config.Exiting)
	{
		return;
	}

	if (ddrawParent)
	{
		ddrawParent->ClearVertexBuffer(this);
	}

	if (D3DInterface)
	{
		D3DInterface->ClearVertexBuffer(this);
	}

	// Don't delete wrapper interface
	SaveInterfaceAddress(WrapperInterface);
	SaveInterfaceAddress(WrapperInterface7);

	ReleaseD9Buffer(false, false);
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

void* m_IDirect3DVertexBufferX::GetWrapperInterfaceX(DWORD DirectXVersion)
{
	switch (DirectXVersion)
	{
	case 0:
		if (WrapperInterface7) return WrapperInterface7;
		if (WrapperInterface) return WrapperInterface;
		break;
	case 1:
		return GetInterfaceAddress(WrapperInterface, (LPDIRECT3DVERTEXBUFFER)ProxyInterface, this);
	case 7:
		return GetInterfaceAddress(WrapperInterface7, (LPDIRECT3DVERTEXBUFFER7)ProxyInterface, this);
	}
	LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
	return nullptr;
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

	d3d9VBDesc.FVF = (VB.Desc.dwFVF == D3DFVF_LVERTEX) ? D3DFVF_LVERTEX9 : VB.Desc.dwFVF;
	d3d9VBDesc.Size = GetVertexStride(d3d9VBDesc.FVF) * VB.Desc.dwNumVertices;
	d3d9VBDesc.Usage = D3DUSAGE_DYNAMIC |
		((VB.Desc.dwCaps & D3DVBCAPS_WRITEONLY) ? D3DUSAGE_WRITEONLY : 0) |
		((VB.Desc.dwCaps & D3DVBCAPS_DONOTCLIP) ? D3DUSAGE_DONOTCLIP : 0);
	d3d9VBDesc.Pool = (VB.Desc.dwCaps & D3DVBCAPS_SYSTEMMEMORY) ? D3DPOOL_SYSTEMMEM : D3DPOOL_DEFAULT;

	HRESULT hr = (*d3d9Device)->CreateVertexBuffer(d3d9VBDesc.Size, d3d9VBDesc.Usage, d3d9VBDesc.FVF, d3d9VBDesc.Pool, &d3d9VertexBuffer, nullptr);
	if (FAILED(hr))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create vertex buffer: " << (D3DERR)hr <<
			" Length: " << d3d9VBDesc.Size << " Usage: " << Logging::hex(d3d9VBDesc.Usage) << " FVF: " << Logging::hex(d3d9VBDesc.FVF) << " Pool: " << Logging::hex(d3d9VBDesc.Pool) <<
			VB.Desc);
		return DDERR_GENERIC;
	}

	VB.Size = GetVertexStride(VB.Desc.dwFVF) * VB.Desc.dwNumVertices;

	IsVBEmulated = (VB.Desc.dwFVF == D3DFVF_LVERTEX) || (Config.DdrawClampVertexZDepth && (VB.Desc.dwFVF && D3DFVF_XYZRHW));

	if (IsVBEmulated)
	{
		// ToDo: restore vertex buffer data
		if (VertexData.size() < VB.Size)
		{
			VertexData.resize(VB.Size);
		}
	}

	LastLockAddr = nullptr;
	LastLockFlags = 0;

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

void m_IDirect3DVertexBufferX::ReleaseD9Buffer(bool BackupData, bool ResetBuffer)
{
	if (BackupData && VB.Desc.dwFVF != D3DFVF_LVERTEX)
	{
		// ToDo: backup vertex buffer data
	}

	if (!ResetBuffer || d3d9VBDesc.Pool == D3DPOOL_DEFAULT)
	{
		ReleaseD3D9VertexBuffer();
	}
}

bool m_IDirect3DVertexBufferX::InterleaveStridedVertexData(std::vector<BYTE, aligned_allocator<BYTE, 4>>& outputBuffer, const D3DDRAWPRIMITIVESTRIDEDDATA* sd, const DWORD dwVertexStart, const DWORD dwNumVertices, const DWORD dwVertexTypeDesc)
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

HRESULT m_IDirect3DVertexBufferX::ProcessVerticesUP(DWORD dwVertexOp, DWORD dwDestIndex, DWORD dwCount, LPVOID lpSrcBuffer, DWORD dwSrcVertexTypeDesc, DWORD dwSrcIndex, LPDIRECT3DDEVICE7 lpD3DDevice, DWORD dwFlags)
{
	if (dwCount == 0)
	{
		return D3D_OK;	// No vertices to process
	}

	if (!lpSrcBuffer)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: missing source buffer data!");
		return DDERR_INVALIDPARAMS;
	}

	if (!lpD3DDevice)
	{
		return DDERR_INVALIDPARAMS;
	}

	m_IDirect3DDeviceX* pDirect3DDeviceX = nullptr;
	lpD3DDevice->QueryInterface(IID_GetInterfaceX, (LPVOID*)&pDirect3DDeviceX);
	if (!pDirect3DDeviceX)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: could not get Direct3DDeviceX interface!");
		return DDERR_GENERIC;
	}

	// Check the dwDestIndex and dwCount to make sure they won't cause an overload
	DWORD DestNumVertices = VB.Desc.dwNumVertices;
	if (dwDestIndex > DestNumVertices)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: destination vertex index is too large: " << DestNumVertices << " -> " << dwDestIndex);
		return D3DERR_INVALIDVERTEXTYPE;
	}
	dwCount = min(dwCount, DestNumVertices - dwDestIndex);

	// Get and verify FVF
	DWORD SrcFVF = dwSrcVertexTypeDesc;
	DWORD DestFVF = VB.Desc.dwFVF;

	UINT SrcStride = GetVertexStride(SrcFVF);
	UINT DestStride = GetVertexStride(DestFVF);

	DWORD SrcPosFVF = (SrcFVF & D3DFVF_POSITION_MASK);
	DWORD DestPosFVF = (DestFVF & D3DFVF_POSITION_MASK);

	// Processing the vertices in a vertex buffer applies the current transformation matrices for the device, and can optionally apply vertex operations
	// such as lighting, generating clip flags, and updating extents
	if (SrcPosFVF == D3DFVF_XYZRHW || DestPosFVF != D3DFVF_XYZRHW)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid FVF conversion: Cannot transform from D3DFVF_XYZRHW or to non-D3DFVF_XYZRHW format: " << Logging::hex(SrcFVF) << " -> " << Logging::hex(DestFVF));
		return D3DERR_INVALIDVERTEXTYPE;
	}

	bool DoNotCopyData = (dwFlags & D3DPV_DONOTCOPYDATA);
	bool IsDiffuseFVF = (DestFVF & D3DFVF_DIFFUSE);
	bool IsSpecularFVF = (DestFVF & D3DFVF_SPECULAR);
	bool IsLight = (IsDiffuseFVF || IsSpecularFVF) && (dwVertexOp & D3DVOP_LIGHT) && ((DestFVF & D3DFVF_NORMAL) || (SrcFVF & D3DFVF_NORMAL));

	// Handle dwVertexOp
	// D3DVOP_TRANSFORM is inherently handled by ProcessVertices() as it performs vertex transformations based on the current world, view, and projection matrices.
	if ((dwVertexOp & D3DVOP_LIGHT) && !IsLight)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DVOP_LIGHT' is specified but verticies don't support it: " << Logging::hex(SrcFVF) << " -> " << Logging::hex(DestFVF));
	}
	if (dwVertexOp & D3DVOP_CLIP)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DVOP_CLIP' not handled!");
	}
	if (dwVertexOp & D3DVOP_EXTENTS)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DVOP_EXTENTS' not handled!");
	}

	DWORD SrcPosSize = GetVertexPositionStride(SrcFVF);
	DWORD DestPosSize = GetVertexPositionStride(DestFVF);

	DWORD NormalSrcOffset = 0;
	DWORD NormalDestOffset = 0;
	DWORD DiffuseOffset = 0;
	DWORD SpecularOffset = 0;

	if (IsLight)
	{
		DWORD offset = DestPosSize;
		if (DestFVF & D3DFVF_RESERVED1)
		{
			offset += sizeof(DWORD);
		}
		if (DestFVF & D3DFVF_NORMAL)
		{
			NormalDestOffset = offset;
			offset += 3 * sizeof(float);
		}
		if (DestFVF & D3DFVF_DIFFUSE)
		{
			DiffuseOffset = offset;
			offset += sizeof(DWORD);
		}
		if (DestFVF & D3DFVF_SPECULAR)
		{
			SpecularOffset = offset;
			offset += sizeof(DWORD);
		}
		if (SrcFVF & D3DFVF_NORMAL)
		{
			NormalDestOffset = SrcPosSize;
		}
	}

	D3DMATRIX matWorld, matView, matProj;
	if (FAILED(pDirect3DDeviceX->GetTransform(D3DTRANSFORMSTATE_WORLD, &matWorld)) ||
		FAILED(pDirect3DDeviceX->GetTransform(D3DTRANSFORMSTATE_VIEW, &matView)) ||
		FAILED(pDirect3DDeviceX->GetTransform(D3DTRANSFORMSTATE_PROJECTION, &matProj)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Failed to get transform matrices");
		return DDERR_GENERIC;
	}

	D3DMATRIX matWorldView = {}, matWorldViewProj = {};
	D3DXMatrixMultiply(&matWorldView, &matWorld, &matView);
	D3DXMatrixMultiply(&matWorldViewProj, &matWorldView, &matProj);

	// Get materal for specular
	bool UseMaterial = false;
	D3DMATERIAL7 mat = {};
	if (IsLight)
	{
		if (SUCCEEDED(pDirect3DDeviceX->GetMaterial(&mat)))
		{
			UseMaterial = true;
		}
	}

	// Cache light data once
	std::vector<DXLIGHT7> cachedLights;
	if (IsLight)
	{
		pDirect3DDeviceX->GetEnabledLightList(cachedLights);

		if (cachedLights.empty())
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: no attached lights found!");
		}
	}

	void* pSrcVertices = lpSrcBuffer;
	void* pDestVertices = nullptr;

	// Lock the destination vertex buffer
	if (FAILED(Lock(0, &pDestVertices, 0)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock destination vertex");
		return DDERR_GENERIC;
	}

	BYTE* pSrcVertex = (BYTE*)pSrcVertices + (dwSrcIndex * SrcStride);
	BYTE* pDestVertex = (BYTE*)pDestVertices + (dwDestIndex * DestStride);

	// Copy vertex data
	bool SimpleVertexCopy = false;
	bool ComplexConvertVertex = false;
	if (!DoNotCopyData)
	{
		if (SrcFVF == DestFVF || ((SrcFVF & ~(D3DFVF_XYZ | D3DFVF_RESERVED1)) == (DestFVF & ~D3DFVF_XYZRHW) && SrcStride == DestStride))
		{
			memcpy(pDestVertex, pSrcVertex, dwCount * DestStride);
		}
		else if ((SrcFVF & ~D3DFVF_POSITION_MASK) == (DestFVF & ~D3DFVF_XYZRHW))
		{
			SimpleVertexCopy = true;
		}
		else
		{
			ComplexConvertVertex = true;
			LOG_LIMIT(100, __FUNCTION__ << " Warning: manually converting vertices may be slower: " << Logging::hex(SrcFVF) << " -> " << Logging::hex(DestFVF));
		}
	}

	for (UINT i = 0; i < dwCount; ++i)
	{
		// Convert and copy vertex data
		if (SimpleVertexCopy)
		{
			// Copy vertex excluding position
			memcpy(pDestVertex + DestPosSize, pSrcVertex + SrcPosSize, DestStride - DestPosSize);
		}
		else if (ComplexConvertVertex)
		{
			// Converts full vertex data
			ConvertVertex(pDestVertex, DestFVF, pSrcVertex, SrcFVF);
		}

		// Apply the transformation to the position
		D3DXVECTOR3& src = *reinterpret_cast<D3DXVECTOR3*>(pSrcVertex);
		D3DXVECTOR4& dst = *reinterpret_cast<D3DXVECTOR4*>(pDestVertex);

		const float* m = &matWorldViewProj._11;
		float x = src.x;
		float y = src.y;
		float z = src.z;
		float w = 1.0f;

		// Matrix multiply: row-major
		float tx = x * m[0] + y * m[4] + z * m[8] + w * m[12];
		float ty = x * m[1] + y * m[5] + z * m[9] + w * m[13];
		float tz = x * m[2] + y * m[6] + z * m[10] + w * m[14];
		float tw = x * m[3] + y * m[7] + z * m[11] + w * m[15];

		// Avoid divide by zero
		tw = (tw == 0.0f) ? FLT_EPSILON : tw;

		dst.x = tx / tw;
		dst.y = ty / tw;
		dst.z = Config.DdrawClampVertexZDepth ? min(tz / tw, 1.0f) : tz / tw;
		dst.w = 1.0f / tw;

		// Perform lighting if required
		if (IsLight)
		{
			D3DXVECTOR3 Pos = *reinterpret_cast<const D3DXVECTOR3*>(pSrcVertex);
			D3DXVECTOR3 Normal = NormalSrcOffset ?
				*reinterpret_cast<const D3DXVECTOR3*>(pSrcVertex + NormalSrcOffset) :
				*reinterpret_cast<const D3DXVECTOR3*>(pDestVertex + NormalDestOffset);

			D3DCOLOR Diffuse = 0, Specular = 0;
			ComputeLightColor(Diffuse, Specular, Pos, Normal, cachedLights, matWorldView, matWorld, matView, mat, UseMaterial);

			if (IsDiffuseFVF)
			{
				*(D3DCOLOR*)(pDestVertex + DiffuseOffset) = Diffuse;
			}
			if (IsSpecularFVF)
			{
				*(D3DCOLOR*)(pDestVertex + SpecularOffset) = Specular;
			}
		}

		// Move to the next vertex
		pSrcVertex += SrcStride;
		pDestVertex += DestStride;
	}

	// Unlock the destination vertex buffer
	if (pDestVertices)
	{
		Unlock();
	}

	return D3D_OK;
}

template HRESULT m_IDirect3DVertexBufferX::TransformVertexUP<XYZ>(m_IDirect3DDeviceX* , XYZ*, D3DTLVERTEX*, D3DHVERTEX*, const DWORD, D3DRECT&, bool, bool);
template HRESULT m_IDirect3DVertexBufferX::TransformVertexUP<D3DLVERTEX>(m_IDirect3DDeviceX* , D3DLVERTEX*, D3DTLVERTEX*, D3DHVERTEX*, const DWORD, D3DRECT&, bool, bool);
template <typename T>
HRESULT m_IDirect3DVertexBufferX::TransformVertexUP(m_IDirect3DDeviceX* pDirect3DDeviceX, T* srcVertex, D3DTLVERTEX* destVertex, D3DHVERTEX* pHOut, const DWORD dwCount, D3DRECT& drExtent, bool bLight, bool bUpdateExtents)
{
	D3DMATRIX matWorld, matView, matProj;
	if (FAILED(pDirect3DDeviceX->GetTransform(D3DTRANSFORMSTATE_WORLD, &matWorld)) ||
		FAILED(pDirect3DDeviceX->GetTransform(D3DTRANSFORMSTATE_VIEW, &matView)) ||
		FAILED(pDirect3DDeviceX->GetTransform(D3DTRANSFORMSTATE_PROJECTION, &matProj)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Failed to get transform matrices");
		return DDERR_GENERIC;
	}

	D3DMATRIX matWorldView = {}, matWorldViewProj = {};
	D3DXMatrixMultiply(&matWorldView, &matWorld, &matView);
	D3DXMatrixMultiply(&matWorldViewProj, &matWorldView, &matProj);

	// Lighting not supported with current vertex types
	if (bLight)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Warning: Lighting requested but no normals are provided.  Cannot compute lighting!");
	}

	LONG minX = LONG_MAX;
	LONG minY = LONG_MAX;
	LONG maxX = LONG_MIN;
	LONG maxY = LONG_MIN;

	for (DWORD i = 0; i < dwCount; ++i)
	{
		T& src = srcVertex[i];
		D3DTLVERTEX& dst = destVertex[i];

		const float* m = &matWorldViewProj._11;
		float x = src.x;
		float y = src.y;
		float z = src.z;
		float w = 1.0f;

		// Matrix multiply: row-major
		float tx = x * m[0] + y * m[4] + z * m[8] + w * m[12];
		float ty = x * m[1] + y * m[5] + z * m[9] + w * m[13];
		float tz = x * m[2] + y * m[6] + z * m[10] + w * m[14];
		float tw = x * m[3] + y * m[7] + z * m[11] + w * m[15];

		// Avoid divide by zero
		tw = (tw == 0.0f) ? FLT_EPSILON : tw;

		dst.sx = tx / tw;
		dst.sy = ty / tw;
		dst.sz = Config.DdrawClampVertexZDepth ? min(tz / tw, 1.0f) : tz / tw;
		dst.rhw = 1.0f / tw;

		// Default values: set for XYZ or copy for detailed vertex
		if constexpr (std::is_same_v<T, XYZ>)
		{
			dst.color = 0xFFFFFFFF;	// Default to white
			dst.specular = 0;
			dst.tu = 0.0f;
			dst.tv = 0.0f;
		}
		else if constexpr (std::is_same_v<T, D3DLVERTEX>)
		{
			dst.color = src.color;
			dst.specular = src.specular;
			dst.tu = src.tu;
			dst.tv = src.tv;
		}
		else
		{
			static_assert(false);
		}

		// Fill homogeneous out if requested
		if (pHOut)
		{
			D3DHVERTEX& hdst = pHOut[i];
			// Store pre-divide homogeneous coords
			hdst.hx = tx;
			hdst.hy = ty;
			hdst.hz = tz;
			hdst.dwFlags = 0; // Clip flags not computed here (TransformVertices only sets them if clipping performed upstream)
		}

		if (bUpdateExtents)
		{
			// floor/ceil convert to integer extents
			minX = min(minX, static_cast<LONG>(floor(dst.sx)));
			minY = min(minY, static_cast<LONG>(floor(dst.sy)));
			maxX = max(maxX, static_cast<LONG>(ceil(dst.sx)));
			maxY = max(maxY, static_cast<LONG>(ceil(dst.sy)));
		}
	}

	if (bUpdateExtents)
	{
		if (minX != LONG_MAX)
		{
			drExtent.x1 = minX;
			drExtent.y1 = minY;
			drExtent.x2 = maxX;
			drExtent.y2 = maxY;
		}
		else
		{
			// no vertices -> empty extent
			drExtent.x1 = drExtent.y1 = drExtent.x2 = drExtent.y2 = 0;
		}
	}

	return D3D_OK;
}

void m_IDirect3DVertexBufferX::ComputeLightColor(D3DCOLOR& outColor, D3DCOLOR& outSpecular, const D3DXVECTOR3& Position, const D3DXVECTOR3& Normal, const std::vector<DXLIGHT7>& cachedLights, const D3DXMATRIX& matWorldView, const D3DMATRIX& matWorld, const D3DMATRIX& matView, const D3DMATERIAL7& mat, bool UseMaterial)
{
	// Transform position using full world-view matrix (this is fine)
	D3DXVECTOR3 worldPos;
	D3DXVec3TransformCoord(&worldPos, &Position, &matWorldView);

	// Now transform the normal using only the world matrix (like DX2 behavior)
	D3DXMATRIX matWorldRotOnly = matWorld;
	matWorldRotOnly._41 = 0.0f;
	matWorldRotOnly._42 = 0.0f;
	matWorldRotOnly._43 = 0.0f;

	// Clear perspective components
	matWorldRotOnly._14 = 0.0f;
	matWorldRotOnly._24 = 0.0f;
	matWorldRotOnly._34 = 0.0f;
	matWorldRotOnly._44 = 1.0f;

	D3DXVECTOR3 worldNormal;
	D3DXVec3TransformNormal(&worldNormal, &Normal, &matWorldRotOnly);
	D3DXVec3Normalize(&worldNormal, &worldNormal);

	D3DCOLORVALUE diffuse = { 0.0f, 0.0f, 0.0f, 0.0f };
	D3DCOLORVALUE specular = { 0.0f, 0.0f, 0.0f, 0.0f };

	for (const auto& light : cachedLights)
	{
		const D3DXVECTOR3& lightPos = *reinterpret_cast<const D3DXVECTOR3*>(&light.dvPosition);
		const D3DXVECTOR3& lightDir = *reinterpret_cast<const D3DXVECTOR3*>(&light.dvDirection);

		D3DXVECTOR3 toLight;
		float dist = 1.0f;
		float attenuation = 1.0f;
		float denom = 1.0f;

		switch (light.dltType)
		{
		case D3DLIGHT_DIRECTIONAL:
			toLight = -lightDir;
			D3DXVec3Normalize(&toLight, &toLight);
			break;

		case D3DLIGHT_POINT:
		case D3DLIGHT_SPOT:
			toLight = lightPos - worldPos;
			dist = D3DXVec3Length(&toLight);
			if (dist == 0.0f) continue;
			toLight /= dist;

			denom = light.dvAttenuation0 +
				light.dvAttenuation1 * dist +
				light.dvAttenuation2 * dist * dist;

			if (denom <= 0.0f) continue;  // Skip bad light

			attenuation = 1.0f / denom;

			// Handle range cutoff
			if (light.dvRange > 0.0f && dist > light.dvRange)
			{
				continue;
			}

			// Spotlight falloff
			if (light.dltType == D3DLIGHT_SPOT)
			{
				D3DXVECTOR3 toLightNeg = -toLight;
				float spotCos = D3DXVec3Dot(&toLightNeg, &lightDir);
				spotCos = max(-1.0f, min(1.0f, spotCos));	// Clamp spotCos to [-1.0, 1.0]
				float cosPhi = cosf(light.dvPhi * 0.5f);
				if (spotCos < cosPhi) continue;

				float cosTheta = cosf(light.dvTheta * 0.5f);
				if (spotCos >= cosTheta)
				{
					// full light
				}
				else
				{
					float falloff = powf((spotCos - cosPhi) / (cosTheta - cosPhi), max(light.dvFalloff, 1.0f));  // Clamp falloff to minimum 1.0
					attenuation *= falloff;
				}
			}
			break;

		default:
			continue; // unsupported light type
		}

		float NdotL = max(0.0f, D3DXVec3Dot(&worldNormal, &toLight));
		if (NdotL <= 0.0f) continue;

		// Clamp attenuation to [0,1]
		attenuation = min(max(attenuation, 0.0f), 1.0f);

		// Diffuse lighting
		diffuse.r += light.dcvDiffuse.r * NdotL * attenuation;
		diffuse.g += light.dcvDiffuse.g * NdotL * attenuation;
		diffuse.b += light.dcvDiffuse.b * NdotL * attenuation;

		// Specular lighting
		if ((light.dwLightVersion != 7 && !(light.dwFlags & D3DLIGHT_NO_SPECULAR) && UseMaterial) ||
			(light.dwLightVersion == 7 && (light.dcvSpecular.r != 0.0f || light.dcvSpecular.g != 0.0f || light.dcvSpecular.b != 0.0f)))
		{
			if (UseMaterial)
			{
				// Compute view direction
				D3DXVECTOR3 viewPos;
				D3DXVec3TransformCoord(&viewPos, &Position, &matView);
				D3DXVECTOR3 viewDir = -viewPos;
				D3DXVec3Normalize(&viewDir, &viewDir);

				D3DXVec3Normalize(&toLight, &toLight);
				D3DXVECTOR3 reflectDir = worldNormal * 2.0f * NdotL - toLight;
				D3DXVec3Normalize(&reflectDir, &reflectDir);

				float RdotV = max(0.0f, D3DXVec3Dot(&reflectDir, &viewDir));
				float shininess = max(1.0f, mat.power);
				float spec = powf(RdotV, shininess) * attenuation;

				// Light versions older than 7 don't have specular
				if (light.dwLightVersion != 7)
				{
					specular.r += mat.specular.r * spec;
					specular.g += mat.specular.g * spec;
					specular.b += mat.specular.b * spec;
				}
				else
				{
					specular.r += mat.specular.r * light.dcvSpecular.r * spec;
					specular.g += mat.specular.g * light.dcvSpecular.g * spec;
					specular.b += mat.specular.b * light.dcvSpecular.b * spec;
				}
			}
			else
			{
				// No material; use light's specular with NdotL
				specular.r += light.dcvSpecular.r * NdotL * attenuation;
				specular.g += light.dcvSpecular.g * NdotL * attenuation;
				specular.b += light.dcvSpecular.b * NdotL * attenuation;
			}
		}
	}

	// Add material ambient color
	if (UseMaterial)
	{
		diffuse.r += mat.diffuse.r * mat.ambient.r;
		diffuse.g += mat.diffuse.g * mat.ambient.g;
		diffuse.b += mat.diffuse.b * mat.ambient.b;
	}

	float alpha = UseMaterial ? mat.diffuse.a : 1.0f;

	// Clamp and convert to DWORD color
	outColor = D3DCOLOR_COLORVALUE(
		min(max(diffuse.r, 0.0f), 1.0f),
		min(max(diffuse.g, 0.0f), 1.0f),
		min(max(diffuse.b, 0.0f), 1.0f),
		alpha);

	// Clamp and convert to DWORD specular
	outSpecular = D3DCOLOR_COLORVALUE(
		min(max(specular.r, 0.0f), 1.0f),
		min(max(specular.g, 0.0f), 1.0f),
		min(max(specular.b, 0.0f), 1.0f),
		1.0f);
}
