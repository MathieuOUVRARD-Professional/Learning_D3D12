#include "DXContext.h"

#include <iostream>


/*
*	GPU
*	<--
*	Command Queue (WorkSet, WorkSet, Sync)
*	<--
*	CPU WorkSet(Setup, Setup, ChangeState, Setup, Draw) = CommandList
*
*/

bool DXContext::Init()
{
	if (FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device))))
	{
		return false;
	}

	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc{};
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
	cmdQueueDesc.NodeMask = 0;
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	if (FAILED(m_device->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&m_commandQueue))))
	{
		return false;
	}

	if (FAILED(m_device->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence))))
	{
		return false;
	}

	m_fenceEvent = CreateEvent(nullptr, false, false, nullptr);
	if (!m_fenceEvent)
	{
		return false;
	}

	if (FAILED(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator))))
	{
		return false;
	}

	if (FAILED(m_device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&m_commandList))))
	{
		return false;
	}

	return true;
}

void DXContext::Shutdown()
{
	if (m_fenceEvent)
	{
		CloseHandle(m_fenceEvent);
	}
	m_fence.Release();
	m_commandList.Release();
	m_commandAllocator.Release();
	m_commandQueue.Release();
	m_device.Release();
}

void DXContext::SignalAndWait()
{
	m_commandQueue->Signal(m_fence, ++m_fenceValue);

	/*while (m_fence->GetCompletedValue() < m_fenceValue)
	{
		BAD APPROACH
	}*/

	if (SUCCEEDED(m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent)))
	{
		if (WaitForSingleObject(m_fenceEvent, 20000) != WAIT_OBJECT_0)
		{
			std::cout << "DXContext::SignalAndWait | program wasn't able to find the fence event to signal";
			exit(-1);
		}
	}
	else
	{
		std::cout << "DXContext::SignalAndWait | program wasn't able to find the fence to signal";
		exit(-1);
	}

}

ID3D12GraphicsCommandList* DXContext::InitCommandList()
{
	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator, nullptr);
	return m_commandList;
}

void DXContext::ExecuteCommandList()
{
	if (SUCCEEDED(m_commandList->Close()))
	{
		ID3D12CommandList* lists[] = { m_commandList };
		m_commandQueue->ExecuteCommandLists(1, lists);
		SignalAndWait();
	}
	else
	{
		std::cout << "DXContext::ExecuteCommandList | wasn't able to close command list";
		exit(-1);
	}
}
