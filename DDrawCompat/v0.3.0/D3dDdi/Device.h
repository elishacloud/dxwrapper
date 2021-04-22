#pragma once

#include <map>
#include <unordered_map>

#include <d3d.h>
#include <d3dnthal.h>
#include <d3dumddi.h>

#include <D3dDdi/DeviceState.h>
#include <D3dDdi/DrawPrimitive.h>

namespace D3dDdi
{
	class Adapter;
	class Resource;

	class Device
	{
	public:
		Device(Adapter& adapter, HANDLE device);

		Device(const Device&) = delete;
		Device(Device&&) = delete;
		Device& operator=(const Device&) = delete;
		Device& operator=(Device&&) = delete;

		operator HANDLE() const { return m_device; }

		HRESULT pfnBlt(const D3DDDIARG_BLT* data);
		HRESULT pfnClear(const D3DDDIARG_CLEAR* data, UINT numRect, const RECT* rect);
		HRESULT pfnColorFill(const D3DDDIARG_COLORFILL* data);
		HRESULT pfnCreateResource(D3DDDIARG_CREATERESOURCE* data);
		HRESULT pfnCreateResource2(D3DDDIARG_CREATERESOURCE2* data);
		HRESULT pfnDestroyDevice();
		HRESULT pfnDestroyResource(HANDLE resource);
		HRESULT pfnDrawIndexedPrimitive2(const D3DDDIARG_DRAWINDEXEDPRIMITIVE2* data,
			UINT indicesSize, const void* indexBuffer, const UINT* flagBuffer);
		HRESULT pfnDrawPrimitive(const D3DDDIARG_DRAWPRIMITIVE* data, const UINT* flagBuffer);
		HRESULT pfnFlush();
		HRESULT pfnFlush1(UINT FlushFlags);
		HRESULT pfnLock(D3DDDIARG_LOCK* data);
		HRESULT pfnOpenResource(D3DDDIARG_OPENRESOURCE* data);
		HRESULT pfnPresent(const D3DDDIARG_PRESENT* data);
		HRESULT pfnPresent1(D3DDDIARG_PRESENT1* data);
		HRESULT pfnSetRenderTarget(const D3DDDIARG_SETRENDERTARGET* data);
		HRESULT pfnSetStreamSource(const D3DDDIARG_SETSTREAMSOURCE* data);
		HRESULT pfnSetStreamSourceUm(const D3DDDIARG_SETSTREAMSOURCEUM* data, const void* umBuffer);
		HRESULT pfnUnlock(const D3DDDIARG_UNLOCK* data);

		Adapter& getAdapter() const { return m_adapter; }
		DrawPrimitive& getDrawPrimitive() { return m_drawPrimitive; }
		const D3DDDI_DEVICEFUNCS& getOrigVtable() const { return m_origVtable; }
		Resource* getResource(HANDLE resource);
		DeviceState& getState() { return m_state; }

		HRESULT createPrivateResource(D3DDDIARG_CREATERESOURCE2& data);
		void flushPrimitives() { m_drawPrimitive.flushPrimitives(); }
		void prepareForRendering(HANDLE resource, UINT subResourceIndex, bool isReadOnly);
		void prepareForRendering();

		bool isSrcColorKeySupported() const { return m_isSrcColorKeySupported; }

		static void add(Adapter& adapter, HANDLE device);
		static Device& get(HANDLE device) { return s_devices.find(device)->second; }

		static void enableFlush(bool enable) { s_isFlushEnabled = enable; }
		static Resource* findResource(HANDLE resource);
		static Resource* getGdiResource();
		static void setGdiResourceHandle(HANDLE resource);

	private:
		bool checkSrcColorKeySupport();

		template <typename Arg>
		HRESULT createResourceImpl(Arg& data);

		D3DDDI_DEVICEFUNCS m_origVtable;
		Adapter& m_adapter;
		HANDLE m_device;
		std::unordered_map<HANDLE, Resource> m_resources;
		Resource* m_renderTarget;
		UINT m_renderTargetSubResourceIndex;
		HANDLE m_sharedPrimary;
		DrawPrimitive m_drawPrimitive;
		DeviceState m_state;
		bool m_isSrcColorKeySupported;

		static std::map<HANDLE, Device> s_devices;
		static bool s_isFlushEnabled;
	};
}
