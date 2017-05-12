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

#ifdef __cplusplus

extern "C" {  // only need to export C interface if
              // used by C++ source code
#endif

//////////////////////////////////////////////////////////////////////////////////////////////

typedef HRESULT (PASCAL *DirectSoundCreate8func)(LPCGUID, LPDIRECTSOUND8*, LPUNKNOWN );
typedef HRESULT (PASCAL *DirectSoundCreatefunc)(LPCGUID, LPDIRECTSOUND*, LPUNKNOWN );
typedef HRESULT (PASCAL *GetDeviceIDfunc)(LPCGUID, LPGUID);
typedef HRESULT (PASCAL *DirectSoundEnumerateAfunc)(LPDSENUMCALLBACKA, LPVOID);
typedef HRESULT (PASCAL *DirectSoundEnumerateWfunc)(LPDSENUMCALLBACKW, LPVOID);
typedef HRESULT (PASCAL *DirectSoundCaptureCreatefunc)(LPCGUID, LPDIRECTSOUNDCAPTURE*, LPUNKNOWN);
typedef HRESULT (PASCAL *DirectSoundCaptureEnumerateAfunc)(LPDSENUMCALLBACKA, LPVOID);
typedef HRESULT (PASCAL *DirectSoundCaptureEnumerateWfunc)(LPDSENUMCALLBACKW, LPVOID);
typedef HRESULT (PASCAL *DirectSoundCaptureCreate8func)(LPCGUID, LPDIRECTSOUNDCAPTURE8*, LPUNKNOWN);
typedef HRESULT (PASCAL *DirectSoundFullDuplexCreatefunc)(LPCGUID, LPCGUID,LPCDSCBUFFERDESC, LPCDSBUFFERDESC, HWND ,DWORD, LPDIRECTSOUNDFULLDUPLEX*, LPDIRECTSOUNDCAPTUREBUFFER8 *,LPDIRECTSOUNDBUFFER8 *, LPUNKNOWN);
typedef	HRESULT	(PASCAL *DllGetClassObjectfunc) (REFCLSID, REFIID, LPVOID *);
typedef HRESULT (PASCAL *DllCanUnloadNowfunc)(void);

//////////////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif
