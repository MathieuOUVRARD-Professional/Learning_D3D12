#pragma once

#include "D3D/FrameBuffer.h"

class ColorBuffer : FrameBuffer
{
public:
	ColorBuffer(uint32_t width, uint32_t height, std::string name, D3D12_HEAP_PROPERTIES* defaultHeapProperties);
	~ColorBuffer();

	void CreateRTV(DescriptorHeapAllocator* rtvHeapAllocator = nullptr);
	void CreateRTV(ID3D12DescriptorHeap* dsvHeap, uint32_t heapIndex);
	void CreateColorBufferSRV(DescriptorHeapAllocator* heapAllocator = nullptr);

	void Bind(ID3D12GraphicsCommandList*& cmdList) override;
	void Resize(uint32_t width, uint32_t height) override;
	void Clear(ID3D12GraphicsCommandList*& cmdList) override;
	void Release() override;

	inline ID3D12Resource* GetTexture()
	{
		return m_RTV;
	}
	inline uint32_t Get_RT_SRVHeapIndex()
	{
		return m_RT_SRVHeapIndex;
	}

	float m_backgroundColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

private:
	void CreateCommitedResource();

	ComPointer<ID3D12Resource> m_RTV = nullptr;

	// === HEAPS === //
	ComPointer<ID3D12DescriptorHeap> m_RTV_Heap = nullptr;
	uint32_t m_RTV_HeapIndex = 0;

	ComPointer<ID3D12DescriptorHeap> m_RT_SRVHeap = nullptr;
	uint32_t m_RT_SRVHeapIndex = 0;


};