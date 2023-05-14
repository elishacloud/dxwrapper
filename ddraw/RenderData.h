#pragma once

#include <d3d9types.h>
#include <DirectXMath.h>
#include <vector>

class RenderData
{
public:

	// Store the projection matrix used to transform the geometry on the gpu
	_D3DMATRIX DdrawConvertHomogeneousToWorld_ProjectionMatrix;

	// Store the view matrix used to transform the geometry on the gpu
	_D3DMATRIX DdrawConvertHomogeneousToWorld_ViewMatrix;

	// Store the original view matrix, so we can restore it
	_D3DMATRIX DdrawConvertHomogeneousToWorld_ViewMatrixOriginal;

	// Store the inverse view matrix to transform the geometry on the cpu
	DirectX::XMMATRIX DdrawConvertHomogeneousToWorld_ViewMatrixInverse;

	// Intermediate buffer for the geometry conversion
	std::vector<uint8_t> DdrawConvertHomogeneousToWorld_IntermediateGeometry;

	RenderData()
	{
		ZeroMemory(&DdrawConvertHomogeneousToWorld_ViewMatrix, sizeof(_D3DMATRIX));
		DdrawConvertHomogeneousToWorld_ViewMatrix._11 = 1.0f;
		DdrawConvertHomogeneousToWorld_ViewMatrix._22 = 1.0f;
		DdrawConvertHomogeneousToWorld_ViewMatrix._33 = 1.0f;
		DdrawConvertHomogeneousToWorld_ViewMatrix._44 = 1.0f;

		std::memcpy(&DdrawConvertHomogeneousToWorld_ProjectionMatrix, &DdrawConvertHomogeneousToWorld_ViewMatrix, sizeof(_D3DMATRIX));
		std::memcpy(&DdrawConvertHomogeneousToWorld_ViewMatrixOriginal, &DdrawConvertHomogeneousToWorld_ViewMatrix, sizeof(_D3DMATRIX));
	}
};
