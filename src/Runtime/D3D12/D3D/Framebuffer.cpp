#include "FrameBuffer.h"

FrameBuffer::FrameBuffer(uint32_t width, uint32_t height, std::string name)
{
	m_width = width;
	m_height = height;
	m_name = name;
}

void FrameBuffer::RenderTargetBuffer(D3D12_HEAP_PROPERTIES* defaultHeapProperties, ID3D12DescriptorHeap* descriptorHeap, uint32_t heapIndex)
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

	DXContext::Get().GetDevice()->CreateCommittedResource(defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &frameBufferDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, IID_PPV_ARGS(&m_RTV));

	if (m_RTVHeap == nullptr && descriptorHeap == nullptr)
	{// Descriptor Heap
		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDescRTV{};
		descriptorHeapDescRTV.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		descriptorHeapDescRTV.NumDescriptors = 1;
		descriptorHeapDescRTV.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		descriptorHeapDescRTV.NodeMask = 0;

		DXContext::Get().GetDevice()->CreateDescriptorHeap(&descriptorHeapDescRTV, IID_PPV_ARGS(&m_RTVHeap));
		
		std::string tempName = m_name + "_RTV_DescriptorHeap";

		m_RTVHeap.Get()->SetName(std::wstring(tempName.begin(), tempName.end()).c_str());
	}
	else
	{
		m_RTVHeap = descriptorHeap;
		m_RTVHeapIndex = heapIndex;
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle;
	uint32_t descriptorSize = DXContext::Get().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	rtvHandle = m_RTVHeap->GetCPUDescriptorHandleForHeapStart();

	if (heapIndex > 0)
	{
		rtvHandle.Offset(m_RTVHeapIndex, descriptorSize);
	}

	DXContext::Get().GetDevice()->CreateRenderTargetView(m_RTV, nullptr, rtvHandle);
}

void FrameBuffer::DepthBuffer(D3D12_HEAP_PROPERTIES* defaultHeapProperties, ID3D12DescriptorHeap* descriptorHeap, uint32_t heapIndex)
{
	std::string name = m_name + "_DSV";

	m_ZBuffer = ZBuffer(defaultHeapProperties, m_name, m_width, m_height, descriptorHeap, heapIndex);

	m_DSVHeap = m_ZBuffer.GetDescriptorHeap();
}

void FrameBuffer::Clear(ID3D12GraphicsCommandList*& cmdList)
{
	if (m_RTVHeap != nullptr)
	{
		const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		cmdList->ClearRenderTargetView(m_RTVHeap->GetCPUDescriptorHandleForHeapStart(), clearColor, 0, nullptr);
	}
	if (m_ZBuffer.GetDescriptorHeap() != nullptr)
	{
		cmdList->ClearDepthStencilView(m_ZBuffer.GetDescriptorHeap()->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}
}

void FrameBuffer::CreateRenderTargetSRV()
{

}

void FrameBuffer::CreateDepthBufferSRV()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;  // Use R32 because it's a depth-only texture
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 1;

	//DXContext().Get().GetDevice()->CreateShaderResourceView(shadowMap.Get(), &srvDesc, m_srvHeap->GetCPUDescriptorHandleForHeapStart());

}
