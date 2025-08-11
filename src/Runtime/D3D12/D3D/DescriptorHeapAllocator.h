#pragma once

#include <D3D/DXContext.h>
#include <Support/ComPointer.h>
#include <Util/HRException.h>

#include <directx/d3dx12.h>

#include <d3d12.h>
#include <queue>
#include <vector>


class DescriptorHeapAllocator
{
	public:
        std::string m_name = "";

        DescriptorHeapAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT numDescriptors, std::string name);
        
        // Allocates a descriptor, returns its heap index
        UINT Allocate();

        // Frees a descriptor by index
        void Free(UINT descriptorIndex);

        // Returns CPU handle for a descriptor
        D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(UINT descriptorIndex);

        // Returns GPU handle for a descriptor
        D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(UINT descriptorIndex);

        // Get the descriptor heap pointer
        inline ID3D12DescriptorHeap* const GetHeap()
        { 
            return m_DescriptorHeap; 
        }

	private:
        ComPointer<ID3D12DescriptorHeap> m_DescriptorHeap;
        UINT m_DescriptorSize;
        UINT m_NumDescriptors;
        std::queue<UINT> m_FreeDescriptors;
};