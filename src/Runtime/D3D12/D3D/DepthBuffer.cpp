#include "DepthBuffer.h"

DepthBuffer::DepthBuffer(uint32_t width, uint32_t height, std::string name, D3D12_HEAP_PROPERTIES* defaultHeapProperties) : FrameBuffer(width, height, name)
{
	m_defaultHeapProperties = defaultHeapProperties;
}

DepthBuffer::~DepthBuffer()
{
	Release();
}

void DepthBuffer::CreateDSV(DescriptorHeapAllocator* dsvHeapAllocator)
{
	// Create commited resource
	CreateCommitedResource();

	// DSV Descriptor Heap
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle;
	if (dsvHeapAllocator == nullptr) // No allocator provided --> Create DSV Heap to hold the DSV
	{
		// Decriptor Heap Desc
		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDescDepth{};
		descriptorHeapDescDepth.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		descriptorHeapDescDepth.NumDescriptors = 1;
		descriptorHeapDescDepth.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		descriptorHeapDescDepth.NodeMask = 0;

		// Create DSV Descriptor Heap
		DXContext::Get().GetDevice()->CreateDescriptorHeap(&descriptorHeapDescDepth, IID_PPV_ARGS(&m_DSV_Heap));

		// Naming
		std::string name = m_name+"_DSV_Heap";
		m_DSV_Heap->SetName(std::wstring(name.begin(), name.end()).c_str());

		dsvHandle = m_DSV_Heap->GetCPUDescriptorHandleForHeapStart();
	}
	else // Using allocator --> Allocator Heap holds the DSV
	{
		m_DSV_Heap = dsvHeapAllocator->GetHeap();

		m_DSV_HeapIndex = dsvHeapAllocator->Allocate();
		dsvHandle = dsvHeapAllocator->GetCPUHandle(m_DSV_HeapIndex);
	}

	// === DSV === //
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.Texture2D.MipSlice = 0;

	DXContext::Get().GetDevice()->CreateDepthStencilView(m_DSV, &dsvDesc, dsvHandle);
}

void DepthBuffer::CreateDepthBufferSRV(DescriptorHeapAllocator* srvHeapAllocator)
{
	// DEPTH SRV DESCRIPTOR HEAP
	CD3DX12_CPU_DESCRIPTOR_HANDLE depth_SRVHandle;
	if (m_D_SRVHeap == nullptr) // No assigned Depth SRV Heap --> Associate one
	{
		if (srvHeapAllocator == nullptr) // No allocator provided --> Create Depth SRV Heap to hold the Depth SRV
		{
			D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDescD_SRV{};
			descriptorHeapDescD_SRV.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			descriptorHeapDescD_SRV.NumDescriptors = 1;
			descriptorHeapDescD_SRV.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			descriptorHeapDescD_SRV.NodeMask = 0;

			DXContext::Get().GetDevice()->CreateDescriptorHeap(&descriptorHeapDescD_SRV, IID_PPV_ARGS(&m_D_SRVHeap));

			std::string tempName = m_name + "_Depth_SRV";

			m_D_SRVHeap.Get()->SetName(std::wstring(tempName.begin(), tempName.end()).c_str());

			depth_SRVHandle = m_D_SRVHeap->GetCPUDescriptorHandleForHeapStart();
		}
		else // No assigned Depth SRV Heap but allocator provided --> Alocate a new slot with provided allocator to hold the Depth SRV
		{
			m_D_SRVHeap = srvHeapAllocator->GetHeap();

			m_D_SRVHeapIndex = srvHeapAllocator->Allocate();
			depth_SRVHandle = srvHeapAllocator->GetCPUHandle(m_D_SRVHeapIndex);
		}

		D3D12_SHADER_RESOURCE_VIEW_DESC dsrvDesc = {};
		dsrvDesc.Format = DXGI_FORMAT_R32_FLOAT; // Use R32 because it's a depth-only texture
		dsrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		dsrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		dsrvDesc.Texture2D.MipLevels = 1;

		DXContext::Get().GetDevice()->CreateShaderResourceView(m_DSV, &dsrvDesc, depth_SRVHandle);
	}
	else
	{
		D3EZ_EXCEPTION_W(fmt::format("Trying to create a Depth SRV for DepthBuffer: %s, but it already has one!", m_name));
	}
}

void DepthBuffer::Bind(ID3D12GraphicsCommandList*& cmdList)
{
	if (m_DSV_Heap == nullptr)
	{
		D3EZ_EXCEPTION_W("m_DSV_Heap is nullptr");
		return;
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle;
	uint32_t descriptorSize = DXContext::Get().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	dsvHandle = m_DSV_Heap->GetCPUDescriptorHandleForHeapStart();
	dsvHandle.Offset(m_DSV_HeapIndex, descriptorSize);

	cmdList->OMSetRenderTargets(0, nullptr, false, &dsvHandle);
	cmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	cmdList->RSSetViewports(1, &m_viewPort);
	cmdList->RSSetScissorRects(1, &m_rect);
}

void DepthBuffer::Resize(uint32_t width, uint32_t height) 
{
	m_width  = width;
	m_height = height;

	if (m_DSV != nullptr) // DSV already created --> Need to recreate it 
	{
		this->RecreateDSV(m_DSV_Heap, m_DSV_HeapIndex);

		if (m_D_SRVHeap != nullptr) // SRV Already created --> Needs to recreate it at the same place
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE depth_SRVHandle;
			uint32_t descriptorSize = DXContext::Get().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			depth_SRVHandle = m_D_SRVHeap->GetCPUDescriptorHandleForHeapStart();
			depth_SRVHandle.Offset(m_D_SRVHeapIndex, descriptorSize);

			D3D12_SHADER_RESOURCE_VIEW_DESC dsrvDesc = {};
			dsrvDesc.Format = DXGI_FORMAT_R32_FLOAT; // Use R32 because it's a depth-only texture
			dsrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			dsrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			dsrvDesc.Texture2D.MipLevels = 1;

			DXContext::Get().GetDevice()->CreateShaderResourceView(m_DSV, &dsrvDesc, depth_SRVHandle);
		}
	}
	else
	{
		D3EZ_EXCEPTION_W(fmt::format("Trying to resize DepthBuffer: %s but DSV = nullprt !", m_name));
	}
}

void DepthBuffer::Clear(ID3D12GraphicsCommandList*& cmdList)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle;
	uint32_t descriptorSize = DXContext::Get().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	dsvHandle = m_DSV_Heap->GetCPUDescriptorHandleForHeapStart();
	dsvHandle.Offset(m_DSV_HeapIndex, descriptorSize);

	cmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void DepthBuffer::Release()
{
	m_D_SRVHeap.Release();
	m_DSV_Heap.Release();
	m_DSV.Release();
}

D3D12_CPU_DESCRIPTOR_HANDLE DepthBuffer::GetDSVHandle()
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle;
	uint32_t descriptorSize = DXContext::Get().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	dsvHandle = m_DSV_Heap->GetCPUDescriptorHandleForHeapStart();
	dsvHandle.Offset(m_DSV_HeapIndex, descriptorSize);

	return dsvHandle;
}

void DepthBuffer::CreateCommitedResource()
{
	// Buffer
	D3D12_RESOURCE_DESC rdd{};
	rdd.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	rdd.Alignment = 0;
	rdd.Width = m_width;
	rdd.Height = m_height;
	rdd.DepthOrArraySize = 1;
	rdd.MipLevels = 1;
	rdd.Format = DXGI_FORMAT_D32_FLOAT;
	rdd.SampleDesc.Count = 1;
	rdd.SampleDesc.Quality = 0;
	rdd.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	rdd.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	// Describe clear values
	D3D12_CLEAR_VALUE clearValueDs;
	ZeroMemory(&clearValueDs, sizeof(D3D12_CLEAR_VALUE));
	clearValueDs.Format = DXGI_FORMAT_D32_FLOAT;
	clearValueDs.DepthStencil.Depth = 1.0f;
	clearValueDs.DepthStencil.Stencil = 0;

	// Create Resource
	DXContext::Get().GetDevice()->CreateCommittedResource(m_defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &rdd, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValueDs, IID_PPV_ARGS(&m_DSV));

	//Naming
	std::string name = m_name + "_DSV";
	m_DSV->SetName(std::wstring(name.begin(), name.end()).c_str());
}

void DepthBuffer::RecreateDSV(ID3D12DescriptorHeap* dsvHeap, uint32_t heapIndex)
{
	// Release old DSV
	m_DSV.Release();

	// Create commited resource
	CreateCommitedResource();

	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle;
	uint32_t descriptorSize = DXContext::Get().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	dsvHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();
	dsvHandle.Offset(heapIndex, descriptorSize);

	// === DSV === //
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.Texture2D.MipSlice = 0;

	DXContext::Get().GetDevice()->CreateDepthStencilView(m_DSV, &dsvDesc, dsvHandle);
}