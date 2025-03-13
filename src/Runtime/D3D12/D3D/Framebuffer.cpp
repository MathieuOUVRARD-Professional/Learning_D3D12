#include "FrameBuffer.h"

FrameBuffer::FrameBuffer(uint32_t width, uint32_t height)
{
	m_width = width;
	m_height = height;
}

void FrameBuffer::RenderTargetBuffer(D3D12_HEAP_PROPERTIES* defaultHeapProperties)
{
	D3D12_RESOURCE_DESC frameBufferDesc = {};
	frameBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	frameBufferDesc.Width = m_width;
	frameBufferDesc.Height = m_height;
	frameBufferDesc.DepthOrArraySize = 1;
	frameBufferDesc.MipLevels = 1;
	frameBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	frameBufferDesc.SampleDesc.Count = 1;
	frameBufferDesc.SampleDesc.Quality = 1;
	frameBufferDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	//Describe clear values
	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = frameBufferDesc.Format;
	clearValue.Color[0] = 0.0f;
	clearValue.Color[1] = 0.0f;
	clearValue.Color[2] = 0.0f;
	clearValue.Color[3] = 1.0f;

	DXContext::Get().GetDevice()->CreateCommittedResource(defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &frameBufferDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, IID_PPV_ARGS(&m_frameBuffer));

	// Descriptor Heap
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDescRTV{};
	descriptorHeapDescRTV.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descriptorHeapDescRTV.NumDescriptors = 1;
	descriptorHeapDescRTV.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	descriptorHeapDescRTV.NodeMask = 0;

	DXContext::Get().GetDevice()->CreateDescriptorHeap(&descriptorHeapDescRTV, IID_PPV_ARGS(&m_rtvHeap));

	DXContext::Get().GetDevice()->CreateRenderTargetView(m_frameBuffer, nullptr, m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
}

void FrameBuffer::DepthBuffer(D3D12_HEAP_PROPERTIES* defaultHeapProperties)
{
	m_ZBuffer = ZBuffer(defaultHeapProperties, m_width, m_height);
}

void FrameBuffer::Clear(ID3D12GraphicsCommandList*& cmdList)
{
	if (m_rtvHeap != nullptr)
	{
		const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		cmdList->ClearRenderTargetView(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), clearColor, 0, nullptr);
	}
	if (m_ZBuffer.GetDescriptorHeap() != nullptr)
	{
		cmdList->ClearDepthStencilView(m_ZBuffer.GetDescriptorHeap()->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}
}
