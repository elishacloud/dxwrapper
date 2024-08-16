#pragma once

#ifdef ENABLE_DEBUGOVERLAY

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>
#include <d3d9.h>
#include <ddraw.h>
#include <d3dtypes.h>

class DebugOverlay
{
private:
	bool IsContextSetup = false;

	// Store current device
	LPDIRECT3DDEVICE9 d3d9Device = nullptr;

	// Store debug matrix information
	D3DMATRIX worldMatrix = {}, viewMatrix = {}, projectionMatrix = {};

	// Store debug light information
#pragma pack(push, 4)

	struct LightDebugInfoColor
	{
		uint8_t r = 0, g = 0, b = 0, a = 0;

		LightDebugInfoColor(const D3DCOLORVALUE& color) :
			r((uint8_t)(color.r * 255.0f)),
			g((uint8_t)(color.g * 255.0f)),
			b((uint8_t)(color.b * 255.0f)),
			a((uint8_t)(color.a * 255.0f))
		{}
	};

	struct LightDebugInfo
	{
		char index = -1;
		char type = 0;
		D3DVECTOR position{ 0.0f, 0.0f, 0.0f };
		D3DVECTOR direction{ 0.0f, 0.0f, 0.0f };
		LightDebugInfoColor diffuseColor;
		LightDebugInfoColor specularColor;
		LightDebugInfoColor ambientColor;
	};

#pragma pack(pop)

	std::vector<LightDebugInfo> LightDebugInfos;

public:
	// Initialize
	void Setup(HWND hWnd, LPDIRECT3DDEVICE9 Device);
	void Shutdown();

	// Frame functions
	void BeginScene();
	void EndScene();

	// Functions
	bool IsSetup() { return IsContextSetup; }
	LPDIRECT3DDEVICE9 Getd3d9Device() { return d3d9Device; }
	void SetTransform(D3DTRANSFORMSTATETYPE dtstTransformStateType, LPD3DMATRIX lpD3DMatrix);
	void SetLight(DWORD dwLightIndex, LPD3DLIGHT7 lpLight);
	void LightEnable(DWORD dwLightIndex, BOOL bEnable);
};

#endif
