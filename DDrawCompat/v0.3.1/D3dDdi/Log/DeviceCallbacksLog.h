#pragma once

#include <ostream>

#include <d3d.h>
#include <d3dumddi.h>
#include <winternl.h>
#include <d3dkmthk.h>

#include <DDrawCompat/v0.3.1/D3dDdi/Log/CommonLog.h>

std::ostream& operator<<(std::ostream& os, const D3DDDI_ALLOCATIONINFO& data);
std::ostream& operator<<(std::ostream& os, const D3DDDICB_ALLOCATE& data);
std::ostream& operator<<(std::ostream& os, const D3DDDICB_DEALLOCATE& data);
std::ostream& operator<<(std::ostream& os, const D3DDDICB_DEALLOCATE2& data);
std::ostream& operator<<(std::ostream& os, const D3DDDICB_LOCK& data);
std::ostream& operator<<(std::ostream& os, const D3DDDICB_LOCK2& data);
std::ostream& operator<<(std::ostream& os, const D3DDDICB_PRESENT& data);
std::ostream& operator<<(std::ostream& os, const D3DDDICB_UNLOCK& data);
std::ostream& operator<<(std::ostream& os, const D3DDDICB_UNLOCK2& data);
