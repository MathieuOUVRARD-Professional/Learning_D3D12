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

	protected:
		virtual void Bind(ID3D12GraphicsCommandList*& cmdList) = 0;
		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual void Clear(ID3D12GraphicsCommandList*& cmdList) = 0;
		virtual void Release() = 0;

		uint32_t m_width = 0;
		uint32_t m_height = 0;

		D3D12_HEAP_PROPERTIES* m_defaultHeapProperties = nullptr;
		D3D12_VIEWPORT m_viewPort = D3D12_VIEWPORT{};
		RECT m_rect = D3D12_RECT();;


		//ZBuffer m_ZBuffer;
};