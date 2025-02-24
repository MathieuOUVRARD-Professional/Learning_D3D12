#pragma once

#include <Support/WinInclude.h>
#include <Support/ComPointer.h>
#include <Support/Window.h>

#include <D3D/DXContext.h>

class ZBuffer
{
	public:
		ZBuffer(D3D12_HEAP_PROPERTIES* defaultHeapProperties);
		inline ID3D12DescriptorHeap* GetDescriptorHeap()
		{
			return m_dsvHeap;
		};

	private:
		ComPointer<ID3D12Resource> m_depth;
		ComPointer<ID3D12DescriptorHeap> m_dsvHeap;
};