#pragma once

class m_IDirectSoundNotify8 final : public IDirectSoundNotify8, AddressLookupTableDsoundObject<m_IDirectSoundNotify8>
{
private:
	LPDIRECTSOUNDNOTIFY8 ProxyInterface;

public:
	m_IDirectSoundNotify8(LPDIRECTSOUNDNOTIFY8 pSound8) : AddressLookupTableDsoundObject(pSound8), ProxyInterface(pSound8)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");
	}
	~m_IDirectSoundNotify8()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");
	}

	// IUnknown methods
	IFACEMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) override;
	IFACEMETHOD_(ULONG, AddRef)(THIS) override;
	IFACEMETHOD_(ULONG, Release)(THIS) override;

	// IDirectSoundNotify methods
	IFACEMETHOD(SetNotificationPositions)(THIS_ DWORD dwPositionNotifies, _In_reads_(dwPositionNotifies) LPCDSBPOSITIONNOTIFY pcPositionNotifies) override;

	// Helper functions
	LPDIRECTSOUNDNOTIFY8 GetProxyInterface() { return ProxyInterface; }
};
