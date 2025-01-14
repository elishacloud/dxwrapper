/**
* Copyright (C) 2024 Elisha Riedlinger
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

// Cached wrapper interface
namespace {
	m_IDirect3DExecuteBuffer* WrapperInterfaceBackup = nullptr;
}

inline static void SaveInterfaceAddress(m_IDirect3DExecuteBuffer* Interface, m_IDirect3DExecuteBuffer*& InterfaceBackup)
{
	if (Interface)
	{
		SetCriticalSection();
		Interface->SetProxy(nullptr, nullptr, nullptr);
		if (InterfaceBackup)
		{
			InterfaceBackup->DeleteMe();
			InterfaceBackup = nullptr;
		}
		InterfaceBackup = Interface;
		ReleaseCriticalSection();
	}
}

m_IDirect3DExecuteBuffer* CreateDirect3DExecuteBuffer(IDirect3DExecuteBuffer* aOriginal, m_IDirect3DDeviceX* NewD3DDInterface, LPD3DEXECUTEBUFFERDESC lpDesc)
{
	SetCriticalSection();
	m_IDirect3DExecuteBuffer* Interface = nullptr;
	if (WrapperInterfaceBackup)
	{
		Interface = WrapperInterfaceBackup;
		WrapperInterfaceBackup = nullptr;
		Interface->SetProxy(aOriginal, NewD3DDInterface, lpDesc);
	}
	else
	{
		if (aOriginal)
		{
			Interface = new m_IDirect3DExecuteBuffer(aOriginal);
		}
		else
		{
			Interface = new m_IDirect3DExecuteBuffer(NewD3DDInterface, lpDesc);
		}
	}
	ReleaseCriticalSection();
	return Interface;
}

HRESULT m_IDirect3DExecuteBuffer::QueryInterface(REFIID riid, LPVOID FAR * ppvObj)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") " << riid;

	if (!ProxyInterface && !D3DDeviceInterface)
	{
		if (ppvObj)
		{
			*ppvObj = nullptr;
		}
		return E_NOINTERFACE;
	}

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

	if (riid == IID_IDirect3DExecuteBuffer || riid == IID_IUnknown)
	{
		AddRef();

		*ppvObj = this;

		return D3D_OK;
	}

	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, WrapperID);
}

ULONG m_IDirect3DExecuteBuffer::AddRef()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface && !D3DDeviceInterface)
	{
		return 0;
	}

	if (!ProxyInterface)
	{
		return InterlockedIncrement(&RefCount);
	}

	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DExecuteBuffer::Release()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface && !D3DDeviceInterface)
	{
		return 0;
	}

	LONG ref;

	if (!ProxyInterface)
	{
		ref = InterlockedDecrement(&RefCount);
	}
	else
	{
		ref = ProxyInterface->Release();
	}

	if (ref == 0)
	{
		SaveInterfaceAddress(this, WrapperInterfaceBackup);

		ReleaseInterface();
	}

	return ref;
}

HRESULT m_IDirect3DExecuteBuffer::Initialize(LPDIRECT3DDEVICE lpDirect3DDevice, LPD3DEXECUTEBUFFERDESC lpDesc)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface && !D3DDeviceInterface)
	{
		return DDERR_INVALIDOBJECT;
	}

	if (!ProxyInterface)
	{
		// The method returns DDERR_ALREADYINITIALIZED because the Direct3DExecuteBuffer object is initialized when it is created.
		return DDERR_ALREADYINITIALIZED;
	}

	if (lpDirect3DDevice)
	{
		lpDirect3DDevice->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDirect3DDevice);
	}

	return ProxyInterface->Initialize(lpDirect3DDevice, lpDesc);
}

HRESULT m_IDirect3DExecuteBuffer::Lock(LPD3DEXECUTEBUFFERDESC lpDesc)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface && !D3DDeviceInterface)
	{
		return DDERR_INVALIDOBJECT;
	}

	if (!ProxyInterface)
	{
		if (!lpDesc)
		{
			return DDERR_INVALIDPARAMS;
		}

		if (lpDesc->dwSize != sizeof(D3DEXECUTEBUFFERDESC))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << lpDesc->dwSize);
			return DDERR_INVALIDPARAMS;
		}

		// Check if the buffer is already locked
		if (IsLocked)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Buffer is already locked!");
			return D3DERR_EXECUTE_LOCKED;
		}

		// Provide access to the execute buffer memory
		*lpDesc = Desc;

		// Mark the buffer as locked
		IsLocked = true;

		// Mark data as unvalidated
		IsDataValidated = false;

		return D3D_OK;
	}

	return ProxyInterface->Lock(lpDesc);
}

HRESULT m_IDirect3DExecuteBuffer::Unlock()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface && !D3DDeviceInterface)
	{
		return DDERR_INVALIDOBJECT;
	}

	if (!ProxyInterface)
	{
		// Check if the buffer is not locked
		if (!IsLocked)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Buffer is not locked!");
			return D3DERR_EXECUTE_NOT_LOCKED;
		}

		// Mark the buffer as unlocked
		IsLocked = false;

		// No specific action required, just return success
		return D3D_OK;
	}

	return ProxyInterface->Unlock();
}

HRESULT m_IDirect3DExecuteBuffer::ValidateInstructionData(const void* lpData, DWORD dwInstructionOffset, DWORD dwInstructionLength)
{
	const BYTE* data = (const BYTE*)lpData + dwInstructionOffset;
	DWORD offset = 0;

	while (offset < dwInstructionLength)
	{
		if (offset + sizeof(D3DINSTRUCTION) > dwInstructionLength)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Instruction header exceeds bounds!");
			return DDERR_INVALIDPARAMS;
		}

		const D3DINSTRUCTION* instruction = (const D3DINSTRUCTION*)(data + offset);

		if (instruction->bOpcode == 0 || instruction->bOpcode > 14)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid opcode: " << instruction->bOpcode);
			return DDERR_INVALIDPARAMS;
		}

		DWORD instructionSize = sizeof(D3DINSTRUCTION) + (instruction->wCount * instruction->bSize);
		if (offset + instructionSize > dwInstructionLength)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Instruction data exceeds bounds!");
			return DDERR_INVALIDPARAMS;
		}

		if (instruction->bOpcode == D3DOP_EXIT)
		{
			// Valid and terminated properly
			IsDataValidated = true;
			return D3D_OK;
		}

		// Move to the next instruction
		offset += instructionSize;
	}

	LOG_LIMIT(100, __FUNCTION__ << " Error: Missing D3DOP_EXIT!");
	return DDERR_INVALIDPARAMS;
}

HRESULT m_IDirect3DExecuteBuffer::SetExecuteData(LPD3DEXECUTEDATA lpData)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface && !D3DDeviceInterface)
	{
		return DDERR_INVALIDOBJECT;
	}

	if (!ProxyInterface)
	{
		if (!lpData)
		{
			return DDERR_INVALIDPARAMS;
		}

		if (lpData->dwSize != sizeof(D3DEXECUTEDATA))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << lpData->dwSize);
			return DDERR_INVALIDPARAMS;
		}

		// Check if the buffer is locked
		if (IsLocked)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Buffer is locked!");
			return D3DERR_EXECUTE_LOCKED;
		}

		// Ensure that the instruction range is within the bounds of the execute buffer
		if (lpData->dwInstructionOffset + lpData->dwInstructionLength > Desc.dwBufferSize)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid instruction range!");
			return DDERR_INVALIDPARAMS;
		}

		// Ensure the vertex data lies within the buffer and does not overlap with instructions
		if (lpData->dwVertexOffset + lpData->dwVertexCount * sizeof(D3DVERTEX) > Desc.dwBufferSize)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid vertex range!");
			return DDERR_INVALIDPARAMS;
		}

		// Ensure the data segments (vertices and instructions) do not overlap unintentionally
		if (lpData->dwVertexOffset < lpData->dwInstructionOffset + lpData->dwInstructionLength &&
			lpData->dwVertexOffset + lpData->dwVertexCount * sizeof(D3DVERTEX) > lpData->dwInstructionOffset)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Overlapping data regions!");
			return DDERR_INVALIDPARAMS;
		}

		// Validate instruction data
		HRESULT hr = ValidateInstructionData(Desc.lpData, lpData->dwInstructionOffset, lpData->dwInstructionLength);
		if (FAILED(hr))
		{
			return hr;
		}

		// Store execute data
		ExecuteData = *lpData;

		return D3D_OK;
	}

	return ProxyInterface->SetExecuteData(lpData);
}

HRESULT m_IDirect3DExecuteBuffer::GetExecuteData(LPD3DEXECUTEDATA lpData)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface && !D3DDeviceInterface)
	{
		return DDERR_INVALIDOBJECT;
	}

	if (!ProxyInterface)
	{
		if (!lpData)
		{
			return DDERR_INVALIDPARAMS;
		}

		if (lpData->dwSize != sizeof(D3DEXECUTEDATA))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << lpData->dwSize);
			return DDERR_INVALIDPARAMS;
		}

		// Check if the buffer is locked
		if (IsLocked)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Buffer is locked!");
			return D3DERR_EXECUTE_LOCKED;
		}

		// Return stored execute data
		*lpData = ExecuteData;

		return D3D_OK;
	}

	return ProxyInterface->GetExecuteData(lpData);
}

HRESULT m_IDirect3DExecuteBuffer::GetExecuteData(D3DEXECUTEBUFFERDESC& CurrentDesc, D3DEXECUTEDATA& CurrentExecuteData)
{
	if (!IsDataValidated)
	{
		HRESULT hr = ValidateInstructionData(&Desc, ExecuteData.dwInstructionOffset, ExecuteData.dwInstructionLength);
		if (FAILED(hr))
		{
			return hr;
		}
	}

	CurrentDesc = Desc;
	CurrentExecuteData = ExecuteData;

	return D3D_OK;
}

HRESULT m_IDirect3DExecuteBuffer::Validate(LPDWORD lpdwOffset, LPD3DVALIDATECALLBACK lpFunc, LPVOID lpUserArg, DWORD dwReserved)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface && !D3DDeviceInterface)
	{
		return DDERR_INVALIDOBJECT;
	}

	if (!ProxyInterface)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	return ProxyInterface->Validate(lpdwOffset, lpFunc, lpUserArg, dwReserved);
}

HRESULT m_IDirect3DExecuteBuffer::Optimize(DWORD dwDummy)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface && !D3DDeviceInterface)
	{
		return DDERR_INVALIDOBJECT;
	}

	if (!ProxyInterface)
	{
		// The Optimize function doesn't exist in Direct3D9 because it manages vertex buffer optimizations internally
		return D3D_OK;
	}

	return ProxyInterface->Optimize(dwDummy);
}

/************************/
/*** Helper functions ***/
/************************/

void m_IDirect3DExecuteBuffer::InitInterface(LPD3DEXECUTEBUFFERDESC lpDesc)
{
	IsLocked = false;
	IsDataValidated = false;
	ExecuteData = {};
	ExecuteData.dwSize = sizeof(D3DEXECUTEDATA);
	Desc = {};
	Desc.dwSize = sizeof(D3DEXECUTEBUFFERDESC);
	MemoryData.clear();

	if (lpDesc)
	{
		Desc = *lpDesc;
		Desc.dwFlags &= (D3DDEB_BUFSIZE | D3DDEB_CAPS | D3DDEB_LPDATA);
		if (!(Desc.dwFlags & D3DDEB_CAPS))
		{
			Desc.dwFlags |= D3DDEB_CAPS;
			Desc.dwCaps = D3DDEBCAPS_SYSTEMMEMORY;
		}
		Desc.dwCaps &= D3DDEBCAPS_MEM;
		if (!(Desc.dwFlags & D3DDEB_LPDATA) || !Desc.lpData)
		{
			Desc.dwFlags |= D3DDEB_LPDATA;
			MemoryData.resize(Desc.dwBufferSize, 0);
			Desc.lpData = MemoryData.data();
		}
	}
}

void m_IDirect3DExecuteBuffer::ReleaseInterface()
{
	if (D3DDeviceInterface)
	{
		D3DDeviceInterface->ReleaseExecuteBuffer(this);
	}
}
