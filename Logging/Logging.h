#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "External\Logging\Logging.h"

namespace Logging
{
	void InitLog();
}

typedef enum _DDFOURCC { } DDFOURCC;
typedef enum _DDERR { } DDERR;
typedef enum _D3DERR { } D3DERR;
#ifndef DIERR_SET
#define DIERR_SET
typedef enum _DIERR { } DIERR;
#endif
typedef enum _DSERR { } DSERR;
typedef enum _WMMSG { } WMMSG;

std::ostream& operator<<(std::ostream& os, const DDFOURCC& dwFourCC);
std::ostream& operator<<(std::ostream& os, const DDERR& ErrCode);
std::ostream& operator<<(std::ostream& os, const D3DERR& ErrCode);
std::ostream& operator<<(std::ostream& os, const DIERR& ErrCode);
std::ostream& operator<<(std::ostream& os, const DSERR& ErrCode);
std::ostream& operator<<(std::ostream& os, const WMMSG& Id);

#ifdef __DDRAW_INCLUDED__
std::ostream& operator<<(std::ostream& os, const DDCAPS& caps);
std::ostream& operator<<(std::ostream& os, const DDSCAPS& caps);
std::ostream& operator<<(std::ostream& os, const DDSCAPS2& caps);
std::ostream& operator<<(std::ostream& os, const DDPIXELFORMAT& pf);
std::ostream& operator<<(std::ostream& os, const DDCOLORKEY& ck);
std::ostream& operator<<(std::ostream& os, const DDSURFACEDESC& sd);
std::ostream& operator<<(std::ostream& os, const DDSURFACEDESC2& sd);
#endif
#ifdef _D3DCAPS_H
std::ostream& operator<<(std::ostream& os, const D3DPRIMCAPS& pc);
std::ostream& operator<<(std::ostream& os, const D3DDEVICEDESC7& dd);
#endif
#ifdef _d3d9TYPES_H_
std::ostream& operator<<(std::ostream& os, const D3DFORMAT& format);
std::ostream& operator<<(std::ostream& os, const D3DPRESENT_PARAMETERS& pp);
#endif
#ifdef GUID_DEFINED
std::ostream& operator<<(std::ostream& os, REFIID riid);
#endif
