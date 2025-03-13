#include "PipelineState.h"

void DXPipelineState::Init(LPCWSTR name, ComPointer<ID3D12RootSignature> &rootSignature, D3D12_INPUT_ELEMENT_DESC vertexLayout[], UINT vertexLayoutCount, Shader &vertexShader, Shader &pixelShader)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gfxPsod{};
	// Root Signature
	gfxPsod.pRootSignature = rootSignature;
	// Input Layout
	gfxPsod.InputLayout.NumElements = vertexLayoutCount;
	gfxPsod.InputLayout.pInputElementDescs = vertexLayout;
	gfxPsod.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	// Vertex Shader
	gfxPsod.VS.pShaderBytecode = vertexShader.GetBuffer();
	gfxPsod.VS.BytecodeLength = vertexShader.GetSize();
	// Pixel Shader
	gfxPsod.PS.pShaderBytecode = pixelShader.GetBuffer();
	gfxPsod.PS.BytecodeLength = pixelShader.GetSize();
	// Other Shaders zeroing
	gfxPsod.DS.BytecodeLength = 0;
	gfxPsod.DS.pShaderBytecode = nullptr;
	gfxPsod.HS.BytecodeLength = 0;
	gfxPsod.HS.pShaderBytecode = nullptr;
	gfxPsod.GS.BytecodeLength = 0;
	gfxPsod.GS.pShaderBytecode = nullptr;
	// Rasterizer
	gfxPsod.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	gfxPsod.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	gfxPsod.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	gfxPsod.RasterizerState.FrontCounterClockwise = TRUE;
	gfxPsod.RasterizerState.DepthBias = 0;
	gfxPsod.RasterizerState.DepthBiasClamp = 0.f;
	gfxPsod.RasterizerState.SlopeScaledDepthBias = 0.f;
	gfxPsod.RasterizerState.DepthClipEnable = FALSE;
	gfxPsod.RasterizerState.MultisampleEnable = FALSE;
	gfxPsod.RasterizerState.AntialiasedLineEnable = FALSE;
	gfxPsod.RasterizerState.ForcedSampleCount = 0;
	// Stream Output
	gfxPsod.StreamOutput.NumEntries = 0;
	gfxPsod.StreamOutput.NumStrides = 0;
	gfxPsod.StreamOutput.pBufferStrides = nullptr;
	gfxPsod.StreamOutput.pSODeclaration = nullptr;
	gfxPsod.StreamOutput.RasterizedStream = 0;
	gfxPsod.NumRenderTargets = 1;
	gfxPsod.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	// Blend State
	gfxPsod.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	gfxPsod.BlendState.AlphaToCoverageEnable = FALSE;
	gfxPsod.BlendState.IndependentBlendEnable = FALSE;
	gfxPsod.BlendState.RenderTarget[0].BlendEnable = TRUE;
	gfxPsod.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	gfxPsod.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	gfxPsod.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	gfxPsod.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	gfxPsod.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
	gfxPsod.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	gfxPsod.BlendState.RenderTarget[0].LogicOpEnable = FALSE;
	gfxPsod.BlendState.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;;
	gfxPsod.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	// Depth State
	gfxPsod.DepthStencilState.DepthEnable = TRUE;
	gfxPsod.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	gfxPsod.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	gfxPsod.DepthStencilState.StencilEnable = FALSE;
	gfxPsod.DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	gfxPsod.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
	const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp =
	{ D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP ,D3D12_COMPARISON_FUNC_ALWAYS };
	gfxPsod.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	gfxPsod.DepthStencilState.FrontFace = defaultStencilOp;
	gfxPsod.DepthStencilState.BackFace = defaultStencilOp;
	// Sample Mask/Desc
	gfxPsod.SampleMask = 0xFFFFFFFF;
	gfxPsod.SampleDesc.Count = 1;
	gfxPsod.SampleDesc.Quality = 0;
	// 
	gfxPsod.NodeMask = 0;
	gfxPsod.CachedPSO.CachedBlobSizeInBytes = 0;
	gfxPsod.CachedPSO.pCachedBlob = nullptr;
	gfxPsod.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	m_gfxPsod = gfxPsod;
	m_name = name;

}

void DXPipelineState::Create()
{
	DXContext::Get().GetDevice()->CreateGraphicsPipelineState(&m_gfxPsod, IID_PPV_ARGS(&m_pso));

	m_pso.Get()->SetName(m_name);
}

void DXPipelineState::Release()
{
	m_pso->Release();
}

void DXPipelineState::SetWireframe()
{
	m_gfxPsod.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME; 
}