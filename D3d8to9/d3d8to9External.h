#pragma once

class __declspec(uuid("1DD9E8DA-1C77-4D40-B0CF-98FEFDFF9512")) Direct3D8;

extern "C" Direct3D8 *WINAPI _Direct3DCreate8(UINT);

namespace D3d8to9
{
	FARPROC Direct3DCreate8 = (FARPROC)*_Direct3DCreate8;
}
