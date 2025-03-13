#pragma once

#include <Support/WinInclude.h>
#include <Support/ComPointer.h>

#include <D3D/DXContext.h>

class ZBuffer
{
	public:
		ZBuffer();
		ZBuffer(D3D12_HEAP_PROPERTIES* defaultHeapProperties, UINT width, UINT height);
		inline void Resize(UINT width, UINT height)
		{
			*this = ZBuffer::ZBuffer(m_defaultHeapProperties, width, height);
		}
		inline ID3D12DescriptorHeap* GetDescriptorHeap()
		{
			return m_dsvHeap;
		};
		inline void Release()
		{
			m_depth.Release();
			m_dsvHeap.Release();
		}

	private:
		D3D12_HEAP_PROPERTIES* m_defaultHeapProperties = nullptr;
		ComPointer<ID3D12Resource> m_depth = nullptr;
		ComPointer<ID3D12DescriptorHeap> m_dsvHeap = nullptr;
};