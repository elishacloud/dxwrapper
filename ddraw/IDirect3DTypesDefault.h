#pragma once

namespace {
	// 0xFFFFFFFF
	constexpr DWORD Z = (DWORD)-1;
	// 1.0f
	constexpr DWORD I = 0x3F800000;
}

constexpr DWORD RenderStateDefault[MaxDeviceStates] = {
	Z, Z, Z, Z, 1, 1, 1, 1, 3, 2,
	0, 0, 0, 0, 1, 0, 1, 1, 1, 2,
	1, 1, 3, 4, 0, 8, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, I, I, I,
	Z, 0, Z, Z, Z, Z, Z, 0, 0, 0,
	Z, Z, 0, 1, 1, 1, 8, 0, Z, Z,
	Z, Z, Z, Z, Z, Z, Z, Z, Z, Z,
	Z, Z, Z, Z, Z, Z, Z, Z, Z, Z,
	Z, Z, Z, Z, Z, Z, Z, Z, Z, Z,
	Z, Z, Z, Z, Z, Z, Z, Z, Z, Z,
	Z, Z, Z, Z, Z, Z, Z, Z, Z, Z,
	Z, Z, Z, Z, Z, Z, Z, Z, Z, Z,
	Z, Z, Z, Z, Z, Z, Z, Z, 0, 0,
	0, 0, 0, 0, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 0, 1, 2, 0, 0, Z,
	Z, 0, 0, Z, Z, Z, Z, Z, Z, Z,
	Z, Z, Z, Z, Z, Z, Z, Z, Z, Z,
	Z, Z, Z, Z, Z, Z, Z, Z, Z, Z,
	Z, Z, Z, Z, Z, Z, Z, Z, Z, Z,
	Z, Z, Z, Z, Z, Z, Z, Z, Z, Z,
	Z, Z, Z, Z, Z, Z, Z, Z, Z, Z,
	Z, Z, Z, Z, Z, Z, Z, Z, Z, Z,
	Z, Z, Z, Z, Z, Z, Z, Z, Z, Z,
	Z, Z, Z, Z, Z, Z, Z, Z, Z, Z,
	Z, Z, Z, Z, Z, Z, Z, Z, Z, Z,
	Z, Z, Z, Z, Z, Z,
};

constexpr DWORD TextureStageStateDefault[MaxTextureStages][MaxTextureStageStates] = {
	{ Z, 4, 2, 1, 2, 2, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, Z, Z, Z, Z, Z, Z, Z, Z, },
	{ Z, 1, 2, 1, 1, 2, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, Z, Z, Z, Z, Z, Z, Z, Z, },
	{ Z, 1, 2, 1, 1, 2, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, Z, Z, Z, Z, Z, Z, Z, Z, },
	{ Z, 1, 2, 1, 1, 2, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, Z, Z, Z, Z, Z, Z, Z, Z, },
	{ Z, 1, 2, 1, 1, 2, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, Z, Z, Z, Z, Z, Z, Z, Z, },
	{ Z, 1, 2, 1, 1, 2, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, Z, Z, Z, Z, Z, Z, Z, Z, },
	{ Z, 1, 2, 1, 1, 2, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, Z, Z, Z, Z, Z, Z, Z, Z, },
	{ Z, 1, 2, 1, 1, 2, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, Z, Z, Z, Z, Z, Z, Z, Z, }
};

constexpr DWORD SamplerStateDefault[MaxTextureStages][MaxSamplerStates] = {
	{ Z, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, },
	{ Z, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, },
	{ Z, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, },
	{ Z, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, },
	{ Z, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, },
	{ Z, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, },
	{ Z, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, },
	{ Z, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, },
};

constexpr D3DCLIPSTATUS ClipStatusDefault {
	D3DCLIPSTATUS_STATUS, D3DSTATUS_DEFAULT, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
};

constexpr float ClipPlaneDefault[4] {
	0.0f, 0.0f, 0.0f, 0.0f
};

constexpr D3DMATERIAL9 MaterialDefault = {
	{ 0.0f, 0.0f, 0.0f, 1.0f }, // Diffuse (RGBA)
	{ 0.0f, 0.0f, 0.0f, 1.0f }, // Ambient (RGBA)
	{ 0.0f, 0.0f, 0.0f, 0.0f }, // Specular (RGBA)
	{ 0.0f, 0.0f, 0.0f, 0.0f }, // Emissive (RGBA)
	0.0f                        // Power (specular exponent)
};

constexpr D3DMATRIX IdentityMatrixDefault = {
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f,
};
