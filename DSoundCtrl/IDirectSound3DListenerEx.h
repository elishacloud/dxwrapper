/**
* Copyright (c) 2003-2007, Arne Bockholdt, github@bockholdt.info
*
* This file is part of Direct Sound Control.
*
* Direct Sound Control is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Direct Sound Control is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with Direct Sound Control.  If not, see <http://www.gnu.org/licenses/>.
*
* Updated 2017 by Elisha Riedlinger
*/

#pragma once

#define IDIRECTSOUND3DLISTENER8EX_CLASS_NAME	"IDirectSound3DListener8Ex"

#ifdef _DEBUG
extern void LogMessage(const char* szClassName, void* pInstance, char* szMessage);
extern bool	g_bLogDirectSound3DListener;
#endif // _DEBUG

class IDirectSound3DListener8Ex : public IDirectSound3DListener8
{
public:
	IDirectSound3DListener8Ex(void);
	virtual ~IDirectSound3DListener8Ex(void);

	virtual HRESULT __stdcall QueryInterface(REFIID, LPVOID *);
	virtual ULONG __stdcall AddRef();
	virtual ULONG __stdcall Release();

	// IDirectSound3DListener methods
	virtual HRESULT __stdcall GetAllParameters(LPDS3DLISTENER pListener);
	virtual HRESULT __stdcall GetDistanceFactor(D3DVALUE* pflDistanceFactor);
	virtual HRESULT __stdcall GetDopplerFactor(D3DVALUE* pflDopplerFactor);
	virtual HRESULT __stdcall GetOrientation(D3DVECTOR* pvOrientFront, D3DVECTOR* pvOrientTop);
	virtual HRESULT __stdcall GetPosition(D3DVECTOR* pvPosition);
	virtual HRESULT __stdcall GetRolloffFactor(D3DVALUE* pflRolloffFactor);
	virtual HRESULT __stdcall GetVelocity(D3DVECTOR* pvVelocity);
	virtual HRESULT __stdcall SetAllParameters(LPCDS3DLISTENER pcListener, DWORD dwApply);
	virtual HRESULT __stdcall SetDistanceFactor(D3DVALUE flDistanceFactor, DWORD dwApply);
	virtual HRESULT __stdcall SetDopplerFactor(D3DVALUE flDopplerFactor, DWORD dwApply);
	virtual HRESULT __stdcall SetOrientation(D3DVALUE xFront, D3DVALUE yFront, D3DVALUE zFront, D3DVALUE xTop, D3DVALUE yTop, D3DVALUE zTop, DWORD dwApply);
	virtual HRESULT __stdcall SetPosition(D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply);
	virtual HRESULT __stdcall SetRolloffFactor(D3DVALUE flRolloffFactor, DWORD dwApply);
	virtual HRESULT __stdcall SetVelocity(D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply);
	virtual HRESULT __stdcall CommitDeferredSettings();

	LPDIRECTSOUND3DLISTENER8 m_lpDirectSound3DListener8;
protected:
#ifdef _DEBUG
	const char* m_cszClassName;
#endif // _DEBUG
};
