#pragma once

#include <Support/WinInclude.h>
#include <Support/ComPointer.h>
#include <Support/Shader.h>

#include <D3D/DXContext.h>

class DXPipelineState
{
	public:
		void Init(LPCWSTR name, ComPointer<ID3D12RootSignature> &rootSignature, D3D12_INPUT_ELEMENT_DESC vertexLayout[], UINT vertexLayoutCount, Shader &vertexShader, Shader &pixelShader);
		void SetWireframe();
		void Release();

		inline ComPointer<ID3D12PipelineState>& Get()
		{
			return m_pso;
		};

		//~DXPipelineState();

	private:
		LPCWSTR m_name;
		D3D12_GRAPHICS_PIPELINE_STATE_DESC m_gfxPsod;
		ComPointer<ID3D12PipelineState> m_pso;
};
		