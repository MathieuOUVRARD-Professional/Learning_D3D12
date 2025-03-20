#pragma once

#include <directx/d3dx12.h>

#include <Support/WinInclude.h>
#include <Support/ComPointer.h>

#include <D3D/DescriptorHeapAllocator.h>
#include <D3D/DXContext.h>

#include <string>

class ZBuffer
{
	public:
		std::string m_name ="";
		UINT m_heapIndex = 0;

		ZBuffer();
		ZBuffer(D3D12_HEAP_PROPERTIES* defaultHeapProperties, std::string name, uint32_t width, uint32_t height, DescriptorHeapAllocator* = nullptr);
		inline void Resize(uint32_t width, uint32_t height)
		{
			*this = ZBuffer::ZBuffer(m_defaultHeapProperties, m_name, width, height);
		}
		inline ID3D12DescriptorHeap* GetDescriptorHeap()
		{
			return m_dsvHeap;
		};
		inline ID3D12Resource* GetTexture()
		{
			return m_depth;
		}
		inline void Release()
		{
			m_depth.Release();
			m_dsvHeap.Release();
		}

	private:
		D3D12_HEAP_PROPERTIES* m_defaultHeapProperties = nullptr;

		ComPointer<ID3D12DescriptorHeap> m_dsvHeap = nullptr;
		ComPointer<ID3D12Resource> m_depth = nullptr;
};