#pragma once

class m_IDirectSoundNotify8 : public IDirectSoundNotify8, public AddressLookupTableDsoundObject
{
private:
	LPDIRECTSOUNDNOTIFY8 ProxyInterface;

public:
	m_IDirectSoundNotify8(LPDIRECTSOUNDNOTIFY8 pSound8) : ProxyInterface(pSound8)
	{
		LOG_LIMIT(3, "Creating device " << __FUNCTION__ << "(" << this << ")");

		ProxyAddressLookupTableDsound.SaveAddress(this, ProxyInterface);
	}
	~m_IDirectSoundNotify8()
	{
		LOG_LIMIT(3, __FUNCTION__ << "(" << this << ")" << " deleting device!");

		ProxyAddressLookupTableDsound.DeleteAddress(this);
	}

	LPDIRECTSOUNDNOTIFY8 GetProxyInterface() { return ProxyInterface; }

	// IUnknown methods
	STDMETHOD(QueryInterface)(THIS_ _In_ REFIID, _Outptr_ LPVOID*);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	// IDirectSoundNotify methods
	STDMETHOD(SetNotificationPositions)(THIS_ DWORD dwPositionNotifies, _In_reads_(dwPositionNotifies) LPCDSBPOSITIONNOTIFY pcPositionNotifies);
};
