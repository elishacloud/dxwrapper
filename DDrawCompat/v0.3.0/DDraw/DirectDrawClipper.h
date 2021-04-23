#pragma once

#include <ddraw.h>

#include <DDrawCompat/v0.3.0/Common/CompatRef.h>

namespace DDraw
{
	namespace DirectDrawClipper
	{
		HRGN getClipRgn(CompatRef<IDirectDrawClipper> clipper);
		HRESULT setClipRgn(CompatRef<IDirectDrawClipper> clipper, HRGN rgn);
		void update();

		void hookVtable(const IDirectDrawClipperVtbl& vtable);
	}
}
