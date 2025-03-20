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

	// RTV DESCRIPTOR HEAP
	if (m_RTVHeap == nullptr && descriptorHeap == nullptr)
	{
		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDescRTV{};
		descriptorHeapDescRTV.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		descriptorHeapDescRTV.NumDescriptors = 1;
		descriptorHeapDescRTV.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		descriptorHeapDescRTV.NodeMask = 0;

		DXContext::Get().GetDevice()->CreateDescriptorHeap(&descriptorHeapDescRTV, IID_PPV_ARGS(&m_RTVHeap));
		
		std::string tempName = m_name + "_RTV";

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
	m_DSVHeapIndex = heapIndex;
}

void FrameBuffer::CreateRenderTargetSRV(ID3D12DescriptorHeap* descriptorHeap, uint32_t heapIndex)
{
	// RT SRV DESCRIPTOR HEAP
	if (m_RT_SRVHeap == nullptr && descriptorHeap == nullptr)
	{
		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDescRT_SRV{};
		descriptorHeapDescRT_SRV.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		descriptorHeapDescRT_SRV.NumDescriptors = 1;
		descriptorHeapDescRT_SRV.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		descriptorHeapDescRT_SRV.NodeMask = 0;

		DXContext::Get().GetDevice()->CreateDescriptorHeap(&descriptorHeapDescRT_SRV, IID_PPV_ARGS(&m_RT_SRVHeap));

		std::string tempName = m_name + "_RT_SRV";

		m_RT_SRVHeap.Get()->SetName(std::wstring(tempName.begin(), tempName.end()).c_str());
	}
	else
	{
		m_RT_SRVHeap = descriptorHeap;
		m_RT_SRVHeapIndex = heapIndex;
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE rT_SRVHandle;
	uint32_t descriptorSize = DXContext::Get().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	rT_SRVHandle = m_RT_SRVHeap->GetCPUDescriptorHandleForHeapStart();

	if (m_RT_SRVHeapIndex > 0)
	{
		rT_SRVHandle.Offset(m_RT_SRVHeapIndex, descriptorSize);
	}

	D3D12_SHADER_RESOURCE_VIEW_DESC rT_SRVDesc = {};
	rT_SRVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  // Use R32 because it's a depth-only texture
	rT_SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	rT_SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	rT_SRVDesc.Texture2D.MipLevels = 1;
	rT_SRVDesc.Texture2D.MostDetailedMip = 0;
	rT_SRVDesc.Texture2D.PlaneSlice = 0;
	rT_SRVDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	DXContext::Get().GetDevice()->CreateShaderResourceView(m_ZBuffer.GetTexture(), &rT_SRVDesc, rT_SRVHandle);
}

void FrameBuffer::CreateDepthBufferSRV(ID3D12DescriptorHeap* descriptorHeap, uint32_t heapIndex)
{
	// DEPTH SRV DESCRIPTOR HEAP
	if (m_D_SRVHeap  == nullptr && descriptorHeap == nullptr)
	{
		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDescD_SRV{};
		descriptorHeapDescD_SRV.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		descriptorHeapDescD_SRV.NumDescriptors = 1;
		descriptorHeapDescD_SRV.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		descriptorHeapDescD_SRV.NodeMask = 0;

		DXContext::Get().GetDevice()->CreateDescriptorHeap(&descriptorHeapDescD_SRV, IID_PPV_ARGS(&m_D_SRVHeap));

		std::string tempName = m_name + "_Depth_SRV";

		m_D_SRVHeap.Get()->SetName(std::wstring(tempName.begin(), tempName.end()).c_str());
	}
	else
	{
		m_D_SRVHeap = descriptorHeap;
		m_D_SRVHeapIndex = heapIndex;
	}
	
	CD3DX12_CPU_DESCRIPTOR_HANDLE depth_SRVHandle;
	uint32_t descriptorSize = DXContext::Get().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	depth_SRVHandle = m_D_SRVHeap->GetCPUDescriptorHandleForHeapStart();

	if (m_D_SRVHeapIndex > 0)
	{
		depth_SRVHandle.Offset(m_D_SRVHeapIndex, descriptorSize);
	}

	D3D12_SHADER_RESOURCE_VIEW_DESC dsrvDesc = {};
	dsrvDesc.Format = DXGI_FORMAT_R32_FLOAT;  // Use R32 because it's a depth-only texture
	dsrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	dsrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	dsrvDesc.Texture2D.MipLevels = 1;
		
	DXContext::Get().GetDevice()->CreateShaderResourceView(m_ZBuffer.GetTexture(), &dsrvDesc, depth_SRVHandle);
}

void FrameBuffer::BindRTV(ID3D12GraphicsCommandList*& cmdList)
{
	if (m_RTVHeap == nullptr)
	{
		D3EZ::EzException noDSVException = D3EZ::EzException(R"(FrameBuffer::BindRTV)", 163, R"(m_RTVHeap is nullptr)");
		return;
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle;
	uint32_t descriptorSize = DXContext::Get().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	rtvHandle = m_RTVHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.Offset(m_RTVHeapIndex, descriptorSize);

	float backgroundColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
	cmdList->ClearRenderTargetView(rtvHandle, backgroundColor, 0 , nullptr);
}

void FrameBuffer::BindDSV(ID3D12GraphicsCommandList*& cmdList)
{
	if (m_DSVHeap == nullptr)
	{
		D3EZ::EzException noDSVException = D3EZ::EzException(R"(FrameBuffer::BindDSV)", 182, R"(m_DSVHeap is nullptr)");
		return;
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle;
	uint32_t descriptorSize = DXContext::Get().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	dsvHandle = m_DSVHeap->GetCPUDescriptorHandleForHeapStart();
	dsvHandle.Offset(m_DSVHeapIndex, descriptorSize);

	cmdList->OMSetRenderTargets(0, nullptr, FALSE, &dsvHandle);
	cmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	

	//ViewPort
	m_viewPort.TopLeftX = 0;
	m_viewPort.TopLeftY = 0;
	m_viewPort.Width = m_width;
	m_viewPort.Height = m_height;
	m_viewPort.MinDepth = 0.0f;
	m_viewPort.MaxDepth = 1.0f;

	// Screeen Rect		
	m_rect.left = m_rect.top = 0;
	m_rect.right = m_width;
	m_rect.bottom = m_height;

	cmdList->RSSetViewports(1, &m_viewPort);
	cmdList->RSSetScissorRects(1, &m_rect);
}

void FrameBuffer::Bind(ID3D12GraphicsCommandList*& cmdList)
{	
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle;
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle;

	uint32_t descriptorSize = DXContext::Get().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	float backgroundColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	if (m_RTVHeap != nullptr)
	{		
		rtvHandle = m_RTVHeap->GetCPUDescriptorHandleForHeapStart();
		rtvHandle.Offset(m_RTVHeapIndex, descriptorSize);
	}
	else
	{
		D3EZ::EzException noRTVException = D3EZ::EzException(R"(FrameBuffer::Bind)", 211, R"(m_RTVHeap is nullptr)");
		return;
	}

	if (m_DSVHeap != nullptr)
	{
		dsvHandle = m_DSVHeap->GetCPUDescriptorHandleForHeapStart();
		dsvHandle.Offset(m_DSVHeapIndex, descriptorSize);
	}
	else
	{
		D3EZ::EzException noDSVException = D3EZ::EzException(R"(FrameBuffer::Bind)", 222, R"(m_DSVHeap is nullptr)");
		return;
	}

	cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
	cmdList->ClearRenderTargetView(rtvHandle, backgroundColor, 0, nullptr);
	cmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void FrameBuffer::Clear(ID3D12GraphicsCommandList*& cmdList)
{
	uint32_t descriptorSize = DXContext::Get().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	if (m_RTVHeap != nullptr)
	{
		const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle;
		rtvHandle = m_RTVHeap->GetCPUDescriptorHandleForHeapStart();
		rtvHandle.Offset(m_RTVHeapIndex, descriptorSize);

		cmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	}
	else
	{
		D3EZ::EzException noRTVException = D3EZ::EzException(R"(FrameBuffer::Clear)", 247, R"(m_RTVHeap is nullptr)");
		return;
	}

	if (m_ZBuffer.GetDescriptorHeap() != nullptr)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle;
		dsvHandle = m_DSVHeap->GetCPUDescriptorHandleForHeapStart();
		dsvHandle.Offset(m_DSVHeapIndex, descriptorSize);

		cmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}
	else
	{
		D3EZ::EzException noDSVException = D3EZ::EzException(R"(FrameBuffer::Clear)", 261, R"(m_DSVHeap is nullptr)");
		return;
	}
}