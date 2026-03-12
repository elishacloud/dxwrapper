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

	DWORD DxVersion = (Config.Dd7to9 && CheckWrapperType(riid)) ? GetGUIDVersion(riid) : DirectXVersion;

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

		if (LastLock.IsLocked)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: locking vertex buffer when buffer is already locked!");
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, false, true)))
		{
			return DDERR_GENERIC;
		}

		// Non-implemented dwFlags:
		// DDLOCK_WAIT and DDLOCK_WRITEONLY can be ignored safely

		DWORD Flags = (dwFlags & (DDLOCK_READONLY | DDLOCK_DISCARDCONTENTS | DDLOCK_NOSYSLOCK | DDLOCK_NOOVERWRITE)) |
			(IsVBEmulated || (Config.DdrawVertexLockDiscard && !(dwFlags & DDLOCK_READONLY)) ? D3DLOCK_DISCARD : NULL) |
			(Config.DdrawNoDrawBufferSysLock ? D3DLOCK_NOSYSLOCK : NULL);

		// Handle emulated readonly
		if (IsVBEmulated && (Flags & D3DLOCK_READONLY))
		{
			LastLock.IsLocked = true;
			LastLock.Addr = nullptr;
			LastLock.Flags = Flags;

			*lplpData = VertexData.data();

			if (lpdwSize)
			{
				*lpdwSize = VB.Size;
			}

			return D3D_OK;
		}

#ifdef ENABLE_PROFILING
		auto startTime = std::chrono::high_resolution_clock::now();
#endif

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

		LastLock.IsLocked = true;
		LastLock.Addr = pData;
		LastLock.Flags = Flags;

		// Handle emulated vertex
		if (IsVBEmulated)
		{
			*lplpData = VertexData.data();

			if (lpdwSize)
			{
				*lpdwSize = VB.Size;
			}

			//if (dwFlags & DDLOCK_DISCARDCONTENTS)
			//{
			//	ZeroMemory(VertexData.data(), VB.Size);
			//}
		}
		else
		{
			*lplpData = pData;

			if (lpdwSize)
			{
				*lpdwSize = VB.Size;
			}
		}

#ifdef ENABLE_PROFILING
		Logging::Log() << __FUNCTION__ << " (" << this << ") hr = " << (D3DERR)hr << " Timing = " << Logging::GetTimeLapseInMS(startTime);
#endif

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
		if (IsVBEmulated && (LastLock.Flags & D3DLOCK_READONLY))
		{
			LastLock.IsLocked = false;
			LastLock.Addr = nullptr;
			LastLock.Flags = 0;

			return D3D_OK;
		}

		// Handle emulated vertex
		if (IsVBEmulated && LastLock.Addr)
		{
			if (VB.Desc.dwFVF == D3DFVF_LVERTEX)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Warning: converting vertex buffer, may cause slowdowns!");

				ConvertLVertex((DXLVERTEX9*)LastLock.Addr, (DXLVERTEX7*)VertexData.data(), VB.Desc.dwNumVertices);
			}
			else
			{
				DWORD stride = GetVertexStride(VB.Desc.dwFVF);
				memcpy(LastLock.Addr, VertexData.data(), VB.Desc.dwNumVertices * stride);

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
		
		LastLock.IsLocked = false;
		LastLock.Addr = nullptr;
		LastLock.Flags = 0;

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

		// Check the dwSrcIndex and dwCount to make sure they won't cause an overload
		DWORD SrcNumVertices = pSrcVertexBufferX->VB.Desc.dwNumVertices;
		if (dwSrcIndex > SrcNumVertices)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: source vertex index is too large: " << SrcNumVertices << " -> " << dwSrcIndex);
			return DDERR_INVALIDPARAMS;
		}
		dwCount = min(dwCount, SrcNumVertices - dwSrcIndex);

		void* pSrcVertices = nullptr;

		// Lock the source vertex buffer
		if (FAILED(pSrcVertexBufferX->Lock(D3DLOCK_READONLY, &pSrcVertices, 0)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock source vertex");
			return DDERR_GENERIC;
		}

		HRESULT hr = ProcessVerticesUP(dwVertexOp, dwDestIndex, dwCount, pSrcVertices, dwSrcVertexTypeDesc, dwSrcIndex, lpD3DDevice, dwFlags);

		// Unlock the source vertex buffer
		pSrcVertexBufferX->Unlock();

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

	IsVBEmulated = (VB.Desc.dwFVF == D3DFVF_LVERTEX) || (Config.DdrawClampVertexZDepth && (VB.Desc.dwFVF && D3DFVF_XYZRHW));

	d3d9VBDesc.FVF = (VB.Desc.dwFVF == D3DFVF_LVERTEX) ? D3DFVF_LVERTEX9 : VB.Desc.dwFVF;
	d3d9VBDesc.Size = GetVertexStride(d3d9VBDesc.FVF) * VB.Desc.dwNumVertices;
	d3d9VBDesc.Usage = D3DUSAGE_DYNAMIC |
		((VB.Desc.dwCaps & D3DVBCAPS_WRITEONLY) || IsVBEmulated ? D3DUSAGE_WRITEONLY : 0) |
		((VB.Desc.dwCaps & D3DVBCAPS_DONOTCLIP) ? D3DUSAGE_DONOTCLIP : 0);
	d3d9VBDesc.Pool = (VB.Desc.dwCaps & D3DVBCAPS_SYSTEMMEMORY) ? D3DPOOL_SYSTEMMEM : D3DPOOL_DEFAULT;

	HRESULT hr = (*d3d9Device)->CreateVertexBuffer(d3d9VBDesc.Size, d3d9VBDesc.Usage, d3d9VBDesc.FVF, d3d9VBDesc.Pool, &d3d9VertexBuffer, nullptr);
	if (FAILED(hr))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create vertex buffer: " << (D3DERR)hr <<
			" Size: " << d3d9VBDesc.Size << " Usage: " << Logging::hex(d3d9VBDesc.Usage) << " FVF: " << Logging::hex(d3d9VBDesc.FVF) <<
			" Pool: " << Logging::hex(d3d9VBDesc.Pool) << VB.Desc);
		return DDERR_GENERIC;
	}

	VB.Size = GetVertexStride(VB.Desc.dwFVF) * VB.Desc.dwNumVertices;

	if (IsVBEmulated)
	{
		// ToDo: restore vertex buffer data
		if (VertexData.size() < VB.Size)
		{
			VertexData.resize(VB.Size);
		}
	}

	LastLock.IsLocked = false;
	LastLock.Addr = nullptr;
	LastLock.Flags = 0;

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

	if (!lpSrcBuffer || !lpD3DDevice)
	{
		return DDERR_INVALIDPARAMS;
	}

	// Get our wrapper device
	m_IDirect3DDeviceX* pDirect3DDeviceX = nullptr;
	lpD3DDevice->QueryInterface(IID_GetInterfaceX, (LPVOID*)&pDirect3DDeviceX);
	if (!pDirect3DDeviceX)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: could not get Direct3DDeviceX interface!");
		return DDERR_GENERIC;
	}

	// Validate destination range
	DWORD DestNumVertices = VB.Desc.dwNumVertices;
	if (dwDestIndex >= DestNumVertices)
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
	DWORD SrcPosFVF = SrcFVF & D3DFVF_POSITION_MASK;
	DWORD DestPosFVF = DestFVF & D3DFVF_POSITION_MASK;

	// Cannot transform RHW vertices or convert to non-RHW TL format
	if (SrcPosFVF == D3DFVF_XYZRHW || DestPosFVF != D3DFVF_XYZRHW)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid FVF conversion: Cannot transform from D3DFVF_XYZRHW or to non-D3DFVF_XYZRHW format: " << Logging::hex(SrcFVF) << " -> " << Logging::hex(DestFVF));
		return D3DERR_INVALIDVERTEXTYPE;
	}

	// Handle dwVertexOp
	if (dwVertexOp & D3DVOP_CLIP)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DVOP_CLIP' not handled!");
	}
	if (dwVertexOp & D3DVOP_EXTENTS)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DVOP_EXTENTS' not handled!");
	}
	// D3DVOP_TRANSFORM is inherently handled by ProcessVertices() as it performs vertex transformations based on the current world, view, and projection matrices.
	if (!(dwVertexOp & D3DVOP_TRANSFORM))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Warning: D3DVOP_TRANSFORM not set, forcing transform");
		dwVertexOp |= D3DVOP_TRANSFORM;
	}

	bool DoNotCopyData = (dwFlags & D3DPV_DONOTCOPYDATA) != 0;
	bool bLighting = (dwVertexOp & D3DVOP_LIGHT);

	DWORD PosSizeSrc = GetVertexPositionStride(SrcFVF);
	DWORD PosSizeDest = GetVertexPositionStride(DestFVF);

	DWORD NormalSrcOffset = 0;
	DWORD DiffuseSrcOffset = 0;
	DWORD SpecularSrcOffset = 0;
	DWORD DiffuseDestOffset = 0;
	DWORD SpecularDestOffset = 0;

	// Only compute offsets if lighting is enabled
	if (bLighting)
	{
		// Get dest offsets
		DWORD offset = PosSizeDest;
		if (DestFVF & D3DFVF_NORMAL)
		{
			offset += sizeof(float) * 3;
		}
		if (DestFVF & D3DFVF_DIFFUSE)
		{
			DiffuseDestOffset = offset;
			offset += sizeof(DWORD);
		}
		if (DestFVF & D3DFVF_SPECULAR)
		{
			SpecularDestOffset = offset;
			offset += sizeof(DWORD);
		}

		// Get src offsets
		offset = PosSizeSrc;
		if (SrcFVF & D3DFVF_NORMAL)
		{
			NormalSrcOffset = offset;
			offset += sizeof(float) * 3;
		}
		if (SrcFVF & D3DFVF_DIFFUSE)
		{
			DiffuseSrcOffset = offset;
			offset += sizeof(DWORD);
		}
		if (SrcFVF & D3DFVF_SPECULAR)
		{
			SpecularSrcOffset = offset;
			offset += sizeof(DWORD);
		}
	}

	// Check for lighiting, must have source normals and dest diffuse or specular
	if (bLighting && (!NormalSrcOffset || (!(DestFVF & D3DFVF_DIFFUSE) && !(DestFVF & D3DFVF_SPECULAR))))
	{
		bLighting = false;
		LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DVOP_LIGHT' is specified but verticies don't support it: " << Logging::hex(SrcFVF) << " -> " << Logging::hex(DestFVF));
	}

	// Get transformation matrices
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

	// Cache specular, ambient, material and lights if needed
	bool UseSpecular = false;
	D3DCOLOR ambient = 0;
	LPD3DMATERIAL7 lpMaterial = nullptr;
	D3DMATERIAL7 Material = {};
	std::vector<DXLIGHT7> cachedLights;

	if (bLighting)
	{
		if (DWORD rsSpecular = 0; SUCCEEDED(pDirect3DDeviceX->GetRenderState(D3DRENDERSTATE_SPECULARENABLE, &rsSpecular)))
		{
			UseSpecular = rsSpecular != FALSE;
		}

		pDirect3DDeviceX->GetRenderState(D3DRENDERSTATE_AMBIENT, &ambient);

		if (SUCCEEDED(pDirect3DDeviceX->GetMaterial(&Material)))
		{
			lpMaterial = &Material;
		}

		pDirect3DDeviceX->GetEnabledLightList(cachedLights);

		if (cachedLights.empty())
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: no attached lights found!");
		}
	}

	// Lock destination buffer
	void* pDestVertices = nullptr;
	if (FAILED(Lock(0, &pDestVertices, 0)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock destination vertex");
		return DDERR_GENERIC;
	}

	BYTE* pSrcVertex = (BYTE*)lpSrcBuffer + (dwSrcIndex * SrcStride);
	BYTE* pDestVertex = (BYTE*)pDestVertices + (dwDestIndex * DestStride);

	const bool SimpleCopy = (SrcFVF & ~D3DFVF_POSITION_MASK) == (DestFVF & ~D3DFVF_POSITION_MASK) && (SrcStride - PosSizeSrc) == (DestStride - PosSizeDest);

	// Copy vertex data
	if (!DoNotCopyData)
	{
		if (SrcFVF == DestFVF || ((SrcFVF & ~(D3DFVF_XYZ | D3DFVF_RESERVED1)) == (DestFVF & ~D3DFVF_XYZRHW) && SrcStride == DestStride))
		{
			DoNotCopyData = true;
			memcpy(pDestVertex, pSrcVertex, dwCount * DestStride);
		}
	}

	for (UINT i = 0; i < dwCount; ++i)
	{
		// Copy or convert vertex data
		if (!DoNotCopyData)
		{
			if (SimpleCopy)
			{
				memcpy(pDestVertex + PosSizeDest, pSrcVertex + PosSizeSrc, SrcStride - PosSizeSrc);
			}
			else
			{
				ConvertVertex(pDestVertex, DestFVF, pSrcVertex, SrcFVF);
			}
		}

		// Transform vertex
		D3DXVECTOR3& src = *reinterpret_cast<D3DXVECTOR3*>(pSrcVertex);
		D3DXVECTOR4& dst = *reinterpret_cast<D3DXVECTOR4*>(pDestVertex);

		D3DXVECTOR4 h;
		D3DXVECTOR4 pos4(src.x, src.y, src.z, 1.0f);
		D3DXVec4Transform(&h, &pos4, &matWorldViewProj);

		float rhw = (h.w != 0.0f) ? (1.0f / h.w) : 0.0f;

		dst.x = h.x * rhw;
		dst.y = h.y * rhw;
		dst.z = h.z * rhw;
		dst.w = rhw;

		// Lighting
		if (bLighting)
		{
			D3DCOLOR Diffuse = 0, Specular = 0;

			// Extract rotation from world matrix
			D3DMATRIX matWorldRotOnly = matWorld;
			matWorldRotOnly._41 = 0.0f;
			matWorldRotOnly._42 = 0.0f;
			matWorldRotOnly._43 = 0.0f;
			matWorldRotOnly._14 = 0.0f;
			matWorldRotOnly._24 = 0.0f;
			matWorldRotOnly._34 = 0.0f;
			matWorldRotOnly._44 = 1.0f;

			// Transform normal
			D3DXVECTOR3 normal = *reinterpret_cast<D3DXVECTOR3*>(pSrcVertex + NormalSrcOffset);
			D3DXVECTOR3 transformedNormal;
			D3DXVec3TransformNormal(&transformedNormal, &normal, &matWorldRotOnly);
			if (D3DXVec3Length(&normal) > 1e-6f)
			{
				D3DXVec3Normalize(&transformedNormal, &transformedNormal);
			}

			D3DXVECTOR3 transformedPos = { dst.x, dst.y, dst.z };

			ComputeLighting(transformedPos, transformedNormal, cachedLights, lpMaterial, ambient, UseSpecular, Diffuse, Specular);

			if (DiffuseDestOffset)
			{
				*reinterpret_cast<D3DCOLOR*>(pDestVertex + DiffuseDestOffset) = Diffuse;
			}
			if (SpecularDestOffset)
			{
				*reinterpret_cast<D3DCOLOR*>(pDestVertex + SpecularDestOffset) = Specular;
			}
		}

		// Move to the next vertex
		pSrcVertex += SrcStride;
		pDestVertex += DestStride;
	}

	// Unlock the destination vertex buffer
	Unlock();

	return D3D_OK;
}

template HRESULT m_IDirect3DVertexBufferX::TransformVertexUP<XYZ>(m_IDirect3DDeviceX* , XYZ*, D3DTLVERTEX*, D3DHVERTEX*, const DWORD, D3DRECT&, bool, bool);
template HRESULT m_IDirect3DVertexBufferX::TransformVertexUP<D3DVERTEX>(m_IDirect3DDeviceX*, D3DVERTEX*, D3DTLVERTEX*, D3DHVERTEX*, const DWORD, D3DRECT&, bool, bool);
template HRESULT m_IDirect3DVertexBufferX::TransformVertexUP<D3DLVERTEX>(m_IDirect3DDeviceX* , D3DLVERTEX*, D3DTLVERTEX*, D3DHVERTEX*, const DWORD, D3DRECT&, bool, bool);
template <typename T>
HRESULT m_IDirect3DVertexBufferX::TransformVertexUP(m_IDirect3DDeviceX* pDirect3DDeviceX, T* srcVertex, D3DTLVERTEX* destVertex, D3DHVERTEX* pHOut, const DWORD dwCount, D3DRECT& drExtent, bool bLighting, bool bUpdateExtents)
{
	// Check for lighiting, must have source normals and dest diffuse or specular
	if (bLighting)
	{
		if constexpr (std::is_same_v<T, XYZ>)
		{
			bLighting = false;
			LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DVOP_LIGHT' is specified but verticies don't support it: XYZ");
		}
		else if constexpr (std::is_same_v<T, D3DVERTEX>)
		{
			// Do nothing
		}
		else if constexpr (std::is_same_v<T, D3DLVERTEX>)
		{
			bLighting = false;
			LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DVOP_LIGHT' is specified but verticies don't support it: D3DLVERTEX");
		}
		else
		{
			static_assert(false);
		}
	}

	// Cache specular, ambient, material and lights if needed
	bool UseSpecular = false;
	D3DCOLOR ambient = 0;
	LPD3DMATERIAL7 lpMaterial = nullptr;
	D3DMATERIAL7 Material = {};
	std::vector<DXLIGHT7> cachedLights;

	if (bLighting)
	{
		if (DWORD rsSpecular = 0; SUCCEEDED(pDirect3DDeviceX->GetRenderState(D3DRENDERSTATE_SPECULARENABLE, &rsSpecular)))
		{
			UseSpecular = rsSpecular != FALSE;
		}

		pDirect3DDeviceX->GetRenderState(D3DRENDERSTATE_AMBIENT, &ambient);

		if (SUCCEEDED(pDirect3DDeviceX->GetMaterial(&Material)))
		{
			lpMaterial = &Material;
		}

		pDirect3DDeviceX->GetEnabledLightList(cachedLights);

		if (cachedLights.empty())
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: no attached lights found!");
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

	D3DRECT newExtents = { LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN };

	for (DWORD i = 0; i < dwCount; ++i)
	{
		T& src = srcVertex[i];
		D3DTLVERTEX& dst = destVertex[i];

		D3DXVECTOR4 h;
		D3DXVECTOR4 pos4(src.x, src.y, src.z, 1.0f);
		D3DXVec4Transform(&h, &pos4, &matWorldViewProj);

		float rhw = (h.w != 0.0f) ? (1.0f / h.w) : 0.0f;

		dst.sx = h.x * rhw;
		dst.sy = h.y * rhw;
		dst.sz = h.z * rhw;
		dst.rhw = rhw;

		// Default values: set for XYZ or copy for detailed vertex
		if constexpr (std::is_same_v<T, XYZ>)
		{
			dst.color = 0xFFFFFFFF;	// Default to white
			dst.specular = 0;
			dst.tu = 0.0f;
			dst.tv = 0.0f;
		}
		else if constexpr (std::is_same_v<T, D3DVERTEX>)
		{
			D3DCOLOR Diffuse = 0, Specular = 0;

			// Extract rotation from world matrix
			D3DMATRIX matWorldRotOnly = matWorld;
			matWorldRotOnly._41 = 0.0f;
			matWorldRotOnly._42 = 0.0f;
			matWorldRotOnly._43 = 0.0f;
			matWorldRotOnly._14 = 0.0f;
			matWorldRotOnly._24 = 0.0f;
			matWorldRotOnly._34 = 0.0f;
			matWorldRotOnly._44 = 1.0f;

			// Transform normal
			D3DXVECTOR3 normal = { src.nx, src.ny, src.nz };
			D3DXVECTOR3 transformedNormal;
			D3DXVec3TransformNormal(&transformedNormal, &normal, &matWorldRotOnly);
			if (D3DXVec3Length(&normal) > 1e-6f)
			{
				D3DXVec3Normalize(&transformedNormal, &transformedNormal);
			}

			D3DXVECTOR3 transformedPos = { dst.sx, dst.sy, dst.sz };

			ComputeLighting(transformedPos, transformedNormal, cachedLights, lpMaterial, ambient, UseSpecular, Diffuse, Specular);

			dst.color = Diffuse;
			dst.specular = Specular;
			dst.tu = src.tu;
			dst.tv = src.tv;
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
			hdst.hx = h.x;
			hdst.hy = h.y;
			hdst.hz = h.z;
			hdst.dwFlags = 0; // Clip flags not computed here (TransformVertices only sets them if clipping performed upstream)
		}

		// Update extents
		if (bUpdateExtents)
		{
			// floor/ceil convert to integer extents
			newExtents.x1 = min(newExtents.x1, static_cast<LONG>(floorf(dst.sx)));
			newExtents.y1 = min(newExtents.y1, static_cast<LONG>(floorf(dst.sy)));
			newExtents.x2 = max(newExtents.x2, static_cast<LONG>(ceilf(dst.sx)));
			newExtents.y2 = max(newExtents.y2, static_cast<LONG>(ceilf(dst.sy)));
		}
	}

	if (bUpdateExtents && newExtents.x1 != LONG_MAX)
	{
		if (!IsRectZero(drExtent))
		{
			// Merge with existing extents if valid
			drExtent.x1 = min(drExtent.x1, newExtents.x1);
			drExtent.y1 = min(drExtent.y1, newExtents.y1);
			drExtent.x2 = max(drExtent.x2, newExtents.x2);
			drExtent.y2 = max(drExtent.y2, newExtents.y2);
		}
		else
		{
			// First valid extents
			drExtent.x1 = newExtents.x1;
			drExtent.y1 = newExtents.y1;
			drExtent.x2 = newExtents.x2;
			drExtent.y2 = newExtents.y2;
		}
	}

	return D3D_OK;
}

void m_IDirect3DVertexBufferX::ComputeLighting(const D3DVECTOR& Position, const D3DVECTOR& Normal, const std::vector<DXLIGHT7>& cachedLights, const LPD3DMATERIAL7 pMat, D3DCOLOR ambient, bool UseSpecular, D3DCOLOR& outColor, D3DCOLOR& outSpecular)
{
	// Note: assumes Normal and cached lights (spot & directional) are already normalized

	// Vertex normal
	D3DXVECTOR3 worldNormal(Normal.x, Normal.y, Normal.z);

	// If a vertex has zero normals, it usually should be lit only by ambient.
	if (D3DXVec3Length(&worldNormal) < 1e-6f)
	{
		outColor = ambient;
		outSpecular = 0;
		return;
	}

	// Disable specular if material specular is zero
	if (UseSpecular && pMat && IsColorValueZero(pMat->dcvSpecular))
	{
		UseSpecular = false;
	}

	// Default material: fully lit
	bool isDefaultMaterial =
		pMat &&
		pMat->diffuse.r == 1.0f && pMat->diffuse.g == 1.0f && pMat->diffuse.b == 1.0f &&
		pMat->ambient.r == 1.0f && pMat->ambient.g == 1.0f && pMat->ambient.b == 1.0f &&
		!UseSpecular;

	if (isDefaultMaterial)
	{
		outColor = D3DCOLOR_COLORVALUE(1.0f, 1.0f, 1.0f, 1.0f);
		outSpecular = 0;
		return;
	}

	// Position vector
	D3DXVECTOR3 pos(Position.x, Position.y, Position.z);

	D3DXVECTOR3 viewDir = -pos;

	// Diffuse and specular accumulators
	float r = 0.0f, g = 0.0f, b = 0.0f;
	float sr = 0.0f, sg = 0.0f, sb = 0.0f;

	// Ambient light
	float ar = ((ambient >> 16) & 0xFF) / 255.0f;
	float ag = ((ambient >> 8) & 0xFF) / 255.0f;
	float ab = (ambient & 0xFF) / 255.0f;

	for (const auto& light : cachedLights)
	{
		D3DXVECTOR3 toLight;
		float attenuation = 1.0f;

		switch (light.dltType)
		{
		case D3DLIGHT_DIRECTIONAL:
		{
			D3DXVECTOR3 dir(light.dvDirection.x, light.dvDirection.y, light.dvDirection.z);
			toLight = -dir;
			break;
		}
		case D3DLIGHT_POINT:
		case D3DLIGHT_SPOT:
		{
			D3DXVECTOR3 lightPos(light.dvPosition.x, light.dvPosition.y, light.dvPosition.z);
			toLight = lightPos - pos;

			float dist = D3DXVec3Length(&toLight);
			if (dist == 0.0f) continue;

			toLight *= (1.0f / dist);

			attenuation = 1.0f / (light.dvAttenuation0 + light.dvAttenuation1 * dist + light.dvAttenuation2 * dist * dist);
			if (attenuation <= 0.0f) continue;
			if (light.dvRange > 0.0f && dist > light.dvRange) continue;

			if (light.dltType == D3DLIGHT_SPOT)
			{
				D3DXVECTOR3 dir(light.dvDirection.x, light.dvDirection.y, light.dvDirection.z);

				D3DXVECTOR3 negLight = -toLight;
				float spotCos = D3DXVec3Dot(&negLight, &dir);
				spotCos = max(-1.0f, min(1.0f, spotCos));

				float cosPhi = cosf(light.dvPhi * 0.5f);
				if (spotCos < cosPhi) continue;

				float cosTheta = cosf(light.dvTheta * 0.5f);
				float prefalloff = cosTheta != cosPhi ? (spotCos - cosPhi) / (cosTheta - cosPhi) : 1.0f;

				attenuation *= powf(prefalloff, max(light.dvFalloff, 1.0f));
			}
			break;
		}
		default:
			continue; // unsupported light type
		}

		float NdotL = max(0.0f, D3DXVec3Dot(&worldNormal, &toLight));
		if (NdotL <= 0.0f) continue;

		attenuation = min(max(attenuation, 0.0f), 1.0f);

		r += light.dcvDiffuse.r * NdotL * attenuation;
		g += light.dcvDiffuse.g * NdotL * attenuation;
		b += light.dcvDiffuse.b * NdotL * attenuation;

		if (UseSpecular && pMat && !(light.dwFlags & D3DLIGHT_NO_SPECULAR))
		{
			D3DXVECTOR3 reflectDir = (worldNormal * (2.0f * NdotL)) - toLight;

			D3DXVec3Normalize(&reflectDir, &reflectDir);

			float RdotV = max(0.0f, D3DXVec3Dot(&reflectDir, &viewDir));
			float spec = powf(RdotV, CLAMP(pMat->power, 1.0f, 128.0f)) * attenuation;

			if (light.dwLightVersion != 7)
			{
				sr += pMat->specular.r * spec;
				sg += pMat->specular.g * spec;
				sb += pMat->specular.b * spec;
			}
			else
			{
				sr += pMat->specular.r * light.dcvSpecular.r * spec;
				sg += pMat->specular.g * light.dcvSpecular.g * spec;
				sb += pMat->specular.b * light.dcvSpecular.b * spec;
			}
		}
	}

	if (pMat)
	{
		r += pMat->ambient.r * ar;
		g += pMat->ambient.g * ag;
		b += pMat->ambient.b * ab;
	}

	float alpha = pMat ? pMat->diffuse.a : 1.0f;

	// Clamp and convert to DWORD color
	outColor = D3DCOLOR_COLORVALUE(
		CLAMP(r, 0.0f, 1.0f),
		CLAMP(g, 0.0f, 1.0f),
		CLAMP(b, 0.0f, 1.0f),
		alpha);

	// Clamp and convert to DWORD specular
	outSpecular = UseSpecular ? D3DCOLOR_COLORVALUE(
		CLAMP(sr, 0.0f, 1.0f),
		CLAMP(sg, 0.0f, 1.0f),
		CLAMP(sb, 0.0f, 1.0f),
		1.0f) : 0;
}
