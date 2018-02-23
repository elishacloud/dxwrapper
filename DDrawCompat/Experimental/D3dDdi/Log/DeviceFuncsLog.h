#pragma once

#define CINTERFACE

#include <ostream>

#include <d3d.h>
#include <d3dumddi.h>

std::ostream& operator<<(std::ostream& os, const D3DDDI_RATIONAL& val);
std::ostream& operator<<(std::ostream& os, const D3DDDI_SURFACEINFO& val);
std::ostream& operator<<(std::ostream& os, const D3DDDIARG_CLEAR& val);
std::ostream& operator<<(std::ostream& os, const D3DDDIARG_COLORFILL& val);
std::ostream& operator<<(std::ostream& os, const D3DDDIARG_CREATERESOURCE& val);
std::ostream& operator<<(std::ostream& os, const D3DDDIARG_CREATERESOURCE2& val);
std::ostream& operator<<(std::ostream& os, const D3DDDIARG_LOCK& val);
std::ostream& operator<<(std::ostream& os, const D3DDDIARG_OPENRESOURCE& val);
std::ostream& operator<<(std::ostream& os, const D3DDDIARG_PRESENT& val);
std::ostream& operator<<(std::ostream& os, const D3DDDIARG_PRESENT1& val);
std::ostream& operator<<(std::ostream& os, const D3DDDIARG_PRESENTSURFACE& val);
std::ostream& operator<<(std::ostream& os, const D3DDDIARG_RENDERSTATE& val);
std::ostream& operator<<(std::ostream& os, const D3DDDIARG_UNLOCK& val);
std::ostream& operator<<(std::ostream& os, const D3DDDIARG_WINFO& val);
std::ostream& operator<<(std::ostream& os, const D3DDDIARG_ZRANGE& val);
std::ostream& operator<<(std::ostream& os, const D3DDDIBOX& val);
