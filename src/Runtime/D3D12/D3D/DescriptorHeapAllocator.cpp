#include "DescriptorHeapAllocator.h"

DescriptorHeapAllocator::DescriptorHeapAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT numDescriptors) : m_NumDescriptors(numDescriptors)
{
    // Describe the descriptor heap
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.Type = type;
    heapDesc.NumDescriptors = numDescriptors;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;  // Bindless needs visibility
    heapDesc.NodeMask = 0;

    // Create descriptor heap
    HRESULT hr = DXContext::Get().GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_DescriptorHeap));
    D3EZ::HRException("DescriptorHeapAllocator Constructor", 13, hr);

    m_DescriptorHeap.Get()->SetName(L"Bindless_SRV");

    // Get descriptor size
    m_DescriptorSize = DXContext::Get().GetDevice()->GetDescriptorHandleIncrementSize(type);

    // Initialize free list
    for (UINT i = 0; i < numDescriptors; i++)
        m_FreeDescriptors.push(i);
}

UINT DescriptorHeapAllocator::Allocate()
{
    if (m_FreeDescriptors.empty())
    {
        D3EZ::EzException("DescriptorHeapAllocator::Allocate", 28, "m_FreeDescriptors is empty !");
    }
    
    UINT index = m_FreeDescriptors.front();
    m_FreeDescriptors.pop();

    return index;
}

void DescriptorHeapAllocator::Free(UINT descriptorIndex)
{
    if (descriptorIndex >= m_NumDescriptors)
    {
        D3EZ::EzException("DescriptorHeapAllocator::Free", 41, "Invalid descriptor index !");
    }

    m_FreeDescriptors.push(descriptorIndex);
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeapAllocator::GetCPUHandle(UINT descriptorIndex)
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE handle;
    handle = m_DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    handle.Offset(descriptorIndex, m_DescriptorSize);
    
    return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeapAllocator::GetGPUHandle(UINT descriptorIndex)
{
    CD3DX12_GPU_DESCRIPTOR_HANDLE handle(m_DescriptorHeap->GetGPUDescriptorHandleForHeapStart());
    handle.Offset(descriptorIndex, m_DescriptorSize);
    return handle;
}
