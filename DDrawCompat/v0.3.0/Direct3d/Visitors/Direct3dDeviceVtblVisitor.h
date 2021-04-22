#pragma once

#include <d3d.h>

#include <Common/VtableVisitor.h>

template <>
struct VtableForEach<IDirect3DDeviceVtbl>
{
	template <typename Vtable, typename Visitor>
	static void forEach(Visitor& visitor, UINT version)
	{
		VtableForEach<IUnknownVtbl>::forEach<Vtable>(visitor, version);

		DD_VISIT(Initialize);
		DD_VISIT(GetCaps);
		DD_VISIT(SwapTextureHandles);
		DD_VISIT(CreateExecuteBuffer);
		DD_VISIT(GetStats);
		DD_VISIT(Execute);
		DD_VISIT(AddViewport);
		DD_VISIT(DeleteViewport);
		DD_VISIT(NextViewport);
		DD_VISIT(Pick);
		DD_VISIT(GetPickRecords);
		DD_VISIT(EnumTextureFormats);
		DD_VISIT(CreateMatrix);
		DD_VISIT(SetMatrix);
		DD_VISIT(GetMatrix);
		DD_VISIT(DeleteMatrix);
		DD_VISIT(BeginScene);
		DD_VISIT(EndScene);
		DD_VISIT(GetDirect3D);
	}
};

template <>
struct VtableForEach<IDirect3DDevice2Vtbl>
{
	template <typename Vtable, typename Visitor>
	static void forEach(Visitor& visitor, UINT version)
	{
		VtableForEach<IUnknownVtbl>::forEach<Vtable>(visitor, version);

		DD_VISIT(GetCaps);
		DD_VISIT(SwapTextureHandles);
		DD_VISIT(GetStats);
		DD_VISIT(AddViewport);
		DD_VISIT(DeleteViewport);
		DD_VISIT(NextViewport);
		DD_VISIT(EnumTextureFormats);
		DD_VISIT(BeginScene);
		DD_VISIT(EndScene);
		DD_VISIT(GetDirect3D);
		DD_VISIT(SetCurrentViewport);
		DD_VISIT(GetCurrentViewport);
		DD_VISIT(SetRenderTarget);
		DD_VISIT(GetRenderTarget);
		DD_VISIT(Begin);
		DD_VISIT(BeginIndexed);
		DD_VISIT(Vertex);
		DD_VISIT(Index);
		DD_VISIT(End);
		DD_VISIT(GetRenderState);
		DD_VISIT(SetRenderState);
		DD_VISIT(GetLightState);
		DD_VISIT(SetLightState);
		DD_VISIT(SetTransform);
		DD_VISIT(GetTransform);
		DD_VISIT(MultiplyTransform);
		DD_VISIT(DrawPrimitive);
		DD_VISIT(DrawIndexedPrimitive);
		DD_VISIT(SetClipStatus);
		DD_VISIT(GetClipStatus);
	}
};

template <>
struct VtableForEach<IDirect3DDevice3Vtbl>
{
	template <typename Vtable, typename Visitor>
	static void forEach(Visitor& visitor, UINT version)
	{
		VtableForEach<IUnknownVtbl>::forEach<Vtable>(visitor, version);

		DD_VISIT(GetCaps);
		DD_VISIT(GetStats);
		DD_VISIT(AddViewport);
		DD_VISIT(DeleteViewport);
		DD_VISIT(NextViewport);
		DD_VISIT(EnumTextureFormats);
		DD_VISIT(BeginScene);
		DD_VISIT(EndScene);
		DD_VISIT(GetDirect3D);
		DD_VISIT(SetCurrentViewport);
		DD_VISIT(GetCurrentViewport);
		DD_VISIT(SetRenderTarget);
		DD_VISIT(GetRenderTarget);
		DD_VISIT(Begin);
		DD_VISIT(BeginIndexed);
		DD_VISIT(Vertex);
		DD_VISIT(Index);
		DD_VISIT(End);
		DD_VISIT(GetRenderState);
		DD_VISIT(SetRenderState);
		DD_VISIT(GetLightState);
		DD_VISIT(SetLightState);
		DD_VISIT(SetTransform);
		DD_VISIT(GetTransform);
		DD_VISIT(MultiplyTransform);
		DD_VISIT(DrawPrimitive);
		DD_VISIT(DrawIndexedPrimitive);
		DD_VISIT(SetClipStatus);
		DD_VISIT(GetClipStatus);
		DD_VISIT(DrawPrimitiveStrided);
		DD_VISIT(DrawIndexedPrimitiveStrided);
		DD_VISIT(DrawPrimitiveVB);
		DD_VISIT(DrawIndexedPrimitiveVB);
		DD_VISIT(ComputeSphereVisibility);
		DD_VISIT(GetTexture);
		DD_VISIT(SetTexture);
		DD_VISIT(GetTextureStageState);
		DD_VISIT(SetTextureStageState);
		DD_VISIT(ValidateDevice);
	}
};

template <>
struct VtableForEach<IDirect3DDevice7Vtbl>
{
	template <typename Vtable, typename Visitor>
	static void forEach(Visitor& visitor, UINT version)
	{
		VtableForEach<IUnknownVtbl>::forEach<Vtable>(visitor, version);

		DD_VISIT(GetCaps);
		DD_VISIT(EnumTextureFormats);
		DD_VISIT(BeginScene);
		DD_VISIT(EndScene);
		DD_VISIT(GetDirect3D);
		DD_VISIT(SetRenderTarget);
		DD_VISIT(GetRenderTarget);
		DD_VISIT(Clear);
		DD_VISIT(SetTransform);
		DD_VISIT(GetTransform);
		DD_VISIT(SetViewport);
		DD_VISIT(MultiplyTransform);
		DD_VISIT(GetViewport);
		DD_VISIT(SetMaterial);
		DD_VISIT(GetMaterial);
		DD_VISIT(SetLight);
		DD_VISIT(GetLight);
		DD_VISIT(SetRenderState);
		DD_VISIT(GetRenderState);
		DD_VISIT(BeginStateBlock);
		DD_VISIT(EndStateBlock);
		DD_VISIT(PreLoad);
		DD_VISIT(DrawPrimitive);
		DD_VISIT(DrawIndexedPrimitive);
		DD_VISIT(SetClipStatus);
		DD_VISIT(GetClipStatus);
		DD_VISIT(DrawPrimitiveStrided);
		DD_VISIT(DrawIndexedPrimitiveStrided);
		DD_VISIT(DrawPrimitiveVB);
		DD_VISIT(DrawIndexedPrimitiveVB);
		DD_VISIT(ComputeSphereVisibility);
		DD_VISIT(GetTexture);
		DD_VISIT(SetTexture);
		DD_VISIT(GetTextureStageState);
		DD_VISIT(SetTextureStageState);
		DD_VISIT(ValidateDevice);
		DD_VISIT(ApplyStateBlock);
		DD_VISIT(CaptureStateBlock);
		DD_VISIT(DeleteStateBlock);
		DD_VISIT(CreateStateBlock);
		DD_VISIT(Load);
		DD_VISIT(LightEnable);
		DD_VISIT(GetLightEnable);
		DD_VISIT(SetClipPlane);
		DD_VISIT(GetClipPlane);
		DD_VISIT(GetInfo);
	}
};
