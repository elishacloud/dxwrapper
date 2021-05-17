#pragma once

#include <d3d.h>
#include <d3dumddi.h>

#include <DDrawCompat/v0.3.1/Common/VtableVisitor.h>

template <>
struct VtableForEach<D3DDDI_DEVICEFUNCS>
{
	template <typename Vtable, typename Visitor>
	static void forEach(Visitor& visitor, UINT version)
	{
		DD_VISIT(pfnSetRenderState);
		DD_VISIT(pfnUpdateWInfo);
		DD_VISIT(pfnValidateDevice);
		DD_VISIT(pfnSetTextureStageState);
		DD_VISIT(pfnSetTexture);
		DD_VISIT(pfnSetPixelShader);
		DD_VISIT(pfnSetPixelShaderConst);
		DD_VISIT(pfnSetStreamSourceUm);
		DD_VISIT(pfnSetIndices);
		DD_VISIT(pfnSetIndicesUm);
		DD_VISIT(pfnDrawPrimitive);
		DD_VISIT(pfnDrawIndexedPrimitive);
		DD_VISIT(pfnDrawRectPatch);
		DD_VISIT(pfnDrawTriPatch);
		DD_VISIT(pfnDrawPrimitive2);
		DD_VISIT(pfnDrawIndexedPrimitive2);
		DD_VISIT(pfnVolBlt);
		DD_VISIT(pfnBufBlt);
		DD_VISIT(pfnTexBlt);
		DD_VISIT(pfnStateSet);
		DD_VISIT(pfnSetPriority);
		DD_VISIT(pfnClear);
		DD_VISIT(pfnUpdatePalette);
		DD_VISIT(pfnSetPalette);
		DD_VISIT(pfnSetVertexShaderConst);
		DD_VISIT(pfnMultiplyTransform);
		DD_VISIT(pfnSetTransform);
		DD_VISIT(pfnSetViewport);
		DD_VISIT(pfnSetZRange);
		DD_VISIT(pfnSetMaterial);
		DD_VISIT(pfnSetLight);
		DD_VISIT(pfnCreateLight);
		DD_VISIT(pfnDestroyLight);
		DD_VISIT(pfnSetClipPlane);
		DD_VISIT(pfnGetInfo);
		DD_VISIT(pfnLock);
		DD_VISIT(pfnUnlock);
		DD_VISIT(pfnCreateResource);
		DD_VISIT(pfnDestroyResource);
		DD_VISIT(pfnSetDisplayMode);
		DD_VISIT(pfnPresent);
		DD_VISIT(pfnFlush);
		DD_VISIT(pfnCreateVertexShaderFunc);
		DD_VISIT(pfnDeleteVertexShaderFunc);
		DD_VISIT(pfnSetVertexShaderFunc);
		DD_VISIT(pfnCreateVertexShaderDecl);
		DD_VISIT(pfnDeleteVertexShaderDecl);
		DD_VISIT(pfnSetVertexShaderDecl);
		DD_VISIT(pfnSetVertexShaderConstI);
		DD_VISIT(pfnSetVertexShaderConstB);
		DD_VISIT(pfnSetScissorRect);
		DD_VISIT(pfnSetStreamSource);
		DD_VISIT(pfnSetStreamSourceFreq);
		DD_VISIT(pfnSetConvolutionKernelMono);
		DD_VISIT(pfnComposeRects);
		DD_VISIT(pfnBlt);
		DD_VISIT(pfnColorFill);
		DD_VISIT(pfnDepthFill);
		DD_VISIT(pfnCreateQuery);
		DD_VISIT(pfnDestroyQuery);
		DD_VISIT(pfnIssueQuery);
		DD_VISIT(pfnGetQueryData);
		DD_VISIT(pfnSetRenderTarget);
		DD_VISIT(pfnSetDepthStencil);
		DD_VISIT(pfnGenerateMipSubLevels);
		DD_VISIT(pfnSetPixelShaderConstI);
		DD_VISIT(pfnSetPixelShaderConstB);
		DD_VISIT(pfnCreatePixelShader);
		DD_VISIT(pfnDeletePixelShader);
		DD_VISIT(pfnCreateDecodeDevice);
		DD_VISIT(pfnDestroyDecodeDevice);
		DD_VISIT(pfnSetDecodeRenderTarget);
		DD_VISIT(pfnDecodeBeginFrame);
		DD_VISIT(pfnDecodeEndFrame);
		DD_VISIT(pfnDecodeExecute);
		DD_VISIT(pfnDecodeExtensionExecute);
		DD_VISIT(pfnCreateVideoProcessDevice);
		DD_VISIT(pfnDestroyVideoProcessDevice);
		DD_VISIT(pfnVideoProcessBeginFrame);
		DD_VISIT(pfnVideoProcessEndFrame);
		DD_VISIT(pfnSetVideoProcessRenderTarget);
		DD_VISIT(pfnVideoProcessBlt);
		DD_VISIT(pfnCreateExtensionDevice);
		DD_VISIT(pfnDestroyExtensionDevice);
		DD_VISIT(pfnExtensionExecute);
		DD_VISIT(pfnCreateOverlay);
		DD_VISIT(pfnUpdateOverlay);
		DD_VISIT(pfnFlipOverlay);
		DD_VISIT(pfnGetOverlayColorControls);
		DD_VISIT(pfnSetOverlayColorControls);
		DD_VISIT(pfnDestroyOverlay);
		DD_VISIT(pfnDestroyDevice);
		DD_VISIT(pfnQueryResourceResidency);
		DD_VISIT(pfnOpenResource);
		DD_VISIT(pfnGetCaptureAllocationHandle);
		DD_VISIT(pfnCaptureToSysMem);
		DD_VISIT(pfnLockAsync);
		DD_VISIT(pfnUnlockAsync);
		DD_VISIT(pfnRename);

		if (version >= D3D_UMD_INTERFACE_VERSION_WIN7)
		{
			DD_VISIT(pfnCreateVideoProcessor);
			DD_VISIT(pfnSetVideoProcessBltState);
			DD_VISIT(pfnGetVideoProcessBltStatePrivate);
			DD_VISIT(pfnSetVideoProcessStreamState);
			DD_VISIT(pfnGetVideoProcessStreamStatePrivate);
			DD_VISIT(pfnVideoProcessBltHD);
			DD_VISIT(pfnDestroyVideoProcessor);
			DD_VISIT(pfnCreateAuthenticatedChannel);
			DD_VISIT(pfnAuthenticatedChannelKeyExchange);
			DD_VISIT(pfnQueryAuthenticatedChannel);
			DD_VISIT(pfnConfigureAuthenticatedChannel);
			DD_VISIT(pfnDestroyAuthenticatedChannel);
			DD_VISIT(pfnCreateCryptoSession);
			DD_VISIT(pfnCryptoSessionKeyExchange);
			DD_VISIT(pfnDestroyCryptoSession);
			DD_VISIT(pfnEncryptionBlt);
			DD_VISIT(pfnGetPitch);
			DD_VISIT(pfnStartSessionKeyRefresh);
			DD_VISIT(pfnFinishSessionKeyRefresh);
			DD_VISIT(pfnGetEncryptionBltKey);
			DD_VISIT(pfnDecryptionBlt);
			DD_VISIT(pfnResolveSharedResource);
		}

		if (version >= D3D_UMD_INTERFACE_VERSION_WIN8)
		{
			DD_VISIT(pfnVolBlt1);
			DD_VISIT(pfnBufBlt1);
			DD_VISIT(pfnTexBlt1);
			DD_VISIT(pfnDiscard);
			DD_VISIT(pfnOfferResources);
			DD_VISIT(pfnReclaimResources);
			DD_VISIT(pfnCheckDirectFlipSupport);
			DD_VISIT(pfnCreateResource2);
			DD_VISIT(pfnCheckMultiPlaneOverlaySupport);
			DD_VISIT(pfnPresentMultiPlaneOverlay);
		}

		if (version >= D3D_UMD_INTERFACE_VERSION_WDDM1_3)
		{
			// DD_VISIT(pfnReserved1);
			DD_VISIT(pfnFlush1);
			DD_VISIT(pfnCheckCounterInfo);
			DD_VISIT(pfnCheckCounter);
			DD_VISIT(pfnUpdateSubresourceUP);
			DD_VISIT(pfnPresent1);
			DD_VISIT(pfnCheckPresentDurationSupport);

			DD_VISIT(pfnSetMarker);
			DD_VISIT(pfnSetMarkerMode);
		}

		if (version >= D3D_UMD_INTERFACE_VERSION_WDDM2_0)
		{
			DD_VISIT(pfnTrimResidencySet);
		}

		if (version >= D3D_UMD_INTERFACE_VERSION_WDDM2_1_2)
		{
			DD_VISIT(pfnAcquireResource);
			DD_VISIT(pfnReleaseResource);
		}
	}
};
