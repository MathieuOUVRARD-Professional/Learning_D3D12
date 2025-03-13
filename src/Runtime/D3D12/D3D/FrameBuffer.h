#pragma once

#include <Support/WinInclude.h>
#include <Support/ComPointer.h>

#include <D3D/DXContext.h>
#include <D3D/ZBuffer.h>

class FrameBuffer
{
	public:
		FrameBuffer(uint32_t width, uint32_t height);
		void RenderTargetBuffer(D3D12_HEAP_PROPERTIES* defaultHeapProperties);
		void DepthBuffer(D3D12_HEAP_PROPERTIES* defaultHeapProperties);
		void Clear(ID3D12GraphicsCommandList*& cmdList);

	private:
		uint32_t m_width = 0;
		uint32_t m_height = 0;

		ComPointer<ID3D12Resource> m_frameBuffer = nullptr;
		ComPointer<ID3D12DescriptorHeap> m_rtvHeap = nullptr;

		ZBuffer m_ZBuffer;
};