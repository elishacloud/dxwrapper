#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmstream.h>    // multimedia stream interfaces
#include <amstream.h>    // DirectShow multimedia stream interfaces
#include <ddstream.h>    // DirectDraw multimedia stream interfaces
#include "Logging\Logging.h"

class m_IAMMediaStream : public IAMMediaStream
{
private:
	ULONG RefCount = 1;
	REFIID WrapperID;

public:
	m_IAMMediaStream(REFIID riid) : WrapperID(riid)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ") " << WrapperID);
	}
	~m_IAMMediaStream()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR * ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IMediaStream methods ***/
	STDMETHOD(GetMultiMediaStream)(THIS_ IMultiMediaStream **ppMultiMediaStream);
	STDMETHOD(GetInformation)(THIS_ MSPID *pPurposeId, STREAM_TYPE *pType);
	STDMETHOD(SetSameFormat)(THIS_ IMediaStream *pStreamThatHasDesiredFormat, DWORD dwFlags);
	STDMETHOD(AllocateSample)(THIS_ DWORD dwFlags, IStreamSample **ppSample);
	STDMETHOD(CreateSharedSample)(THIS_ IStreamSample *pExistingSample, DWORD dwFlags, IStreamSample **ppNewSample);
	STDMETHOD(SendEndOfStream)(THIS_ DWORD dwFlags);

	/*** IAMMediaStream methods ***/
	STDMETHOD(Initialize)(THIS_ IUnknown *pSourceObject, DWORD dwFlags, REFMSPID PurposeId, const STREAM_TYPE StreamType);
	STDMETHOD(SetState)(THIS_ FILTER_STATE State);
	STDMETHOD(JoinAMMultiMediaStream)(THIS_ IAMMultiMediaStream *pAMMultiMediaStream);
	STDMETHOD(JoinFilter)(THIS_ IMediaStreamFilter *pMediaStreamFilter);
	STDMETHOD(JoinFilterGraph)(THIS_ IFilterGraph *pFilterGraph);
};
