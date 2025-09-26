#pragma once

constexpr UINT MaxLightStates = 9;			// Devices have up to 8+1 light states.
constexpr UINT MaxTextureStageStates = 33;  // Devices have up to 32+1 texture stage states.
constexpr UINT MaxClipPlaneIndex = 6;       // Devices can have up to 6 clip planes.
constexpr UINT MaxActiveLights = 32;        // Devices can have up to 32 lights.

namespace {
	// 0xFFFFFFFF
	constexpr DWORD Z = (DWORD)-1;
	// 1.0f
	constexpr DWORD I = 0x3F800000;
	// 256.0f
	constexpr DWORD J = 0x43800000;
	// 15
	constexpr DWORD F = 15;
}

constexpr DWORD DefaultRenderState[D3D_MAXRENDERSTATES] = {
	Z, 0, 0, 0, 0, 0, 0, 1, 3, 2,
	0, 0, 0, 0, 1, 0, 1, 0, 0, 2,
	1, 0, 3, 4, 0, 8, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, I, I, 0,
	0, 0, Z, 0, 0, 0, 0, Z, 0, 0,
	Z, 0, 0, 1, 1, 1, 8, 0, Z, Z,
	Z, Z, Z, Z, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, Z, Z, Z, Z,
	Z, Z, Z, Z, Z, Z, Z, Z, Z, Z,
	Z, Z, Z, Z, Z, Z, Z, Z, Z, Z,
	Z, Z, Z, Z, Z, Z, Z, Z, 0, 0,
	0, 0, 0, 0, 0, 0, 1, 1, Z, 0,
	0, 1, 1, 0, 0, 1, 2, 0, 0, Z,
	Z, 0, 0, 0, I, I, 0, 0, I, 0,
	0, 1, Z, 0, 0, Z, J, 0, F, Z,
	0, 1, 3, 1, 0, 0, 0, Z, I, I,
	0, 0, I, 0, 0, 0, 1, 1, 1, 8,
	F, F, F, Z, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 2, 1, 1,
	Z, Z, Z, Z, Z, Z, Z, Z, Z, Z,
	Z, Z, Z, Z, Z, Z, Z, Z, Z, Z,
	Z, Z, Z, Z, Z, Z, Z, Z, Z, Z,
	Z, Z, Z, Z, Z, Z, Z, Z, Z, Z,
	Z, Z, Z, Z, Z, Z,
};

constexpr DWORD DefaultTextureStageState[D3DHAL_TSS_MAXSTAGES][MaxTextureStageStates] = {
	{ Z, 4, 2, 1, 2, 2, 1, 0, 0, 0, 0, 0, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, 0, 0, 0, Z, 1, 1, 1, Z, Z, Z, 0, },
	{ Z, 1, 2, 1, 1, 2, 1, 0, 0, 0, 0, 1, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, 0, 0, 0, Z, 1, 1, 1, Z, Z, Z, 0, },
	{ Z, 1, 2, 1, 1, 2, 1, 0, 0, 0, 0, 2, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, 0, 0, 0, Z, 1, 1, 1, Z, Z, Z, 0, },
	{ Z, 1, 2, 1, 1, 2, 1, 0, 0, 0, 0, 3, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, 0, 0, 0, Z, 1, 1, 1, Z, Z, Z, 0, },
	{ Z, 1, 2, 1, 1, 2, 1, 0, 0, 0, 0, 4, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, 0, 0, 0, Z, 1, 1, 1, Z, Z, Z, 0, },
	{ Z, 1, 2, 1, 1, 2, 1, 0, 0, 0, 0, 5, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, 0, 0, 0, Z, 1, 1, 1, Z, Z, Z, 0, },
	{ Z, 1, 2, 1, 1, 2, 1, 0, 0, 0, 0, 6, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, 0, 0, 0, Z, 1, 1, 1, Z, Z, Z, 0, },
	{ Z, 1, 2, 1, 1, 2, 1, 0, 0, 0, 0, 7, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, 0, 0, 0, Z, 1, 1, 1, Z, Z, Z, 0, },
};

constexpr DWORD DefaultSamplerState[D3DHAL_TSS_MAXSTAGES][D3DHAL_TEXTURESTATEBUF_SIZE] = {
	{ Z, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, },
	{ Z, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, },
	{ Z, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, },
	{ Z, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, },
	{ Z, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, },
	{ Z, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, },
	{ Z, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, },
	{ Z, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, },
};

constexpr D3DCLIPSTATUS DefaultClipStatus {
	D3DCLIPSTATUS_STATUS, D3DSTATUS_DEFAULT, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
};

constexpr float DefaultClipPlane[4] {
	0.0f, 0.0f, 0.0f, 0.0f
};

constexpr D3DMATERIAL9 DefaultMaterial = {
	{ 0.0f, 0.0f, 0.0f, 0.0f },
	{ 0.0f, 0.0f, 0.0f, 0.0f },
	{ 0.0f, 0.0f, 0.0f, 0.0f },
	{ 0.0f, 0.0f, 0.0f, 0.0f },
	0.0f
};

constexpr D3DMATRIX DefaultIdentityMatrix = {
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f,
};
