//////////////////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2003-2007, Arne Bockholdt, github@bockholdt.info
//
// This file is part of Direct Sound Control.
//
// Direct Sound Control is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Direct Sound Control is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Direct Sound Control.  If not, see <http://www.gnu.org/licenses/>.
//
//////////////////////////////////////////////////////////////////////////////////////////////
//
// Updated by Elisha Riedlinger 2017
//
//////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////

#define IDIRECTSOUND3DBUFFER8EX_CLASS_NAME	"IDirectSound3DBuffer8Ex"

//////////////////////////////////////////////////////////////////////////////////////////////


#ifdef _DEBUG
extern void LogMessage(const char* szClassName, void* pInstance, char* szMessage );
extern bool	g_bLogDirectSound3DBuffer;
#endif // _DEBUG

//////////////////////////////////////////////////////////////////////////////////////////////

class IDirectSound3DBuffer8Ex : public IDirectSound3DBuffer8
{
public:
	IDirectSound3DBuffer8Ex(void);
	virtual ~IDirectSound3DBuffer8Ex(void);

    virtual HRESULT __stdcall	QueryInterface	   (REFIID, LPVOID *);
    virtual ULONG   __stdcall	AddRef();
    virtual ULONG   __stdcall	Release();

	// IDirectSound3DBuffer methods
    virtual HRESULT __stdcall	GetAllParameters		(LPDS3DBUFFER pDs3dBuffer);
    virtual HRESULT __stdcall	GetConeAngles			(LPDWORD pdwInsideConeAngle, LPDWORD pdwOutsideConeAngle);
    virtual HRESULT __stdcall	GetConeOrientation		(D3DVECTOR* pvOrientation);
    virtual HRESULT __stdcall	GetConeOutsideVolume	(LPLONG plConeOutsideVolume);
    virtual HRESULT __stdcall	GetMaxDistance			(D3DVALUE* pflMaxDistance);
    virtual HRESULT __stdcall	GetMinDistance			(D3DVALUE* pflMinDistance);
	virtual HRESULT __stdcall	GetMode					(LPDWORD pdwMode);
    virtual HRESULT __stdcall	GetPosition				(D3DVECTOR* pvPosition);
    virtual HRESULT __stdcall	GetVelocity				(D3DVECTOR* pvVelocity);
    virtual HRESULT __stdcall	SetAllParameters		(LPCDS3DBUFFER pcDs3dBuffer, DWORD dwApply);
    virtual HRESULT __stdcall	SetConeAngles			(DWORD dwInsideConeAngle, DWORD dwOutsideConeAngle, DWORD dwApply);
    virtual HRESULT __stdcall	SetConeOrientation		(D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply);
    virtual HRESULT __stdcall	SetConeOutsideVolume	(LONG lConeOutsideVolume, DWORD dwApply);
    virtual HRESULT __stdcall	SetMaxDistance			(D3DVALUE flMaxDistance, DWORD dwApply);
    virtual HRESULT __stdcall	SetMinDistance			(D3DVALUE flMinDistance, DWORD dwApply);
    virtual HRESULT __stdcall	SetMode					(DWORD dwMode, DWORD dwApply);
    virtual HRESULT __stdcall	SetPosition				(D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply);
    virtual HRESULT __stdcall	SetVelocity				(D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply);

	LPDIRECTSOUND3DBUFFER8		m_lpDirectSound3DBuffer8;
protected : 
#ifdef _DEBUG
	const char*					m_cszClassName;
#endif // _DEBUG
};

//////////////////////////////////////////////////////////////////////////////////////////////
