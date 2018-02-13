#pragma once

namespace ddraw
{
	extern FARPROC DirectDrawCreate_var;
	extern FARPROC DirectDrawCreateEx_var;
	HMODULE Load(const char *strName);
}
namespace dsound
{
	extern FARPROC DirectSoundCreate_var;
	extern FARPROC DirectSoundCreate8_var;
	HMODULE Load(const char *strName);
}
namespace d3d8
{
	extern FARPROC Direct3DCreate8_var;
}
namespace ShardProcs
{
	extern FARPROC DllGetClassObject_var;
}
namespace Wrapper
{
	HMODULE CreateWrapper(const char *ProxyDll, const char *WrapperMode);
}
