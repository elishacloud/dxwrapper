#pragma once

#include "ddraw-testing.h"

#define LOG_TEST_RESULT(TestID, LogEntry, TestValue, ExpectedResult) \
	{ \
		DWORD Value = (DWORD)TestValue; \
		DWORD Result = (DWORD)ExpectedResult; \
		if (Value == Result && Value != TEST_FAILED && Result != TEST_FAILED) \
		{ \
				Logging::Log() << "Testing: " << TestID << " Succeded! " << LogEntry << ExpectedResult << " -> " << TestValue; \
		} \
		else \
		{ \
				Logging::Log() << "Testing: " << TestID << " FAILED! " << LogEntry << ExpectedResult << " -> " << TestValue; \
		} \
	}

constexpr UINT MaxLightStates = 10;  // Devices have up to 10 types.
constexpr UINT MaxTextureStageStates = 33;  // Devices have up to 33 types.

namespace {
	// 0xFFFFFFFF
	constexpr DWORD Z = (DWORD)-1;
	// 1.0f
	constexpr DWORD I = 0x3F800000;
}

// Defaults for native DirectDraw
constexpr DWORD DefaultLightStateDX5[MaxLightStates] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
constexpr DWORD DefaultLightStateDX6[MaxLightStates] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 0 };

constexpr DWORD UnchangeableRenderTarget[] = { 0, 50 };
constexpr DWORD UnchangeableRenderTargetDX7[] = { 1, 3, 5, 6, 11, 12, 13, 17, 18, 21, 31, 32, 39, 43, 44, 45, 46, 49, 51, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95 };
constexpr DWORD DefaultRenderTargetDX5[D3D_MAXRENDERSTATES] = { 0, 0, 0, 1, 0, 0, 0, 1, 3, 2, 0, 0, 13, Z, 1, 0, 1, 1, 1, 2, 1, 2, 3, 4, 0, 8, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, I, I, 0, 0, 0, Z, 0, 1, 1, 0, 0, 0, 1, Z, Z, 0, 1, 1, 1, 8, 0, Z, Z, Z, Z, Z, Z, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, 0, 0, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z };
constexpr DWORD DefaultRenderTargetDX6[D3D_MAXRENDERSTATES] = { 0, 0, 0, 1, 1, 0, 0, 1, 3, 2, 0, 0, 13, Z, 1, 0, 1, 1, 1, 2, 1, 2, 3, 4, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, I, I, 0, 0, 0, Z, 0, 1, 1, 0, 0, 0, 1, Z, Z, 0, 1, 1, 1, 8, 0, Z, Z, Z, Z, Z, Z, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, 0, 0, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z };
constexpr DWORD DefaultRenderTargetDX7[D3D_MAXRENDERSTATES] = { Z, 0, Z, 0, 1, 0, 0, 1, 3, 2, 0, 0,  0, 0, 1, 0, 1, 0, 0, 2, 1, 0, 3, 4, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, I, I, 0, Z, 0, Z, 0, 0, 0, 0, 0, 0, 0, Z, 0, 0, 1, 1, 1, 8, 0, Z, Z, Z, Z, Z, Z, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 2, 0, 0, Z, Z, 0, 0, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z };

constexpr DWORD DefaultTextureStageStateDX6[D3DHAL_TSS_MAXSTAGES][MaxTextureStageStates] = {
	{ Z, 4, 2, 1, 2, 2, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, Z, Z, Z, Z, Z, Z, Z, Z, Z },
	{ Z, 1, 2, 1, 1, 2, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, Z, Z, Z, Z, Z, Z, Z, Z, Z },
	{ Z, 1, 2, 1, 1, 2, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, Z, Z, Z, Z, Z, Z, Z, Z, Z },
	{ Z, 1, 2, 1, 1, 2, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, Z, Z, Z, Z, Z, Z, Z, Z, Z },
	{ Z, 1, 2, 1, 1, 2, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, Z, Z, Z, Z, Z, Z, Z, Z, Z },
	{ Z, 1, 2, 1, 1, 2, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, Z, Z, Z, Z, Z, Z, Z, Z, Z },
	{ Z, 1, 2, 1, 1, 2, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, Z, Z, Z, Z, Z, Z, Z, Z, Z },
	{ Z, 1, 2, 1, 1, 2, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, Z, Z, Z, Z, Z, Z, Z, Z, Z }};

constexpr DWORD DefaultTextureStageStateDX7[D3DHAL_TSS_MAXSTAGES][MaxTextureStageStates] = {
	{ Z, 4, 2, 1, 2, 2, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, Z, Z, Z, Z, Z, Z, Z, Z },
	{ Z, 1, 2, 1, 1, 2, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, Z, Z, Z, Z, Z, Z, Z, Z },
	{ Z, 1, 2, 1, 1, 2, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, Z, Z, Z, Z, Z, Z, Z, Z },
	{ Z, 1, 2, 1, 1, 2, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, Z, Z, Z, Z, Z, Z, Z, Z },
	{ Z, 1, 2, 1, 1, 2, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, Z, Z, Z, Z, Z, Z, Z, Z },
	{ Z, 1, 2, 1, 1, 2, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, Z, Z, Z, Z, Z, Z, Z, Z },
	{ Z, 1, 2, 1, 1, 2, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, Z, Z, Z, Z, Z, Z, Z, Z },
	{ Z, 1, 2, 1, 1, 2, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, Z, Z, Z, Z, Z, Z, Z, Z }};

constexpr DWORD vDDraw1 = 0;
constexpr DWORD vDDraw2 = 1;
constexpr DWORD vDDraw3 = 2;
constexpr DWORD vDDraw4 = 3;
constexpr DWORD vDDraw7 = 4;
constexpr DWORD vDDraw7Ex = 5;

constexpr DWORD TEST_FAILED = (DWORD)-1;

struct TESTMATRIX {
	DWORD TestID;
	DWORD Result[6];
};

constexpr TESTMATRIX TestResults[] = {
	// IDirectDraw
	{ 100, { 1, 1, 1, 1, 1, 1 } },
	{ 101, { 2, 1, (DWORD)E_NOINTERFACE, 1, 1, 2 } },
	{ 102, { 2, 1, 0, 1, 1, 2 } },
	{ 103, { 1, 0, 0, 0, 0, 1 } },
	{ 104, { 1, 1, 0, 1, 1, 1 } },
	{ 105, { 0, 0, 0, 0, 0, 0 } },
	{ 106, { 0, 0, 0, 0, 0, 0 } },

	// IDirectDrawSurface
	{ 200, { 1, 1, 1, 1, 1, 1 } },
	{ 201, { 1, 1, 1, 2, 2, 2 } },
	{ 202, { 1, 1, 1, 1, 1, 1 } },
	{ 203, { 1, 1, 1, 3, 3, 4 } },
	{ 204, { 1, 1, 1, 2, 2, 2 } },
	{ 205, { 1, 1, 1, 1, 1, 1 } },
	{ 206, { 1, 1, 1, 1, 1, 1 } },
	{ 207, { 1, 1, 1, 3, 3, 3 } },
	{ 208, { 1, 1, 1, 1, 1, 1 } },
	{ 209, { 1, 1, 1, 3, 3, 3 } },
	{ 210, { 2, 1, 1, 1, 1, 1 } },
	{ 211, { 1, 2, 1, 1, 1, 1 } },
	{ 212, { 1, 1, 2, 1, 1, 1 } },
	{ 213, { 1, 1, 1, 2, 1, 1 } },
	{ 214, { 1, 1, 1, 1, 2, 2 } },
	{ 215, { 2, 1, 1, 1, 1, 1 } },
	{ 216, { 1, 2, 1, 1, 1, 1 } },
	{ 217, { 1, 1, 2, 1, 1, 1 } },
	{ 218, { 1, 1, 1, 2, 1, 1 } },
	{ 219, { 1, 1, 1, 1, 2, 2 } },
	{ 220, { 1, 1, 1, 2, 2, 2 } },
	{ 221, { 1, 1, 1, 2, 2, 2 } },
	{ 222, { 1, 1, 1, 2, 2, 2 } },
	{ 223, { 1, 1, 1, 2, 2, 2 } },
	{ 224, { 1, 1, 1, 2, 2, 2 } },
	{ 225, { (DWORD)DDERR_CANTDUPLICATE, (DWORD)DDERR_CANTDUPLICATE, (DWORD)DDERR_CANTDUPLICATE, (DWORD)DDERR_CANTDUPLICATE, (DWORD)DDERR_CANTDUPLICATE, (DWORD)DDERR_CANTDUPLICATE } },
	{ 226, { 1, 1, 1, 1, 1, 1 } },
	{ 227, { 1, 1, 1, 3, 3, 4 } },
	{ 228, { 2, 2, 2, 2, 2, 2 } },
	{ 229, { 1, 1, 1, 1, 1, 1 } },
	{ 230, { 2, 2, 2, 2, 2, 2 } },
	{ 231, { 2, 2, 2, 2, 2, 2 } },
	{ 232, { 1, 1, 1, 1, 1, 1 } },
	{ 233, { 268485176, 268485176, 0, 268485176, 268485176, 268485176 } },
	{ 234, { 268451868, 268451868, 0, 268451868, 268451868, 268451868 } },

	// IDirect3DTexture
	{ 300, { 2, 2, 2, 2, 2, (DWORD)E_NOINTERFACE } },
	{ 301, { 2, 2, 2, 2, 2, 0 } },
	{ 302, { 1, 1, 1, 3, 3, 0 } },
	{ 303, { 3, 3, 3, 3, 3, 0 } },
	{ 304, { 3, 3, 3, 3, 3, 0 } },
	{ 305, { 1, 1, 1, 3, 3, 0 } },
	{ 306, { 3, 3, 3, 3, 3, 0 } },
	{ 307, { 3, 3, 3, 3, 3, 0 } },
	{ 308, { 1, 1, 1, 3, 3, 0 } },
	{ 309, { 3, 3, 3, 3, 3, 0 } },
	{ 310, { 3, 3, 3, 3, 3, 0 } },
	{ 311, { 3, 3, 3, 3, 3, 0 } },
	{ 312, { 1, 1, 1, 3, 3, 0 } },
	{ 313, { 4, 4, 4, 4, 4, 0 } },
	{ 314, { 4, 4, 4, 4, 4, 0 } },
	{ 315, { 4, 4, 4, 4, 4, 0 } },
	{ 316, { 1, 1, 1, 3, 3, 0 } },

	// IDirectDrawGammaControl / IDirectDrawColorControl
	{ 400, { 1, 1, 1, 1, 1, 1 } },
	{ 401, { 1, 1, 1, 1, 1, 1 } },
	{ 402, { 1, 1, 1, 2, 2, 2 } },
	{ 403, { (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE } },
	{ 405, { 1, 1, 1, 1, 1, 1 } },
	{ 406, { 1, 1, 1, 1, 1, 1 } },
	{ 407, { 1, 1, 1, 3, 3, 4 } },
	{ 408, { (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE } },
	{ 410, { 1, 1, 1, 1, 1, 1 } },
	{ 411, { 2, 2, 1, 1, 1, 1 } },
	{ 412, { 1, 1, 1, 2, 2, 2 } },
	{ 413, { (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE } },
	{ 415, { 1, 1, 1, 1, 1, 1 } },
	{ 416, { 1, 1, 1, 1, 1, 1 } },
	{ 417, { 1, 1, 1, 2, 2, 2 } },
	{ 418, { (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE } },
	{ 420, { 1, 1, 1, 1, 1, 1 } },
	{ 421, { 1, 1, 1, 1, 1, 1 } },
	{ 422, { 1, 1, 1, 2, 2, 2 } },
	{ 423, { (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE } },
	{ 425, { 1, 1, 1, 1, 1, 1 } },
	{ 426, { 1, 1, 1, 2, 1, 1 } },
	{ 427, { 1, 1, 1, 2, 2, 2 } },
	{ 428, { (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE } },
	{ 430, { 1, 1, 1, 1, 1, 1 } },
	{ 431, { 1, 1, 1, 1, 2, 2 } },
	{ 432, { 1, 1, 1, 2, 2, 2 } },
	{ 433, { (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE } },
	{ 450, { (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE } },

	// IDirect3D
	{ 500, { 2, 2, 2, 2, (DWORD)E_NOINTERFACE, 2 } },
	{ 501, { 2, 2, 2, 2, 2, 2 } },
	{ 502, { 3, 3, 3, 3, 3, 3 } },
	{ 503, { 3, 3, 3, 3, 3, 3 } },
	{ 504, { 3, 3, 3, 3, 3, 3 } },
	{ 505, { 3, 3, 3, 3, 3, 3 } },
	{ 506, { 3, 3, 3, 3, 3, (DWORD)E_NOINTERFACE } },
	{ 507, { 3, 3, 3, 3, 3, 3 } },
	{ 508, { 3, 3, 3, 3, 3, 3 } },
	{ 509, { 4, 4, 4, 4, 4, 4 } },
	{ 510, { 4, 4, 4, 4, 4, 4 } },
	{ 511, { 4, 4, 4, 4, 4, 4 } },
	{ 512, { 1, 1, 1, 1, 1, 1 } },
	{ 513, { 3, 3, 3, 3, 3, 3 } },
	{ 514, { 3, 3, 3, 3, 3, 3 } },
	{ 515, { 3, 3, 3, 3, 3, 3 } },
	{ 516, { 3, 3, 3, 3, 3, (DWORD)E_NOINTERFACE } },
	{ 517, { 3, 3, 3, 3, 3, 3 } },
	{ 518, { 3, 3, 3, 3, 3, 3 } },
	{ 519, { 4, 4, 4, 4, 4, 4 } },
	{ 520, { 4, 4, 4, 4, 4, 4 } },
	{ 521, { 4, 4, 4, 4, 4, 4 } },
	{ 522, { 1, 1, 1, 1, 1, 1 } },
	{ 523, { 3, 3, 3, 3, 3, 3 } },
	{ 524, { 3, 3, 3, 3, 3, 3 } },
	{ 525, { 3, 3, 3, 3, 3, 3 } },
	{ 526, { 3, 3, 3, 3, 3, (DWORD)E_NOINTERFACE } },
	{ 527, { 3, 3, 3, 3, 3, 3 } },
	{ 528, { 3, 3, 3, 3, 3, 3 } },
	{ 529, { 4, 4, 4, 4, 4, 4 } },
	{ 530, { 4, 4, 4, 4, 4, 4 } },
	{ 530, { 4, 4, 4, 4, 4, 4 } },
	{ 531, { 4, 4, 4, 4, 4, 4 } },
	{ 532, { 1, 1, 1, 1, 1, 1 } },
	{ 533, { 3, 3, 3, 3, 3, 3 } },
	{ 534, { 3, 3, 3, 3, 3, 3 } },
	{ 535, { 3, 3, 3, 3, 3, 3 } },
	{ 536, { (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, 3 } },
	{ 537, { 3, 3, 3, 3, 3, 3 } },
	{ 538, { 3, 3, 3, 3, 3, 3 } },
	{ 539, { 4, 4, 4, 4, 4, 4 } },
	{ 540, { 4, 4, 4, 4, 4, 4 } },
	{ 541, { 4, 4, 4, 4, 4, 4 } },
	{ 542, { 1, 1, 1, 1, 1, 1 } },
	{ 543, { 3, 3, 3, 3, 3, 3 } },
	{ 544, { 3, 3, 3, 3, 3, 3 } },
	{ 545, { 3, 3, 3, 3, 3, 3 } },

	// IDirectDrawPalette / IDirectDrawClipper
	{ 600, { 1, 1, 1, 1, 1, 1 } },
	{ 601, { 1, 1, 1, 2, 2, 2 } },
	{ 602, { 1, 1, 1, 1, 1, 1 } },
	{ 650, { 1, 1, 1, 1, 1, 1 } },
	{ 651, { 1, 1, 1, 2, 2, 2 } },
	{ 652, { 1, 1, 1, 1, 1, 1 } },

	// IDirect3DLight / IDirect3DMaterial
	{ 700, { 1, 1, 1, 1, 0, 0 } },
	{ 701, { 2, 2, 2, 2, 0, 0 } },
	{ 702, { 2, 2, 2, 2, 0, 0 } },
	{ 703, { 2, 2, 2, 2, 0, 0 } },
	{ 710, { 1, 1, 1, 1, 0, 0 } },
	{ 711, { 2, 2, 2, 2, 0, 0 } },
	{ 712, { 2, 2, 2, 2, 0, 0 } },
	{ 713, { 2, 2, 2, 2, 0, 0 } },
	{ 720, { 1, 1, 1, 1, 0, 0 } },
	{ 721, { 2, 2, 2, 2, 0, 0 } },
	{ 722, { 2, 2, 2, 2, 0, 0 } },
	{ 723, { 2, 2, 2, 2, 0, 0 } },
	{ 730, { 1, 1, 1, 1, 0, 1 } },
	{ 731, { 2, 2, 2, 2, 0, 3 } },
	{ 732, { 2, 2, 2, 2, 0, 3 } },
	{ 733, { 2, 2, 2, 2, 0, 2 } },

	// IDirect3DDevice
	{ 800, { 1, 1, 1, 1, 1, 1 } },
	{ 801, { 2, 2, 2, 3, 3, 3 } },
	{ 802, { 2, 2, 2, 3, 3, 3 } },
	{ 803, { 2, 1, 1, 1, 1, 1 } },
	{ 804, { 2, 2, 2, 2, 2, 2 } },
	{ 805, { 2, 2, 2, 4, 4, 11 } },
	{ 806, { 2, 2, 2, 4, 4, 11 } },
	{ 807, { 1, 1, 1, 1, 1, 1 } },
	{ 808, { 2, 2, 2, 5, 5, 12 } },
	{ 809, { 2, 2, 2, 5, 5, 12 } },
	{ 810, { 0, 0, 0, 0, 0, 0 } },
	{ 811, { 2, 1, 1, 1, 1, 1 } },
	{ 812, { 2, 2, 2, 2, 2, 2 } },
	{ 813, { 2, 1, 1, 1, 1, 1 } },
	{ 814, { 2, 2, 2, 2, 2, 2 } },
	{ 815, { 2, 2, 2, 5, 5, 19 } },
	{ 816, { 2, 2, 2, 5, 5, 19 } },
	{ 817, { 1, 1, 1, 1, 1, 1 } },
	{ 818, { 2, 1, 1, 1, 1, 1 } },
	{ 819, { 2, 2, 2, 2, 2, 2 } },
	{ 820, { 2, 2, 2, 5, 5, 12 } },
	{ 821, { 2, 2, 2, 5, 5, 12 } },
	{ 822, { 2, 2, 2, 4, 4, 11 } },
	{ 823, { 2, 1, 1, 1, 1, 1 } },
	{ 824, { 2, 2, 2, 2, 2, 2 } },
	{ 825, { 2, 2, 2, 4, 4, 11 } },
	{ 826, { 2, 2, 2, 4, 4, 4 } },
	{ 827, { 2, 2, 2, 4, 4, 4 } },
	{ 828, { 2, 2, 2, 2, 2, 2 } },
	{ 829, { 2, 2, 2, 2, 2, 2 } },
	{ 830, { 3, 2, 2, 2, 2, 2 } },
	{ 831, { 3, 2, 2, 2, 2, 2 } },
	{ 832, { 2, 2, 2, 4, 4, 11 } },
	{ 833, { 2, 2, 2, 4, 4, 11 } },
	{ 834, { 3, 3, 3, 3, 3, 3 } },
	{ 835, { 3, 1, 1, 1, 1, 1 } },
	{ 836, { 2, 2, 2, 4, 4, 11 } },
	{ 837, { 2, 2, 2, 4, 4, 11 } },
	{ 838, { 1, 1, 1, 1, 1, 1 } },
	{ 839, { 1, 1, 1, 1, 1, 1 } },
	{ 840, { 2, 2, 2, 4, 4, 4 } },
	{ 841, { 2, 2, 2, 4, 4, 4 } },
	{ 842, { 2, 2, 2, 2, 2, 2 } },
	{ 843, { 1, 1, 1, 1, 1, 1 } },
	{ 844, { 2, 2, 2, 4, 4, 4 } },
	{ 845, { 2, 2, 2, 4, 4, 4 } },

	// Execute Buffer
	{ 900, { 1, 0, 0, 0, 0, 0 } },
	{ 901, { 2, 0, 0, 0, 0, 0 } },
	{ 902, { 2, 0, 0, 0, 0, 0 } },
	{ 903, { 2, 0, 0, 0, 0, 0 } },

	// All interfaces quried by IDirectDraw
	{ 1000, { 2, 1, (DWORD)E_NOINTERFACE, 1, 1, 1 } },
	{ 1001, { 2, 1, (DWORD)E_NOINTERFACE, 1, 1, 1 } },
	{ 1002, { 2, 1, (DWORD)E_NOINTERFACE, 1, 1, 1 } },
	{ 1003, { 1, 2, (DWORD)E_NOINTERFACE, 1, 1, 1 } },
	{ 1004, { 1, 2, (DWORD)E_NOINTERFACE, 1, 1, 1 } },
	{ 1005, { 1, 2, (DWORD)E_NOINTERFACE, 1, 1, 1 } },
	{ 1006, { (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE } },
	{ 1009, { 1, 1, (DWORD)E_NOINTERFACE, 2, 1, 1 } },
	{ 1010, { 1, 1, (DWORD)E_NOINTERFACE, 2, 1, 1 } },
	{ 1011, { 1, 1, (DWORD)E_NOINTERFACE, 2, 1, 1 } },
	{ 1012, { 1, 1, (DWORD)E_NOINTERFACE, 1, 2, 2 } },
	{ 1013, { 1, 1, (DWORD)E_NOINTERFACE, 1, 2, 2 } },
	{ 1014, { 1, 1, (DWORD)E_NOINTERFACE, 1, 2, 2 } },
	{ 1015, { (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE } },
	{ 1018, { (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE } },
	{ 1021, { (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE } },
	{ 1024, { (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE } },
	{ 1027, { (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE } },
	{ 1030, { (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE } },
	{ 1033, { (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE } },
	{ 1036, { (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE } },
	{ 1039, { (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE } },
	{ 1042, { 2, 2, (DWORD)E_NOINTERFACE, 2, 2, (DWORD)E_NOINTERFACE } },
	{ 1043, { 2, 2, (DWORD)E_NOINTERFACE, 2, 2, (DWORD)E_NOINTERFACE } },
	{ 1044, { 2, 2, (DWORD)E_NOINTERFACE, 2, 2, (DWORD)E_NOINTERFACE } },
	{ 1045, { 2, 2, (DWORD)E_NOINTERFACE, 2, 2, (DWORD)E_NOINTERFACE } },
	{ 1046, { 2, 2, (DWORD)E_NOINTERFACE, 2, 2, (DWORD)E_NOINTERFACE } },
	{ 1047, { 2, 2, (DWORD)E_NOINTERFACE, 2, 2, (DWORD)E_NOINTERFACE } },
	{ 1048, { 2, 2, (DWORD)E_NOINTERFACE, 2, 2, (DWORD)E_NOINTERFACE } },
	{ 1049, { 2, 2, (DWORD)E_NOINTERFACE, 2, 2, (DWORD)E_NOINTERFACE } },
	{ 1050, { 2, 2, (DWORD)E_NOINTERFACE, 2, 2, (DWORD)E_NOINTERFACE } },
	{ 1051, { (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, 2 } },
	{ 1052, { (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, 2 } },
	{ 1053, { (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, 2 } },
	{ 1054, { (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE } },
	{ 1057, { (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE } },
	{ 1060, { (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE } },
	{ 1063, { (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE } },
	{ 1066, { (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE } },
	{ 1069, { (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE } },
	{ 1072, { (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE } },
	{ 1075, { (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE } },
	{ 1078, { (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE } },
	{ 1081, { (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE } },
	{ 1084, { (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE } },
	{ 1087, { (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE } },
	{ 1090, { (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE } },
	{ 1093, { (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE } },
	{ 1096, { (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE } },
	{ 1099, { (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE, (DWORD)E_NOINTERFACE } },
};

template <typename DDType>
static DWORD GetResults(DWORD TestID)
{
	static DWORD ArrayLoc =
		std::is_same_v<DDType, IDirectDraw> ? vDDraw1 :
		std::is_same_v<DDType, IDirectDraw2> ? vDDraw2 :
		std::is_same_v<DDType, IDirectDraw3> ? vDDraw3 :
		std::is_same_v<DDType, IDirectDraw4> ? vDDraw4 :
		std::is_same_v<DDType, IDirectDraw7> ? vDDraw7 :
		std::is_same_v<DDType, IDirectDraw7Ex> ? vDDraw7Ex : TEST_FAILED;
	if (ArrayLoc == TEST_FAILED)
	{
		return TEST_FAILED;
	}

	for (const TESTMATRIX& entry : TestResults)
	{
		if (entry.TestID == TestID)
		{
			return entry.Result[ArrayLoc];
		}
	}

	return TEST_FAILED;
}
