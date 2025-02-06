#pragma once

#include <Support/WinInclude.h>
#include <Support/ComPointer.h>
#include <Support/Shader.h>

#include <D3D/DXContext.h>

class DXPipelineState
{
	public:
		void Init(ComPointer<ID3D12RootSignature> &rootSignature, D3D12_INPUT_ELEMENT_DESC vertexLayout[], UINT vertexLayoutCount, Shader &vertexShader, Shader &pixelShader);
		ComPointer<ID3D12PipelineState>& Get();
		void Release();

		//~DXPipelineState();

	private:
		ComPointer<ID3D12PipelineState> pso;
};