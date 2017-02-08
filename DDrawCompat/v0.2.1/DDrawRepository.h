#pragma once

#define CINTERFACE

#include <ddraw.h>

namespace DDrawRepository
{
	struct Surface
	{
		DDSURFACEDESC2 desc;
		IDirectDrawSurface7* surface;
	};

	class ScopedSurface : public Surface
	{
	public:
		ScopedSurface(const DDSURFACEDESC2& desc);
		~ScopedSurface();
	};

	IDirectDraw7* getDirectDraw();
}
