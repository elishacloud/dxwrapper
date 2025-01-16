#pragma once

#include <ostream>

#include <d3d.h>
#include <d3dumddi.h>

std::ostream& operator<<(std::ostream& os, const D3DDDI_RATIONAL& val);
std::ostream& operator<<(std::ostream& os, D3DDDIFORMAT val);
