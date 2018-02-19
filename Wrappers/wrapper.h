#pragma once

namespace Wrapper
{
	bool ValidProcAddress(FARPROC ProcAddress);
	void ShimProc(FARPROC &var, FARPROC in, FARPROC &out);
	HMODULE CreateWrapper(const char *ProxyDll, const char *WrapperMode);
}

// Shared procs
#include "shared.h"

#define VISIT_DLLS(visit) \
	visit(bcrypt) \
	visit(cryptsp) \
	visit(d2d1) \
	visit(d3d8) \
	visit(d3d9) \
	visit(d3d10) \
	visit(d3d10core) \
	visit(d3d11) \
	visit(d3d12) \
	visit(d3dim) \
	visit(d3dim700) \
	visit(dciman32) \
	visit(ddraw) \
	visit(dinput) \
	visit(dinput8) \
	visit(dplayx) \
	visit(dsound) \
	visit(dxgi) \
	visit(msacm32) \
	visit(msvfw32) \
	visit(vorbisfile) \
	visit(winmm) \
	visit(winspool) \
	visit(xlive)

// Wrappers
#include "bcrypt.h"
#include "cryptsp.h"
#include "d2d1.h"
#include "d3d8.h"
#include "d3d9.h"
#include "d3d10.h"
#include "d3d10core.h"
#include "d3d11.h"
#include "d3d12.h"
#include "d3dim.h"
#include "d3dim700.h"
#include "dciman32.h"
#include "ddraw.h"
#include "dinput.h"
#include "dinput8.h"
#include "dplayx.h"
#include "dsound.h"
#include "dxgi.h"
#include "msacm32.h"
#include "msvfw32.h"
#include "vorbisfile.h"
#include "winmm.h"
#include "winspool.h"
#include "xlive.h"

#define DECLARE_FORWARD_FUNCTIONS(procName, unused) \
	extern "C" void __stdcall procName();

#define DECLARE_PROC_VARABLES(procName, unused) \
	extern FARPROC procName ## _var;

namespace ShardProcs
{
	VISIT_PROCS_SHAREDPROCS(DECLARE_FORWARD_FUNCTIONS);
	VISIT_PROCS_SHAREDPROCS(DECLARE_PROC_VARABLES);
}
namespace ddraw
{
	VISIT_PROCS_DDRAW(DECLARE_FORWARD_FUNCTIONS);
	VISIT_PROCS_DDRAW(DECLARE_PROC_VARABLES);
	HMODULE Load(const char *strName);
}
namespace d3d8
{
	VISIT_PROCS_D3D8(DECLARE_PROC_VARABLES);
	HMODULE Load(const char *strName);
}
namespace d3d9
{
	VISIT_PROCS_D3D9(DECLARE_FORWARD_FUNCTIONS);
	VISIT_PROCS_D3D9(DECLARE_PROC_VARABLES);
	HMODULE Load(const char *strName);
}
namespace dsound
{
	VISIT_PROCS_DSOUND(DECLARE_FORWARD_FUNCTIONS);
	VISIT_PROCS_DSOUND(DECLARE_PROC_VARABLES);
	HMODULE Load(const char *strName);
}
