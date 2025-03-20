#include <D3D/ZBuffer.h>

ZBuffer::ZBuffer()
{
	m_defaultHeapProperties = nullptr;
	m_depth = nullptr;
	m_dsvHeap = nullptr;
}

ZBuffer::ZBuffer(D3D12_HEAP_PROPERTIES* defaultHeapProperties, std::string name, uint32_t width, uint32_t height, DescriptorHeapAllocator* srvHeapAllocator)
{
	m_defaultHeapProperties = defaultHeapProperties;
	m_name = name;

	//Buffer
	D3D12_RESOURCE_DESC rdd{};
	rdd.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	rdd.Alignment = 0;
	rdd.Width = width;
	rdd.Height = height;
	rdd.DepthOrArraySize = 1;
	rdd.MipLevels = 1;
	rdd.Format = DXGI_FORMAT_D32_FLOAT;
	rdd.SampleDesc.Count = 1;
	rdd.SampleDesc.Quality = 0;
	rdd.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	rdd.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	//Describe clear values
	D3D12_CLEAR_VALUE clearValueDs;
	ZeroMemory(&clearValueDs, sizeof(D3D12_CLEAR_VALUE));
	clearValueDs.Format = DXGI_FORMAT_D32_FLOAT;
	clearValueDs.DepthStencil.Depth = 1.0f;
	clearValueDs.DepthStencil.Stencil = 0;
	
	// Create Ressource
	DXContext::Get().GetDevice()->CreateCommittedResource(defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &rdd, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValueDs, IID_PPV_ARGS(&m_depth));
	m_depth->SetName(std::wstring(m_name.begin(), m_name.end()).c_str());

	// Heap
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle;
	if (srvHeapAllocator == nullptr)
	{
		// Descriptor Heap
		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDescDepth{};
		descriptorHeapDescDepth.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		descriptorHeapDescDepth.NumDescriptors = 1;
		descriptorHeapDescDepth.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		descriptorHeapDescDepth.NodeMask = 0;

		DXContext::Get().GetDevice()->CreateDescriptorHeap(&descriptorHeapDescDepth, IID_PPV_ARGS(&m_dsvHeap));		
		m_dsvHeap->SetName(std::wstring(m_name.begin(), m_name.end()).c_str());

		dsvHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
	}
	else
	{
		m_dsvHeap = srvHeapAllocator->GetHeap();

		m_heapIndex = srvHeapAllocator->Allocate();
		dsvHandle = srvHeapAllocator->GetCPUHandle(m_heapIndex);
	}

	uint32_t descriptorSize = DXContext::Get().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// === DSV === //
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.Texture2D.MipSlice = 0;

	DXContext::Get().GetDevice()->CreateDepthStencilView(m_depth, &dsvDesc, dsvHandle);
}