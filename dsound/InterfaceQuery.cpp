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

	QUERYINTERFACE(IDirectSound3DBuffer8);
	QUERYINTERFACE(IDirectSound3DListener8);
	QUERYINTERFACE8(IDirectSound);
	QUERYINTERFACE8(IDirectSoundBuffer);
	QUERYINTERFACE(IDirectSoundCapture8);
	QUERYINTERFACE8(IDirectSoundCaptureBuffer);
	QUERYINTERFACE(IDirectSoundCaptureFXAec8);
	QUERYINTERFACE(IDirectSoundCaptureFXNoiseSuppress8);
	QUERYINTERFACE(IDirectSoundFullDuplex8);
	QUERYINTERFACE(IDirectSoundFXChorus8);
	QUERYINTERFACE(IDirectSoundFXCompressor8);
	QUERYINTERFACE(IDirectSoundFXDistortion8);
	QUERYINTERFACE(IDirectSoundFXEcho8);
	QUERYINTERFACE(IDirectSoundFXFlanger8);
	QUERYINTERFACE(IDirectSoundFXGargle8);
	QUERYINTERFACE(IDirectSoundFXI3DL2Reverb8);
	QUERYINTERFACE(IDirectSoundFXParamEq8);
	QUERYINTERFACE(IDirectSoundFXWavesReverb8);
	QUERYINTERFACE(IDirectSoundNotify8);
	QUERYINTERFACE(IKsPropertySet);
}
