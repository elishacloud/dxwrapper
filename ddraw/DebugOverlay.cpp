/**
* Copyright (C) 2023 Elisha Riedlinger
*
* This software is  provided 'as-is', without any express  or implied  warranty. In no event will the
* authors be held liable for any damages arising from the use of this software.
* Permission  is granted  to anyone  to use  this software  for  any  purpose,  including  commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*   1. The origin of this software must not be misrepresented; you must not claim that you  wrote the
*      original  software. If you use this  software  in a product, an  acknowledgment in the product
*      documentation would be appreciated but is not required.
*   2. Altered source versions must  be plainly  marked as such, and  must not be  misrepresented  as
*      being the original software.
*   3. This notice may not be removed or altered from any source distribution.
*/

#include "DebugOverlay.h"
#include "RenderData.h"
#include <sstream>

#define IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DISABLE_OBSOLETE_KEYIO

HRESULT DwmIsCompositionEnabled(BOOL* enabled) { *enabled = FALSE; return E_NOTIMPL; }
HRESULT DwmGetColorizationColor(DWORD* colorization, BOOL* opaqueBlend) { *colorization = 0; *opaqueBlend = FALSE; return E_NOTIMPL; }
HRESULT DwmEnableBlurBehindWindow(HWND, const struct DWM_BLURBEHIND*) { return E_NOTIMPL; }
struct DWM_BLURBEHIND { DWORD dwFlags; BOOL fEnable; HRGN hRgnBlur; BOOL fTransitionOnMaximized; };
#define DWM_BB_ENABLE 0x00000001
#define DWM_BB_BLURREGION 0x00000002

#include "External/imgui/imgui.h"
#include "External/imgui/backends/imgui_impl_win32.h"
#include "External/imgui/backends/imgui_impl_dx9.h"

#include "External/imgui/imgui.cpp"
#include "External/imgui/imgui_draw.cpp"
#include "External/imgui/imgui_tables.cpp"
#include "External/imgui/imgui_widgets.cpp"
#include "External/imgui/backends/imgui_impl_win32.cpp"
#include "External/imgui/backends/imgui_impl_dx9.cpp"

namespace {
	HWND OriginalHwnd = nullptr;
	WNDPROC OverrideWndProc_OriginalWndProc = nullptr;
	LRESULT CALLBACK OverrideWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		LRESULT res = OverrideWndProc_OriginalWndProc(hwnd, uMsg, wParam, lParam);

		ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam);

		return res;
	}
}

void DebugOverlay::Setup(HWND hwnd, LPDIRECT3DDEVICE9 d3d9Device)
{
	if (IsContextSetup)
	{
		Shutdown();
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX9_Init(d3d9Device);

	// Restore WndProc if hwnd changes
	if (IsWindow(OriginalHwnd) && OriginalHwnd != hwnd)
	{
		SetWindowLongPtr(OriginalHwnd, GWLP_WNDPROC, (LONG_PTR)OverrideWndProc_OriginalWndProc);
		OverrideWndProc_OriginalWndProc = nullptr;
		OriginalHwnd = nullptr;
	}

	// Override WndProc if not already overridden
	if (OriginalHwnd != hwnd)
	{
		OriginalHwnd = hwnd;
		OverrideWndProc_OriginalWndProc = (WNDPROC)GetWindowLongPtr(hwnd, GWLP_WNDPROC);
		SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)OverrideWndProc);
	}

	// Context is setup
	IsContextSetup = true;
}

void DebugOverlay::Shutdown()
{
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	IsContextSetup = false;
}

void DebugOverlay::BeginScene()
{
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void DebugOverlay::EndScene(const RenderData &RenderData)
{
	static bool ShowDebugUI = false;
	if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_LeftAlt)) &&
		ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_D), false))
	{
		ShowDebugUI = !ShowDebugUI;
	}

	if (ShowDebugUI)
	{
		std::stringstream matrices;
		matrices << "WORLD\n" <<
			worldMatrix._11 << " / " << worldMatrix._12 << " / " << worldMatrix._13 << " / " << worldMatrix._14 << '\n' <<
			worldMatrix._21 << " / " << worldMatrix._22 << " / " << worldMatrix._23 << " / " << worldMatrix._24 << '\n' <<
			worldMatrix._31 << " / " << worldMatrix._32 << " / " << worldMatrix._33 << " / " << worldMatrix._34 << '\n' <<
			worldMatrix._41 << " / " << worldMatrix._42 << " / " << worldMatrix._43 << " / " << worldMatrix._44;

		matrices << "\n\nVIEW\n" <<
			viewMatrix._11 << " / " << viewMatrix._12 << " / " << viewMatrix._13 << " / " << viewMatrix._14 << '\n' <<
			viewMatrix._21 << " / " << viewMatrix._22 << " / " << viewMatrix._23 << " / " << viewMatrix._24 << '\n' <<
			viewMatrix._31 << " / " << viewMatrix._32 << " / " << viewMatrix._33 << " / " << viewMatrix._34 << '\n' <<
			viewMatrix._41 << " / " << viewMatrix._42 << " / " << viewMatrix._43 << " / " << viewMatrix._44;

		matrices << "\n\nPROJECTION\n" <<
			projectionMatrix._11 << " / " << projectionMatrix._12 << " / " << projectionMatrix._13 << " / " << projectionMatrix._14 << '\n' <<
			projectionMatrix._21 << " / " << projectionMatrix._22 << " / " << projectionMatrix._23 << " / " << projectionMatrix._24 << '\n' <<
			projectionMatrix._31 << " / " << projectionMatrix._32 << " / " << projectionMatrix._33 << " / " << projectionMatrix._34 << '\n' <<
			projectionMatrix._41 << " / " << projectionMatrix._42 << " / " << projectionMatrix._43 << " / " << projectionMatrix._44;

		ImGui::Begin("Matrices");
		ImGui::Text(matrices.str().c_str());
		ImGui::End();

		ImGui::Begin("Lights");
		if (LightDebugInfos.size() < 1)
		{
			ImGui::Text("None");
		}
		else
		{
			std::stringstream ss;

			for (size_t i = 0; i < LightDebugInfos.size(); ++i)
			{
				const LightDebugInfo& light = LightDebugInfos[i];

				const char* type;
				switch (light.type)
				{
				case D3DLIGHT_POINT: type = "Point"; break;
				case D3DLIGHT_SPOT: type = "Spot"; break;
				case D3DLIGHT_DIRECTIONAL: type = "Dir"; break;
				default: type = "ERROR"; break;
				}

				ss << (int)light.index << ' ' << type;

				if (light.diffuseColor.a > 0)
				{
					ss << "  dif: " << (int)light.diffuseColor.r << ',' << (int)light.diffuseColor.g << ',' << (int)light.diffuseColor.b << ',' << (int)light.diffuseColor.a;
				}

				if (light.specularColor.a > 0)
				{
					ss << "  spec: " << (int)light.specularColor.r << ',' << (int)light.specularColor.g << ',' << (int)light.specularColor.b << ',' << (int)light.specularColor.a;
				}

				if (light.ambientColor.a > 0)
				{
					ss << "  amb: " << (int)light.ambientColor.r << ',' << (int)light.ambientColor.g << ',' << (int)light.ambientColor.b << ',' << (int)light.ambientColor.a;
				}

				ss << "\n  pos: " << light.position.x << " / " << light.position.y << " / " << light.position.z;

				if (light.type != D3DLIGHT_POINT)
				{
					ss << "  dir: " << light.direction.x << " / " << light.direction.y << " / " << light.direction.z;
				}

				if (i < LightDebugInfos.size() - 1)
				{
					ss << '\n';
				}
			}

			ImGui::Text(ss.str().c_str());
		}
		ImGui::End();

		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	}
	else
	{
		ImGui::EndFrame();
	}
}

void DebugOverlay::SetTransform(D3DTRANSFORMSTATETYPE dtstTransformStateType, LPD3DMATRIX lpD3DMatrix)
{
	switch ((int)dtstTransformStateType)
	{
	case D3DTRANSFORMSTATE_WORLD:
	case D3DTS_WORLD:
		std::memcpy(&worldMatrix, lpD3DMatrix, sizeof(D3DMATRIX));
		break;

	case D3DTS_VIEW:
		std::memcpy(&viewMatrix, lpD3DMatrix, sizeof(D3DMATRIX));
		break;

	case D3DTS_PROJECTION:
		std::memcpy(&projectionMatrix, lpD3DMatrix, sizeof(D3DMATRIX));
		break;

	default:
		break;
	}
}

void DebugOverlay::SetLight(DWORD dwLightIndex, LPD3DLIGHT7 lpLight)
{
	bool found = false;
	for (size_t i = 0; i < LightDebugInfos.size(); ++i)
	{
		LightDebugInfo& info = LightDebugInfos[i];

		if (info.index == (char)dwLightIndex)
		{
			found = true;
			info.type = (char)lpLight->dltType;
			info.position = lpLight->dvPosition;
			info.direction = lpLight->dvDirection;
			info.diffuseColor = lpLight->dcvDiffuse;
			info.specularColor = lpLight->dcvSpecular;
			info.ambientColor = lpLight->dcvAmbient;
		}
	}

	if (!found)
	{
		LightDebugInfos.push_back({ (char)dwLightIndex, (char)lpLight->dltType, lpLight->dvPosition, lpLight->dvDirection,
								 lpLight->dcvDiffuse, lpLight->dcvSpecular, lpLight->dcvAmbient });
	}
}

void DebugOverlay::LightEnable(DWORD dwLightIndex, BOOL bEnable)
{
	UNREFERENCED_PARAMETER(bEnable);

	for (size_t i = 0; i < LightDebugInfos.size(); ++i)
	{
		if (LightDebugInfos[i].index == (char)dwLightIndex)
		{
			LightDebugInfos.erase(LightDebugInfos.begin() + i);
			break;
		}
	}
}
