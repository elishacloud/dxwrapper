#pragma once

#include <array>
#include <map>
#include <vector>

namespace D3dDdi
{
	class Device;

	class DeviceState
	{
	public:
		DeviceState(Device& device);
		
		HRESULT pfnCreateVertexShaderDecl(D3DDDIARG_CREATEVERTEXSHADERDECL* data, const D3DDDIVERTEXELEMENT* vertexElements);
		HRESULT pfnDeletePixelShader(HANDLE shader);
		HRESULT pfnDeleteVertexShaderDecl(HANDLE shader);
		HRESULT pfnDeleteVertexShaderFunc(HANDLE shader);
		HRESULT pfnSetPixelShader(HANDLE shader);
		HRESULT pfnSetPixelShaderConst(const D3DDDIARG_SETPIXELSHADERCONST* data, const FLOAT* registers);
		HRESULT pfnSetPixelShaderConstB(const D3DDDIARG_SETPIXELSHADERCONSTB* data, const BOOL* registers);
		HRESULT pfnSetPixelShaderConstI(const D3DDDIARG_SETPIXELSHADERCONSTI* data, const INT* registers);
		HRESULT pfnSetRenderState(const D3DDDIARG_RENDERSTATE* data);
		HRESULT pfnSetTexture(UINT stage, HANDLE texture);
		HRESULT pfnSetTextureStageState(const D3DDDIARG_TEXTURESTAGESTATE* data);
		HRESULT pfnSetVertexShaderConst(const D3DDDIARG_SETVERTEXSHADERCONST* data, const void* registers);
		HRESULT pfnSetVertexShaderConstB(const D3DDDIARG_SETVERTEXSHADERCONSTB* data, const BOOL* registers);
		HRESULT pfnSetVertexShaderConstI(const D3DDDIARG_SETVERTEXSHADERCONSTI* data, const INT* registers);
		HRESULT pfnSetVertexShaderDecl(HANDLE shader);
		HRESULT pfnSetVertexShaderFunc(HANDLE shader);
		HRESULT pfnSetZRange(const D3DDDIARG_ZRANGE* data);
		HRESULT pfnUpdateWInfo(const D3DDDIARG_WINFO* data);

		void removeTexture(HANDLE texture);

	private:
		typedef std::tuple<FLOAT, FLOAT, FLOAT, FLOAT> ShaderConstF;
		typedef std::tuple<INT, INT, INT, INT> ShaderConstI;

		HRESULT deleteShader(HANDLE shader, HANDLE& currentShader,
			HRESULT(APIENTRY* origDeleteShaderFunc)(HANDLE, HANDLE));
		HRESULT setShader(HANDLE shader, HANDLE& currentShader,
			HRESULT(APIENTRY* origSetShaderFunc)(HANDLE, HANDLE));

		template <typename SetShaderConstData, typename ShaderConst, typename Registers>
		HRESULT setShaderConst(const SetShaderConstData* data, const Registers* registers,
			std::vector<ShaderConst>& shaderConst,
			HRESULT(APIENTRY* origSetShaderConstFunc)(HANDLE, const SetShaderConstData*, const Registers*));

		template <typename StateData>
		HRESULT setState(const StateData* data, StateData& currentState,
			HRESULT(APIENTRY* origSetState)(HANDLE, const StateData*));

		template <typename StateData, UINT size>
		HRESULT setStateArray(const StateData* data, std::array<UINT, size>& currentState,
			HRESULT(APIENTRY* origSetState)(HANDLE, const StateData*));

		Device& m_device;
		HANDLE m_pixelShader;
		std::vector<ShaderConstF> m_pixelShaderConst;
		std::vector<BOOL> m_pixelShaderConstB;
		std::vector<ShaderConstI> m_pixelShaderConstI;
		std::array<UINT, D3DDDIRS_BLENDOPALPHA + 1> m_renderState;
		std::array<HANDLE, 8> m_textures;
		std::array<std::array<UINT, D3DDDITSS_TEXTURECOLORKEYVAL + 1>, 8> m_textureStageState;
		std::vector<ShaderConstF> m_vertexShaderConst;
		std::vector<BOOL> m_vertexShaderConstB;
		std::vector<ShaderConstI> m_vertexShaderConstI;
		std::map<HANDLE, std::vector<D3DDDIVERTEXELEMENT>> m_vertexShaderDecls;
		HANDLE m_vertexShaderDecl;
		HANDLE m_vertexShaderFunc;
		D3DDDIARG_WINFO m_wInfo;
		D3DDDIARG_ZRANGE m_zRange;
	};
}
