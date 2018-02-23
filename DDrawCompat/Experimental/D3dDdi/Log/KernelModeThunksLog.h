#pragma once

#define CINTERFACE

#include <ostream>

#include <d3d.h>
#include <d3dumddi.h>
#include <../km/d3dkmthk.h>

std::ostream& operator<<(std::ostream& os, const D3DKMT_CREATECONTEXT& data);
std::ostream& operator<<(std::ostream& os, const D3DKMT_CREATECONTEXTVIRTUAL& data);
std::ostream& operator<<(std::ostream& os, const D3DKMT_CREATEDEVICE& data);
std::ostream& operator<<(std::ostream& os, const D3DKMT_DESTROYCONTEXT& data);
std::ostream& operator<<(std::ostream& os, const D3DKMT_DESTROYDEVICE& data);
std::ostream& operator<<(std::ostream& os, const D3DKMT_PRESENT& data);
std::ostream& operator<<(std::ostream& os, const D3DKMT_SETQUEUEDLIMIT& data);
std::ostream& operator<<(std::ostream& os, const D3DKMT_SETVIDPNSOURCEOWNER& data);
std::ostream& operator<<(std::ostream& os, const D3DKMT_SETVIDPNSOURCEOWNER1& data);
