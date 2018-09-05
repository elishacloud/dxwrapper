#pragma once

struct ENUMDEVICES
{
	LPVOID lpContext = nullptr;
	LPD3DENUMDEVICESCALLBACK lpCallback = nullptr;
};

class m_IDirect3DEnumDevices
{
public:
	m_IDirect3DEnumDevices() {}
	~m_IDirect3DEnumDevices() {}

	static HRESULT CALLBACK ConvertCallback(LPSTR lpDeviceDescription, LPSTR lpDeviceName, LPD3DDEVICEDESC7 lpDeviceDesc, LPVOID lpContext);
};
