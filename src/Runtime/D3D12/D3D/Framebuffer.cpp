#include "FrameBuffer.h"

FrameBuffer::FrameBuffer(uint32_t width, uint32_t height, std::string name) : m_width(width), m_height(height), m_name(name)
{
	//ViewPort
	m_viewPort.TopLeftX = 0;
	m_viewPort.TopLeftY = 0;
	m_viewPort.Width = (float)m_width;
	m_viewPort.Height = (float)m_height;
	m_viewPort.MinDepth = 0.0f;
	m_viewPort.MaxDepth = 1.0f;

	// Screeen Rect
	m_rect.left = m_rect.top = 0;
	m_rect.right = m_width;
	m_rect.bottom = m_height;
}
//
//void FrameBuffer::Bind(ID3D12GraphicsCommandList*& cmdList)
//{	
//	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle;
//	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle;
//
//	uint32_t descriptorSize = DXContext::Get().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
//
//	float backgroundColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
//
//	if (m_RTV_Heap != nullptr)
//	{		
//		rtvHandle = m_RTV_Heap->GetCPUDescriptorHandleForHeapStart();
//		rtvHandle.Offset(m_RTVHeapIndex, descriptorSize);
//	}
//	else
//	{
//		D3EZ_EXCEPTION_W("m_RTV_Heap is nullptr)");
//		return;
//	}
//
//	if (m_DSV_Heap != nullptr)
//	{
//		dsvHandle = m_DSV_Heap->GetCPUDescriptorHandleForHeapStart();
//		dsvHandle.Offset(m_DSV_HeapIndex, descriptorSize);
//	}
//	else
//	{
//		D3EZ_EXCEPTION_W("m_DSV_Heap is nullptr)");
//		return;
//	}
//
//	cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
//	cmdList->ClearRenderTargetView(rtvHandle, backgroundColor, 0, nullptr);
//	cmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
//}
//
//void FrameBuffer::Clear(ID3D12GraphicsCommandList*& cmdList)
//{
//	uint32_t descriptorSize = DXContext::Get().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
//
//	if (m_RTV_Heap != nullptr)
//	{
//		const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
//
//		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle;
//		rtvHandle = m_RTV_Heap->GetCPUDescriptorHandleForHeapStart();
//		rtvHandle.Offset(m_RTVHeapIndex, descriptorSize);
//
//		cmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
//	}
//	else
//	{
//		D3EZ_EXCEPTION_W("m_RTV_Heap is nullptr)");
//		return;
//	}
//
//	if (m_ZBuffer.GetDescriptorHeap() != nullptr)
//	{
//		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle;
//		dsvHandle = m_DSV_Heap->GetCPUDescriptorHandleForHeapStart();
//		dsvHandle.Offset(m_DSV_HeapIndex, descriptorSize);
//
//		cmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
//	}
//	else
//	{
//		D3EZ_EXCEPTION_W("m_DSV_Heap is nullptr)");
//		return;
//	}
//}