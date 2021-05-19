/**
* Copyright (C) 2021 Elisha Riedlinger
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

#include "IAMMediaStream.h"

#define LogDebug Log

HRESULT m_IAMMediaStream::QueryInterface(REFIID riid, LPVOID FAR * ppvObj)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") " << riid;

	if (ppvObj && (riid == WrapperID || riid == IID_IUnknown))
	{
		AddRef();

		*ppvObj = this;

		return DD_OK;
	}

	return E_POINTER;
}

ULONG m_IAMMediaStream::AddRef()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return InterlockedIncrement(&RefCount);
}

ULONG m_IAMMediaStream::Release()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	ULONG ref = InterlockedDecrement(&RefCount);

	if (ref == 0)
	{
		delete this;
	}

	return ref;
}

HRESULT m_IAMMediaStream::GetMultiMediaStream(IMultiMediaStream **ppMultiMediaStream)
{
	UNREFERENCED_PARAMETER(ppMultiMediaStream);

	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");

	return E_FAIL;
}

HRESULT m_IAMMediaStream::GetInformation(MSPID *pPurposeId, STREAM_TYPE *pType)
{
	UNREFERENCED_PARAMETER(pPurposeId);
	UNREFERENCED_PARAMETER(pType);

	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");

	return E_FAIL;
}

HRESULT m_IAMMediaStream::SetSameFormat(IMediaStream *pStreamThatHasDesiredFormat, DWORD dwFlags)
{
	UNREFERENCED_PARAMETER(pStreamThatHasDesiredFormat);
	UNREFERENCED_PARAMETER(dwFlags);

	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");

	return E_FAIL;
}

HRESULT m_IAMMediaStream::AllocateSample(DWORD dwFlags, IStreamSample **ppSample)
{
	UNREFERENCED_PARAMETER(dwFlags);
	UNREFERENCED_PARAMETER(ppSample);

	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");

	return E_FAIL;
}

HRESULT m_IAMMediaStream::CreateSharedSample(IStreamSample *pExistingSample, DWORD dwFlags, IStreamSample **ppNewSample)
{
	UNREFERENCED_PARAMETER(pExistingSample);
	UNREFERENCED_PARAMETER(dwFlags);
	UNREFERENCED_PARAMETER(ppNewSample);

	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");

	return E_FAIL;
}

HRESULT m_IAMMediaStream::SendEndOfStream(DWORD dwFlags)
{
	UNREFERENCED_PARAMETER(dwFlags);

	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");

	return E_FAIL;
}

// IAMMediaStream
HRESULT m_IAMMediaStream::Initialize(IUnknown *pSourceObject, DWORD dwFlags, REFMSPID PurposeId, const STREAM_TYPE StreamType)
{
	UNREFERENCED_PARAMETER(pSourceObject);
	UNREFERENCED_PARAMETER(dwFlags);
	UNREFERENCED_PARAMETER(PurposeId);
	UNREFERENCED_PARAMETER(StreamType);

	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");

	return E_FAIL;
}

HRESULT m_IAMMediaStream::SetState(FILTER_STATE State)
{
	UNREFERENCED_PARAMETER(State);

	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");

	return E_FAIL;
}

HRESULT m_IAMMediaStream::JoinAMMultiMediaStream(IAMMultiMediaStream *pAMMultiMediaStream)
{
	UNREFERENCED_PARAMETER(pAMMultiMediaStream);

	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");

	return E_FAIL;
}

HRESULT m_IAMMediaStream::JoinFilter(IMediaStreamFilter *pMediaStreamFilter)
{
	UNREFERENCED_PARAMETER(pMediaStreamFilter);

	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");

	return E_FAIL;
}

HRESULT m_IAMMediaStream::JoinFilterGraph(IFilterGraph *pFilterGraph)
{
	UNREFERENCED_PARAMETER(pFilterGraph);

	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	LOG_LIMIT(100, __FUNCTION__ << " Not Implemented");

	return E_FAIL;
}
