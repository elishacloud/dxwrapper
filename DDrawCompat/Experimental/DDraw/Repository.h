#pragma once

#define CINTERFACE

#include <ddraw.h>

#include "Common/CompatRef.h"
#include "Common/CompatWeakPtr.h"

namespace DDraw
{
	namespace Repository
	{
		struct Surface
		{
			void* ddObject;
			DDSURFACEDESC2 desc;
			CompatWeakPtr<IDirectDrawSurface7> surface;
		};

		class ScopedSurface : public Surface
		{
		public:
			ScopedSurface(CompatRef<IDirectDraw7> dd, const DDSURFACEDESC2& desc);
			~ScopedSurface();
		};

		CompatWeakPtr<IDirectDraw7> getDirectDraw();
		void onRelease(void* ddObject);
	}
}
