#include "ConstantBuffer.h"

ConstantBuffer::ConstantBuffer()
{
}

ConstantBuffer::ConstantBuffer(UINT size, std::string name) : m_name(name)
{
	// Aligning size to 255 bytes
	m_size = (size + 255) & ~255;
}

void ConstantBuffer::CreateResource()
{
	// Describe the constant buffer
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD; // Upload heap (CPU -> GPU)

	D3D12_RESOURCE_DESC bufferDesc = {};
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Width = m_size;
	bufferDesc.Height = 1;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	// Create the buffer resource
	DXContext::Get().GetDevice()->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_data)
	);

	m_data->SetName(std::wstring(m_name.begin(), m_name.end()).c_str());

}

void ConstantBuffer::CopyData(const void * dataPointer, UINT manualSize)
{
	// Map the constant buffer
	char* mappedData;
	D3D12_RANGE uploadRange;
	uploadRange.Begin = 0;
	if (manualSize != 0)
	{
		manualSize = (manualSize + 255) & ~255;
		uploadRange.End = manualSize;
	}
	else
	{
		uploadRange.End = m_size;
	}
	m_data->Map(0, &uploadRange, (void**)&mappedData);

	// Copy
	memcpy(&mappedData[0], dataPointer, uploadRange.End - uploadRange.Begin);

	//Unmap
	m_data->Unmap(0, &uploadRange);
}

void ConstantBuffer::CreateCBV(DescriptorHeapAllocator * heapAllocator)
{
	if (heapAllocator == nullptr) // No allocator provided --> Create CBV/SRV/UAV Heap to hold the CBV
	{
		// Decriptor Heap Desc
		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDescDepth{};
		descriptorHeapDescDepth.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		descriptorHeapDescDepth.NumDescriptors = 1;
		descriptorHeapDescDepth.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		descriptorHeapDescDepth.NodeMask = 0;

		// Create DSV Descriptor Heap
		DXContext::Get().GetDevice()->CreateDescriptorHeap(&descriptorHeapDescDepth, IID_PPV_ARGS(&m_CBV_Heap));

		// Naming
		std::string name = m_name + "_CBV_Heap";
		m_CBV_Heap->SetName(std::wstring(name.begin(), name.end()).c_str());

		m_CBV_handle = m_CBV_Heap->GetCPUDescriptorHandleForHeapStart();
	}
	else // Using allocator --> Allocator Heap holds the CBV
	{
		m_CBV_Heap = heapAllocator->GetHeap();

		m_CBV_heapIndex = heapAllocator->Allocate();
		m_CBV_handle = heapAllocator->GetCPUHandle(m_CBV_heapIndex);
	}

	// === CBV === //
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = m_data->GetGPUVirtualAddress(); // GPU address of buffer
	cbvDesc.SizeInBytes = m_size;							// Must be 256-byte aligned	

	// Create the CBV in the descriptor heap
	DXContext::Get().GetDevice()->CreateConstantBufferView(&cbvDesc, m_CBV_handle);
}

void ConstantBuffer::Update(const void * dataPointer, UINT dataSize)
{
	if (m_data != nullptr)
	{
		CopyData(dataPointer, dataSize);
	}
	else
	{
		std::string errorMessage = "Trying to update ConstantBuffer: " + m_name + "'s data but the pointer is nullptr.\n";
		D3EZ_EXCEPTION_W(errorMessage)
	}
}
