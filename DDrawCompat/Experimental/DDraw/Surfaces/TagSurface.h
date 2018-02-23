#pragma once

#include "DDraw/Surfaces/Surface.h"

namespace DDraw
{
	class TagSurface : public Surface
	{
	public:
		virtual ~TagSurface();

		static HRESULT create(CompatRef<IDirectDraw> dd);
		static TagSurface* get(void* ddObject);
	};
}
