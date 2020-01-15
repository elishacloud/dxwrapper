/**
* Copyright (C) 2020 Elisha Riedlinger
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

#include "dsound.h"

void WINAPI DsoundWrapper::genericQueryInterface(REFIID riid, LPVOID * ppvObj)
{
	if (!ppvObj || !*ppvObj)
	{
		return;
	}

#define QUERYINTERFACE(x) \
	if (riid == IID_ ## x) \
		{ \
			*ppvObj = ProxyAddressLookupTableDsound.FindAddress<m_ ## x>(*ppvObj); \
		}

#define QUERYINTERFACE8(x) \
	if (riid == IID_ ## x || riid == IID_ ## x ## 8) \
		{ \
			*ppvObj = ProxyAddressLookupTableDsound.FindAddress<m_ ## x ## 8>(*ppvObj); \
		}

	QUERYINTERFACE8(IDirectSound3DBuffer);
	QUERYINTERFACE8(IDirectSound3DListener);
	QUERYINTERFACE8(IDirectSound);
	QUERYINTERFACE8(IDirectSoundBuffer);
	QUERYINTERFACE8(IDirectSoundCapture);
	QUERYINTERFACE8(IDirectSoundCaptureBuffer);
	QUERYINTERFACE8(IDirectSoundCaptureFXAec);
	QUERYINTERFACE8(IDirectSoundCaptureFXNoiseSuppress);
	QUERYINTERFACE8(IDirectSoundFullDuplex);
	QUERYINTERFACE8(IDirectSoundFXChorus);
	QUERYINTERFACE8(IDirectSoundFXCompressor);
	QUERYINTERFACE8(IDirectSoundFXDistortion);
	QUERYINTERFACE8(IDirectSoundFXEcho);
	QUERYINTERFACE8(IDirectSoundFXFlanger);
	QUERYINTERFACE8(IDirectSoundFXGargle);
	QUERYINTERFACE8(IDirectSoundFXI3DL2Reverb);
	QUERYINTERFACE8(IDirectSoundFXParamEq);
	QUERYINTERFACE8(IDirectSoundFXWavesReverb);
	QUERYINTERFACE8(IDirectSoundNotify);
	QUERYINTERFACE(IKsPropertySet);
}
