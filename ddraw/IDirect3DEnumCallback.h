#pragma once

struct ENUMDEVICES
{
	LPVOID lpContext;
	LPD3DENUMDEVICESCALLBACK lpCallback;
};

class m_IDirect3DEnumDevices
{
public:
	m_IDirect3DEnumDevices() {}
	~m_IDirect3DEnumDevices() {}

	static HRESULT CALLBACK ConvertCallback(LPSTR lpDeviceDescription, LPSTR lpDeviceName, LPD3DDEVICEDESC7 lpDeviceDesc, LPVOID lpContext);
};
