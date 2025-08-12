#include "ColorBuffer.h"

ColorBuffer::ColorBuffer(uint32_t width, uint32_t height, std::string name, D3D12_HEAP_PROPERTIES* defaultHeapProperties) : FrameBuffer(width, height, name)
{
	m_defaultHeapProperties = defaultHeapProperties;
}

ColorBuffer::~ColorBuffer()
{
	Release();
}

void ColorBuffer::CreateRTV(DescriptorHeapAllocator* rtvHeapAllocator)
{
	// Create commited resource
	CreateCommitedResource();

	// RTV Descriptor Heap
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle;
	if (rtvHeapAllocator == nullptr) // No allocator provided --> Create RTV Heap to hold the RTV
	{
		// Decriptor Heap Desc
		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDescRTV{};
		descriptorHeapDescRTV.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		descriptorHeapDescRTV.NumDescriptors = 1;
		descriptorHeapDescRTV.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		descriptorHeapDescRTV.NodeMask = 0;

		// Create RTV Descriptor Heap
		DXContext::Get().GetDevice()->CreateDescriptorHeap(&descriptorHeapDescRTV, IID_PPV_ARGS(&m_RTV_Heap));

		// Naming
		std::string name = m_name + "_RTV_Heap";
		m_RTV_Heap->SetName(std::wstring(name.begin(), name.end()).c_str());
	}
	else // Using allocator --> Allocate a new slot so the allocator Heap holds the DSV
	{
		m_RTV_Heap = rtvHeapAllocator->GetHeap();

		m_RTV_HeapIndex = rtvHeapAllocator->Allocate();
		rtvHandle = rtvHeapAllocator->GetCPUHandle(m_RTV_HeapIndex);
	}

	// === RTV === //
	D3D12_RENDER_TARGET_VIEW_DESC rtv{};
	rtv.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtv.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtv.Texture2D.MipSlice = 0;
	rtv.Texture2D.PlaneSlice = 0;

	DXContext::Get().GetDevice()->CreateRenderTargetView(m_RTV, nullptr, rtvHandle);
}

void ColorBuffer::CreateRTV(ID3D12DescriptorHeap* rtvHeap, uint32_t heapIndex)
{
	// Create commited resource
	CreateCommitedResource();

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle;
	uint32_t descriptorSize = DXContext::Get().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.Offset(heapIndex, descriptorSize);

	// === RTV === //
	D3D12_RENDER_TARGET_VIEW_DESC rtv{};
	rtv.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtv.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtv.Texture2D.MipSlice = 0;
	rtv.Texture2D.PlaneSlice = 0;

	DXContext::Get().GetDevice()->CreateRenderTargetView(m_RTV, nullptr, rtvHandle);
}

void ColorBuffer::CreateColorBufferSRV(DescriptorHeapAllocator* heapAllocator)
{
	// RT SRV DESCRIPTOR HEAP
	CD3DX12_CPU_DESCRIPTOR_HANDLE renderTarget_SRVHandle;
	if (m_RT_SRVHeap == nullptr) // No assigned RT SRV Heap --> Associate one
	{
		if (heapAllocator == nullptr) // No allocator provided --> Create RT SRV Heap to hold the RT SRV
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
		else // No assigned RT SRV Heap but allocator provided --> Alocate a new slot with provided allocator to hold the RT SRV
		{
			m_RT_SRVHeap = heapAllocator->GetHeap();

			m_RT_SRVHeapIndex = heapAllocator->Allocate();
			renderTarget_SRVHandle = heapAllocator->GetCPUHandle(m_RT_SRVHeapIndex);
		}

		D3D12_SHADER_RESOURCE_VIEW_DESC rT_SRVDesc = {};
		rT_SRVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // Use R32 because it's a depth-only texture
		rT_SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		rT_SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		rT_SRVDesc.Texture2D.MipLevels = 1;
		rT_SRVDesc.Texture2D.MostDetailedMip = 0;
		rT_SRVDesc.Texture2D.PlaneSlice = 0;
		rT_SRVDesc.Texture2D.ResourceMinLODClamp = 0.0f;

		DXContext::Get().GetDevice()->CreateShaderResourceView(m_RTV, &rT_SRVDesc, renderTarget_SRVHandle);
	}
	else
	{
		D3EZ_EXCEPTION_W(std::format("Trying to create a Render Target SRV for ColorBuffer: {}, but it already has one!", m_name))
	}
}

void ColorBuffer::Bind(ID3D12GraphicsCommandList*& cmdList)
{
	if (m_RTV_Heap == nullptr)
	{
		D3EZ_EXCEPTION_W(std::format("Trying to bind {} Render Target but m_RTV_Heap is nullptr", m_name));
		return;
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle;
	uint32_t descriptorSize = DXContext::Get().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	rtvHandle = m_RTV_Heap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.Offset(m_RTV_HeapIndex, descriptorSize);


	cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
	cmdList->ClearRenderTargetView(rtvHandle, m_backgroundColor, 0, nullptr);
}

void ColorBuffer::Resize(uint32_t width, uint32_t height)
{
	m_width = width;
	m_height = height;

	if (m_RTV != nullptr) // RTV already created --> Need to recreate it at the same place
	{
		this->CreateRTV(m_RTV_Heap, m_RTV_HeapIndex);

		if (m_RT_SRVHeap != nullptr) // SRV Already created --> Needs to recreate it at the same place
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE depth_SRVHandle;
			uint32_t descriptorSize = DXContext::Get().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			depth_SRVHandle = m_RT_SRVHeap->GetCPUDescriptorHandleForHeapStart();
			depth_SRVHandle.Offset(m_RT_SRVHeapIndex, descriptorSize);

			D3D12_SHADER_RESOURCE_VIEW_DESC dsrvDesc = {};
			dsrvDesc.Format = DXGI_FORMAT_R32_FLOAT; // Use R32 because it's a depth-only texture
			dsrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			dsrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			dsrvDesc.Texture2D.MipLevels = 1;

			DXContext::Get().GetDevice()->CreateShaderResourceView(m_RTV, &dsrvDesc, depth_SRVHandle);
		}
	}
}

void ColorBuffer::Clear(ID3D12GraphicsCommandList*& cmdList)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle;
	uint32_t descriptorSize = DXContext::Get().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	rtvHandle = m_RTV_Heap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.Offset(m_RTV_HeapIndex, descriptorSize);

	cmdList->ClearRenderTargetView(rtvHandle, m_backgroundColor, 0, nullptr);

}

void ColorBuffer::Release()
{
	m_RT_SRVHeap.Release();
	m_RTV_Heap.Release();
	m_RTV.Release();
}

void ColorBuffer::CreateCommitedResource()
{
	// Buffer
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

	// Create Resource
	DXContext::Get().GetDevice()->CreateCommittedResource(m_defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &frameBufferDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, IID_PPV_ARGS(&m_RTV));

	// Naming
	std::string name = m_name + "_RTV";
	m_RTV->SetName(std::wstring(name.begin(), name.end()).c_str());
}
