#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "External\Logging\Logging.h"

namespace Logging
{
	void InitLog();
}

#ifdef __DDRAW_INCLUDED__
std::ostream& operator<<(std::ostream& os, const DDCAPS& caps);
std::ostream& operator<<(std::ostream& os, const DDSCAPS& caps);
std::ostream& operator<<(std::ostream& os, const DDSCAPS2& caps);
std::ostream& operator<<(std::ostream& os, const DDPIXELFORMAT& pf);
std::ostream& operator<<(std::ostream& os, const DDCOLORKEY& ck);
std::ostream& operator<<(std::ostream& os, const DDSURFACEDESC& sd);
std::ostream& operator<<(std::ostream& os, const DDSURFACEDESC2& sd);
#endif
#ifdef _d3d9TYPES_H_
std::ostream& operator<<(std::ostream& os, const D3DPRESENT_PARAMETERS& pp);
#endif
#ifdef GUID_DEFINED
std::ostream& operator<<(std::ostream& os, REFIID riid);
#endif
