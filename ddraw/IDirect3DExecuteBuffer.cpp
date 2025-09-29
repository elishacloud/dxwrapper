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

namespace {
	constexpr DWORD ExtraDataBufferSize = 256;
}

// ******************************
// IUnknown functions
// ******************************

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

	if (Config.Dd7to9)
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

	if (Config.Dd7to9)
	{
		ULONG ref = (InterlockedCompareExchange(&RefCount, 0, 0)) ? InterlockedDecrement(&RefCount) : 0;

		if (ref == 0)
		{
			SaveInterfaceAddress(this);
		}

		return ref;
	}

	ULONG ref = ProxyInterface->Release();

	if (ref == 0)
	{
		SaveInterfaceAddress(this);
	}

	return ref;
}

// ******************************
// IDirect3DExecuteBuffer functions
// ******************************

HRESULT m_IDirect3DExecuteBuffer::Initialize(LPDIRECT3DDEVICE lpDirect3DDevice, LPD3DEXECUTEBUFFERDESC lpDesc)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface && !D3DDeviceInterface)
	{
		return DDERR_INVALIDOBJECT;
	}

	if (Config.Dd7to9)
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

	if (Config.Dd7to9)
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

		// Check buffer size
		if (!UsingAppMemory && (MemoryData.empty() || MemoryData.size() < Desc.dwBufferSize))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: incorrect buffer size: " << Desc.dwBufferSize << " -> " << MemoryData.size());
			return DDERR_GENERIC;
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

	if (Config.Dd7to9)
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

HRESULT m_IDirect3DExecuteBuffer::SetExecuteData(LPD3DEXECUTEDATA lpExecuteData)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface && !D3DDeviceInterface)
	{
		return DDERR_INVALIDOBJECT;
	}

	if (Config.Dd7to9)
	{
		if (!lpExecuteData)
		{
			return DDERR_INVALIDPARAMS;
		}

		if (lpExecuteData->dwSize != sizeof(D3DEXECUTEDATA))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << lpExecuteData->dwSize);
			return DDERR_INVALIDPARAMS;
		}

		// Check if the buffer is locked
		if (IsLocked)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Buffer is locked!");
			return D3DERR_EXECUTE_LOCKED;
		}

		// Ensure that the instruction range is within the bounds of the execute buffer
		if (lpExecuteData->dwInstructionOffset + lpExecuteData->dwInstructionLength > Desc.dwBufferSize)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid instruction range!");
			return DDERR_INVALIDPARAMS;
		}

		// Ensure the vertex data lies within the buffer and does not overlap with instructions
		if (lpExecuteData->dwVertexOffset + lpExecuteData->dwVertexCount * sizeof(D3DVERTEX) > Desc.dwBufferSize)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid vertex range!");
			return DDERR_INVALIDPARAMS;
		}

		// Ensure the data segments (vertices and instructions) do not overlap unintentionally
		if (lpExecuteData->dwVertexOffset < lpExecuteData->dwInstructionOffset + lpExecuteData->dwInstructionLength &&
			lpExecuteData->dwVertexOffset + lpExecuteData->dwVertexCount * sizeof(D3DVERTEX) > lpExecuteData->dwInstructionOffset)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Overlapping data regions!");
			return DDERR_INVALIDPARAMS;
		}

		// Store execute data
		ExecuteData = *lpExecuteData;

		// Mark data as unvalidated
		IsDataValidated = false;

		return D3D_OK;
	}

	return ProxyInterface->SetExecuteData(lpExecuteData);
}

HRESULT m_IDirect3DExecuteBuffer::GetExecuteData(LPD3DEXECUTEDATA lpExecuteData)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface && !D3DDeviceInterface)
	{
		return DDERR_INVALIDOBJECT;
	}

	if (Config.Dd7to9)
	{
		if (!lpExecuteData)
		{
			return DDERR_INVALIDPARAMS;
		}

		if (lpExecuteData->dwSize != sizeof(D3DEXECUTEDATA))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Incorrect dwSize: " << lpExecuteData->dwSize);
			return DDERR_INVALIDPARAMS;
		}

		// Check if the buffer is locked
		if (IsLocked)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: Buffer is locked!");
			return D3DERR_EXECUTE_LOCKED;
		}

		// Return stored execute data
		*lpExecuteData = ExecuteData;

		return D3D_OK;
	}

	return ProxyInterface->GetExecuteData(lpExecuteData);
}

HRESULT m_IDirect3DExecuteBuffer::Validate(LPDWORD lpdwOffset, LPD3DVALIDATECALLBACK lpFunc, LPVOID lpUserArg, DWORD dwReserved)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface && !D3DDeviceInterface)
	{
		return DDERR_INVALIDOBJECT;
	}

	if (Config.Dd7to9)
	{
		if (!lpdwOffset && !lpFunc)
		{
			return DDERR_INVALIDPARAMS;
		}

		ValidateInstructionData(&ExecuteData, lpdwOffset, lpFunc, lpUserArg);

		return D3D_OK;
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

	if (Config.Dd7to9)
	{
		// The method is not currently supported.
		return D3D_OK;
	}

	return ProxyInterface->Optimize(dwDummy);
}

// ******************************
// Helper functions
// ******************************

void m_IDirect3DExecuteBuffer::InitInterface(LPD3DEXECUTEBUFFERDESC lpDesc)
{
	if (D3DDeviceInterface)
	{
		D3DDeviceInterface->AddExecuteBuffer(this);
	}

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
			MemoryData.resize(Desc.dwBufferSize + ExtraDataBufferSize, 0);
			Desc.lpData = MemoryData.data();
		}
		else
		{
			UsingAppMemory = true;
			LOG_LIMIT(100, __FUNCTION__ << " Warning: lpData is non-null, using application data.");
		}
	}
}

void m_IDirect3DExecuteBuffer::ReleaseInterface()
{
	if (Config.Exiting)
	{
		return;
	}

	if (D3DDeviceInterface)
	{
		D3DDeviceInterface->ClearExecuteBuffer(this);
	}
}

HRESULT m_IDirect3DExecuteBuffer::GetBuffer(LPVOID* lplpData, D3DEXECUTEDATA& CurrentExecuteData, LPD3DSTATUS* lplpStatus)
{
	if (!lplpData || !lplpStatus)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: invalid params: " << lplpData << " " << lplpStatus);
		return DDERR_INVALIDPARAMS;
	}

	if (!IsDataValidated)
	{
		if (FAILED(ValidateInstructionData(&ExecuteData, nullptr, nullptr, nullptr)) || !IsDataValidated)
		{
			return DDERR_INVALIDPARAMS;
		}
	}

	*lplpData = Desc.lpData;
	CurrentExecuteData = ExecuteData;
	*lplpStatus = &ExecuteData.dsStatus;

	return D3D_OK;
}

HRESULT m_IDirect3DExecuteBuffer::ValidateInstructionData(LPD3DEXECUTEDATA lpExecuteData, LPDWORD lpdwOffset, LPD3DVALIDATECALLBACK lpFunc, LPVOID lpUserArg)
{
	if (!lpExecuteData)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: invalid params!");
		return DDERR_INVALIDPARAMS;
	}

	if (lpdwOffset)
	{
		*lpdwOffset = NULL;
	}

	DWORD dwInstructionOffset = lpExecuteData->dwInstructionOffset;
	DWORD dwInstructionLength = lpExecuteData->dwInstructionLength;

	const BYTE* data = (const BYTE*)Desc.lpData + dwInstructionOffset;
	DWORD offset = 0;

	bool FoundError = false;

	while (true)
	{
		// Check offset exceeds instruction length
		if (offset > dwInstructionLength)
		{
			FoundError = true;
			if (lpFunc)
			{
				lpFunc(lpUserArg, offset);
			}
			else if (lpdwOffset)
			{
				*lpdwOffset = offset;
				return D3D_OK;
			}
			else
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: Missing D3DOP_EXIT!");
				return DDERR_INVALIDPARAMS;
			}
			return D3D_OK;
		}

		// Check instruction header bounds
		if (offset + sizeof(D3DINSTRUCTION) > dwInstructionLength)
		{
			FoundError = true;
			if (lpFunc)
			{
				lpFunc(lpUserArg, offset);
			}
			else if (lpdwOffset)
			{
				*lpdwOffset = offset;
				return D3D_OK;
			}
			else
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: Instruction header exceeds bounds!");
				return DDERR_INVALIDPARAMS;
			}
		}

		const D3DINSTRUCTION* instruction = (const D3DINSTRUCTION*)(data + offset);

		// Check instruction data bounds
		DWORD instructionSize = sizeof(D3DINSTRUCTION) + (instruction->wCount * instruction->bSize);
		if (offset + instructionSize > dwInstructionLength)
		{
			FoundError = true;
			if (lpFunc)
			{
				lpFunc(lpUserArg, offset);
			}
			else if (lpdwOffset)
			{
				*lpdwOffset = offset;
				return D3D_OK;
			}
			else
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: Instruction data exceeds bounds!");
				return DDERR_INVALIDPARAMS;
			}
		}

		// Check opcode
		if (instruction->bOpcode == 0 || instruction->bOpcode > 14)
		{
			FoundError = true;
			if (lpFunc)
			{
				lpFunc(lpUserArg, offset);
			}
			else if (lpdwOffset)
			{
				*lpdwOffset = offset;
				return D3D_OK;
			}
			else
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid opcode: " << (DWORD)instruction->bOpcode);
				return DDERR_INVALIDPARAMS;
			}
		}

		// Check for termination
		if (instruction->bOpcode == D3DOP_EXIT)
		{
			if (!FoundError)
			{
				IsDataValidated = true;
			}
			return D3D_OK;
		}

		// Check instruction size
		DWORD StructSize =
			instruction->bOpcode == D3DOP_POINT ? sizeof(D3DPOINT) :
			instruction->bOpcode == D3DOP_LINE ? sizeof(D3DLINE) :
			instruction->bOpcode == D3DOP_TRIANGLE ? sizeof(D3DTRIANGLE) :
			instruction->bOpcode == D3DOP_MATRIXLOAD ? sizeof(D3DMATRIXLOAD) :
			instruction->bOpcode == D3DOP_MATRIXMULTIPLY ? sizeof(D3DMATRIXMULTIPLY) :
			instruction->bOpcode == D3DOP_STATETRANSFORM ? sizeof(D3DSTATE) :
			instruction->bOpcode == D3DOP_STATELIGHT ? sizeof(D3DSTATE) :
			instruction->bOpcode == D3DOP_STATERENDER ? sizeof(D3DSTATE) :
			instruction->bOpcode == D3DOP_TEXTURELOAD ? sizeof(D3DTEXTURELOAD) :
			instruction->bOpcode == D3DOP_PROCESSVERTICES ? sizeof(D3DPROCESSVERTICES) :
			instruction->bOpcode == D3DOP_SPAN ? sizeof(D3DSPAN) :
			instruction->bOpcode == D3DOP_SETSTATUS ? sizeof(D3DSTATUS) :
			instruction->bOpcode == D3DOP_BRANCHFORWARD ? sizeof(D3DBRANCH) : 0;
		if (instruction->bSize != StructSize)
		{
			FoundError = true;
			if (lpFunc)
			{
				lpFunc(lpUserArg, offset);
			}
			else if (lpdwOffset)
			{
				*lpdwOffset = offset;
				return D3D_OK;
			}
			else
			{
				LOG_LIMIT(100, __FUNCTION__ << " Error: Instruction size does not match structure size: " << StructSize << "->" << instruction->bSize);
				return DDERR_INVALIDPARAMS;
			}
		}

		// Handle branch
		bool SkipNextMove = false;
		if (instruction->bOpcode == D3DOP_BRANCHFORWARD)
		{
			D3DBRANCH* branch = reinterpret_cast<D3DBRANCH*>((BYTE*)instruction + sizeof(D3DINSTRUCTION));

			DWORD EmulatedDriverStatus = 0;	// Just use 0 for now
			if (bool((EmulatedDriverStatus & branch->dwMask) == branch->dwValue) != bool(branch->bNegate))
			{
				// Exit the execute buffer if offset is null
				if (branch->dwOffset == 0)
				{
					if (!FoundError)
					{
						IsDataValidated = true;
					}
					return D3D_OK;
				}
				// Move the instruction pointer forward by the offset
				else
				{
					SkipNextMove = true;
					offset += branch->dwOffset;
				}
			}
		}

		// Move to the next instruction
		if (!SkipNextMove)
		{
			offset += instructionSize;
		}
	}

	return D3D_OK;
}

m_IDirect3DExecuteBuffer* m_IDirect3DExecuteBuffer::CreateDirect3DExecuteBuffer(IDirect3DExecuteBuffer* aOriginal, m_IDirect3DDeviceX* NewD3DDInterface, LPD3DEXECUTEBUFFERDESC lpDesc)
{
	m_IDirect3DExecuteBuffer* Interface = InterfaceAddressCache<m_IDirect3DExecuteBuffer>(nullptr);
	if (Interface)
	{
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
	return Interface;
}
