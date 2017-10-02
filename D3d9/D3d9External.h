#pragma once

struct __declspec(uuid("81BDCBCA-64D4-426d-AE8D-AD0147F4275C")) IDirect3D9;

IDirect3D9 *WINAPI _Direct3DCreate9(UINT);

namespace d3d9_wrap
{
	constexpr FARPROC Direct3DCreate9 = (FARPROC)*_Direct3DCreate9;
	extern FARPROC Direct3DCreate9_Proxy;
}
