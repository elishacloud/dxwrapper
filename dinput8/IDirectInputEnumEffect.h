#pragma once

struct ENUMEFFECT
{
	LPVOID pvRef;
	LPDIENUMCREATEDEFFECTOBJECTSCALLBACK lpCallback;
};

class m_IDirectInputEnumEffect8
{
public:
	m_IDirectInputEnumEffect8() {}
	~m_IDirectInputEnumEffect8() {}

	static BOOL CALLBACK EnumEffectCallback(LPDIRECTINPUTEFFECT, LPVOID);
};
