#pragma once

#define CINTERFACE

#include <ddraw.h>

struct DDrawTypes
{
	typedef IDirectDrawSurface TSurface;
	typedef IDirectDrawSurface TCreatedSurface;
	typedef DDSURFACEDESC TSurfaceDesc;
	typedef DDSCAPS TDdsCaps;
	typedef LPDDENUMSURFACESCALLBACK TEnumSurfacesCallbackPtr;
	typedef LPVOID TUnlockParam;
};

struct DDrawTypes2
{
	typedef IDirectDrawSurface2 TSurface;
	typedef IDirectDrawSurface TCreatedSurface;
	typedef DDSURFACEDESC TSurfaceDesc;
	typedef DDSCAPS TDdsCaps;
	typedef LPDDENUMSURFACESCALLBACK TEnumSurfacesCallbackPtr;
	typedef LPVOID TUnlockParam;
};

struct DDrawTypes3
{
	typedef IDirectDrawSurface3 TSurface;
	typedef IDirectDrawSurface3 TCreatedSurface;
	typedef DDSURFACEDESC TSurfaceDesc;
	typedef DDSCAPS TDdsCaps;
	typedef LPDDENUMSURFACESCALLBACK TEnumSurfacesCallbackPtr;
	typedef LPVOID TUnlockParam;
};

struct DDrawTypes4
{
	typedef IDirectDrawSurface4 TSurface;
	typedef IDirectDrawSurface4 TCreatedSurface;
	typedef DDSURFACEDESC2 TSurfaceDesc;
	typedef DDSCAPS2 TDdsCaps;
	typedef LPDDENUMSURFACESCALLBACK2 TEnumSurfacesCallbackPtr;
	typedef LPRECT TUnlockParam;
};

struct DDrawTypes7
{
	typedef IDirectDrawSurface7 TSurface;
	typedef IDirectDrawSurface7 TCreatedSurface;
	typedef DDSURFACEDESC2 TSurfaceDesc;
	typedef DDSCAPS2 TDdsCaps;
	typedef LPDDENUMSURFACESCALLBACK7 TEnumSurfacesCallbackPtr;
	typedef LPRECT TUnlockParam;
};

template <typename Interface>
struct Types;

#define DD_CONCAT(x, y, ...) x##y

#define DD_TYPES(Interface, ...) \
	template <> \
	struct Types<DD_CONCAT(Interface, __VA_ARGS__)> : DD_CONCAT(DDrawTypes, __VA_ARGS__) \
	{}

DD_TYPES(IDirectDraw);
DD_TYPES(IDirectDraw, 2);
DD_TYPES(IDirectDraw, 4);
DD_TYPES(IDirectDraw, 7);

DD_TYPES(IDirectDrawSurface);
DD_TYPES(IDirectDrawSurface, 2);
DD_TYPES(IDirectDrawSurface, 3);
DD_TYPES(IDirectDrawSurface, 4);
DD_TYPES(IDirectDrawSurface, 7);

#undef DD_TYPES
#undef DD_CONCAT
