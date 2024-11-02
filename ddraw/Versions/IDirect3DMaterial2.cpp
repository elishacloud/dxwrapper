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

#include "..\ddraw.h"

HRESULT m_IDirect3DMaterial2::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	return ProxyInterface->QueryInterface(ReplaceIIDUnknown(riid, WrapperID), ppvObj, DirectXVersion);
}

ULONG m_IDirect3DMaterial2::AddRef()
{
	return ProxyInterface->AddRef(DirectXVersion);
}

ULONG m_IDirect3DMaterial2::Release()
{
	return ProxyInterface->Release(DirectXVersion);
}

HRESULT m_IDirect3DMaterial2::SetMaterial(LPD3DMATERIAL a)
{
	return ProxyInterface->SetMaterial(a);
}

HRESULT m_IDirect3DMaterial2::GetMaterial(LPD3DMATERIAL a)
{
	return ProxyInterface->GetMaterial(a);
}

HRESULT m_IDirect3DMaterial2::GetHandle(LPDIRECT3DDEVICE2 a, LPD3DMATERIALHANDLE b)
{
	return ProxyInterface->GetHandle((LPDIRECT3DDEVICE3)a, b);
}
