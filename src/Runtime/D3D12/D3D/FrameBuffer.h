#pragma once

#include <Support/WinInclude.h>
#include <Support/ComPointer.h>

#include <D3D/DescriptorHeapAllocator.h>
#include <D3D/DXContext.h>
#include <D3D/ZBuffer.h>

#include "Util/EzException.h"
#include <string>

class FrameBuffer
{
	public:
		std::string m_name = "";

		FrameBuffer(uint32_t width, uint32_t height, std::string name);
		void RenderTargetBuffer(D3D12_HEAP_PROPERTIES* defaultHeapProperties, ID3D12DescriptorHeap* descriptorHeap = nullptr, uint32_t heapIndex = 0);
		void DepthBuffer(D3D12_HEAP_PROPERTIES* defaultHeapProperties, DescriptorHeapAllocator* srvHeapAllocator = nullptr);

		void CreateRenderTargetSRV(DescriptorHeapAllocator* heapAllocator = nullptr);
		void CreateDepthBufferSRV(DescriptorHeapAllocator* heapAllocator = nullptr);

		inline uint32_t Get_RT_SRVHeapIndex()
		{
			return m_RT_SRVHeapIndex;
		}
		inline uint32_t Get_D_SRVHeapIndex()
		{
			return m_D_SRVHeapIndex;
		}

		void BindRTV(ID3D12GraphicsCommandList*& cmdList);
		void BindDSV(ID3D12GraphicsCommandList*& cmdList);

		void Bind(ID3D12GraphicsCommandList*& cmdList);
		void Clear(ID3D12GraphicsCommandList*& cmdList);


	private:
		uint32_t m_width = 0;
		uint32_t m_height = 0;
		
		// === HEAPS === //
		// RenderTarget
		ComPointer<ID3D12DescriptorHeap> m_RTV_Heap = nullptr;
		uint32_t m_RTVHeapIndex = 0;

		ComPointer<ID3D12DescriptorHeap> m_RT_SRVHeap = nullptr;
		uint32_t m_RT_SRVHeapIndex = 0;	
		
		//Depth
		ComPointer<ID3D12DescriptorHeap> m_DSV_Heap = nullptr;
		uint32_t m_DSVHeapIndex = 0;

		ComPointer<ID3D12DescriptorHeap> m_D_SRVHeap = nullptr;
		uint32_t m_D_SRVHeapIndex = 0;

		ComPointer<ID3D12Resource> m_RTV = nullptr;

		ZBuffer m_ZBuffer;

		D3D12_VIEWPORT m_viewPort;
		RECT m_rect;
};