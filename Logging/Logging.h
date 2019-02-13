#pragma once

#define CINTERFACE

#include <ddraw.h>
#include "External\Logging\Logging.h"

namespace Logging
{
	void InitLog();
}

std::ostream& operator<<(std::ostream& os, const DDCAPS& caps);
std::ostream& operator<<(std::ostream& os, const DDSCAPS& caps);
std::ostream& operator<<(std::ostream& os, const DDSCAPS2& caps);
std::ostream& operator<<(std::ostream& os, const DDPIXELFORMAT& pf);
std::ostream& operator<<(std::ostream& os, const DDSURFACEDESC& sd);
std::ostream& operator<<(std::ostream& os, const DDSURFACEDESC2& sd);
std::ostream& operator<<(std::ostream& os, REFIID riid);
