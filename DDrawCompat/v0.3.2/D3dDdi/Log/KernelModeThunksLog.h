#pragma once

#include <ostream>

#include <d3d.h>
#include <d3dumddi.h>
#include <winternl.h>
#include <d3dkmthk.h>

#include <DDrawCompat/v0.3.2/D3dDdi/Log/CommonLog.h>

std::ostream& operator<<(std::ostream& os, const LUID& luid);
std::ostream& operator<<(std::ostream& os, const D3DKMT_CREATECONTEXT& data);
std::ostream& operator<<(std::ostream& os, const D3DKMT_CREATECONTEXTVIRTUAL& data);
std::ostream& operator<<(std::ostream& os, const D3DKMT_CREATEDCFROMMEMORY& data);
std::ostream& operator<<(std::ostream& os, const D3DKMT_CREATEDEVICE& data);
std::ostream& operator<<(std::ostream& os, const D3DKMT_DESTROYCONTEXT& data);
std::ostream& operator<<(std::ostream& os, const D3DKMT_DESTROYDEVICE& data);
std::ostream& operator<<(std::ostream& os, const D3DKMT_OPENADAPTERFROMHDC& data);
std::ostream& operator<<(std::ostream& os, const D3DKMT_PRESENT& data);
std::ostream& operator<<(std::ostream& os, const D3DKMT_QUERYADAPTERINFO& data);
std::ostream& operator<<(std::ostream& os, const D3DKMT_SETQUEUEDLIMIT& data);
std::ostream& operator<<(std::ostream& os, const D3DKMT_SETVIDPNSOURCEOWNER& data);
std::ostream& operator<<(std::ostream& os, const D3DKMT_SETVIDPNSOURCEOWNER1& data);
