/**
* Created from source code found in DdrawCompat v2.0
* https://github.com/narzoul/DDrawCompat/
*
* Updated 2017 by Elisha Riedlinger
*
*/

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

#include "DDrawLog.h"

namespace
{
	template <typename T>
	bool isEmptyStruct(const T& t)
	{
		static T empty = {};
		empty.dwSize = t.dwSize;
		return 0 == memcmp(&t, &empty, sizeof(t));
	}
}

std::ostream& operator<<(std::ostream& os, const RECT& rect)
{
	return os << "R(" << rect.left << ',' << rect.top << ',' << rect.right << ',' << rect.bottom << ')';
}

std::ostream& operator<<(std::ostream& os, const DDSCAPS& caps)
{
	return os << "C(" << std::hex << caps.dwCaps << std::dec << ')';
}

std::ostream& operator<<(std::ostream& os, const DDSCAPS2& caps)
{
	return os << "C(" << std::hex <<
		caps.dwCaps << ',' << caps.dwCaps2 << ',' << caps.dwCaps3 << ',' << caps.dwCaps4 << std::dec << ')';
}

std::ostream& operator<<(std::ostream& os, const DDPIXELFORMAT& pf)
{
	if (isEmptyStruct(pf))
	{
		return os << "PF()";
	}

	return os << "PF(" << std::hex << pf.dwFlags << "," << pf.dwFourCC << "," <<
		std::dec << pf.dwRGBBitCount << "," <<
		std::hex << pf.dwRBitMask << "," << pf.dwGBitMask << "," << pf.dwBBitMask << "," <<
		pf.dwRGBAlphaBitMask << std::dec << ')';
}

std::ostream& operator<<(std::ostream& os, const DDSURFACEDESC& sd)
{
	DDSURFACEDESC2 sd2 = {};
	memcpy(&sd2, &sd, sizeof(sd));
	sd2.dwSize = sizeof(sd2);
	return os << sd2;
}

std::ostream& operator<<(std::ostream& os, const DDSURFACEDESC2& sd)
{
	if (isEmptyStruct(sd))
	{
		return os << "SD()";
	}

	return os << "SD(" << std::hex << sd.dwFlags << std::dec << "," <<
		sd.dwHeight << "," << sd.dwWidth << "," << sd.lPitch << "," << sd.dwBackBufferCount << "," <<
		sd.dwMipMapCount << "," << sd.dwAlphaBitDepth << "," << sd.dwReserved << "," <<
		sd.lpSurface << "," << sd.ddpfPixelFormat << "," << sd.ddsCaps << "," << sd.dwTextureStage << ')';
}

namespace Compat
{

	Log::Log()
	{
		SYSTEMTIME st = {};
		GetLocalTime(&st);

		char time[100];
		sprintf_s(time, "%02hu:%02hu:%02hu.%03hu ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);


		s_logFile << GetCurrentThreadId() << " " << time;
	}

	Log::~Log()
	{
		s_logFile << std::endl;
	}

	//********** Begin Edit *************
	//std::ofstream Log::s_logFile("ddraw.log");
	std::ofstream Log::s_logFile(Log::Init());
	//********** End Edit ***************
}
