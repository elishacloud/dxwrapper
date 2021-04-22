#pragma once

#include <ostream>

#include <d3d.h>
#include <d3dumddi.h>

#include <D3dDdi/Log/CommonLog.h>

std::ostream& operator<<(std::ostream& os, const D3DDDI_ALLOCATIONLIST& data);
std::ostream& operator<<(std::ostream& os, const D3DDDI_PATCHLOCATIONLIST& data);
std::ostream& operator<<(std::ostream& os, const D3DDDIARG_CREATEDEVICE& data);
