#pragma once

#include "External\Logging\Logging.h"

typedef enum _DDERR {} DDERR;

std::ostream& operator<<(std::ostream& os, REFIID riid);
std::ostream& operator<<(std::ostream& os, const DDERR& ErrCode);
std::ostream& operator<<(std::ostream& os, const D3DCOLORVALUE& data);
