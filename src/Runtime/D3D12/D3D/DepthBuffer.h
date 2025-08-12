#pragma once

#include "D3D/FrameBuffer.h"

class DepthBuffer : FrameBuffer
{
public:
	DepthBuffer(uint32_t width, uint32_t height, std::string name, D3D12_HEAP_PROPERTIES* defaultHeapProperties);
	~DepthBuffer();

	void CreateDSV( DescriptorHeapAllocator* dsvHeapAllocator = nullptr);
	void CreateDSV( ID3D12DescriptorHeap* dsvHeap, uint32_t heapIndex);
	void CreateDepthBufferSRV(DescriptorHeapAllocator* srvHeapAllocator = nullptr);

	void Bind(ID3D12GraphicsCommandList*& cmdList) override;
	void Resize(uint32_t width, uint32_t height) override;
	void Clear(ID3D12GraphicsCommandList*& cmdList) override;
	void Release() override;

	inline ID3D12Resource* GetTexture()
	{
		return m_DSV;
	}
	inline uint32_t Get_D_SRVHeapIndex()
	{
		return m_D_SRVHeapIndex;
	}

private:
	void CreateCommitedResource();

	ComPointer<ID3D12Resource> m_DSV = nullptr;

	// === HEAPS === //
	ComPointer<ID3D12DescriptorHeap> m_DSV_Heap = nullptr;
	uint32_t m_DSV_HeapIndex = 0;

	ComPointer<ID3D12DescriptorHeap> m_D_SRVHeap = nullptr;
	uint32_t m_D_SRVHeapIndex = 0;	
};