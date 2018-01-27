#pragma once

#include <regex>
#include <assert.h>
#include "d3d8to9.hpp"
#include "d3dx9.h"
#include "Logging\Logging.h"


#ifndef D3D8TO9NOLOG
#define LOG Logging::Log()
namespace newstd
{
	using namespace std;
	static constexpr char endl = '\n';
};

#define std newstd
#endif
