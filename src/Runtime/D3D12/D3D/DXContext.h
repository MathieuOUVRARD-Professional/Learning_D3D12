#pragma once

#include <Support/WinInclude.h>
#include <Support/ComPointer.h>

class DXContext
{
public:
	bool Init();
	void Shutdown();

	void SignalAndWait();
	ID3D12GraphicsCommandList* InitCommandList();
	void ExecuteCommandList();

	inline void Flush(size_t count)
	{
		for (size_t i = 0; i < count; i++)
		{
			SignalAndWait();
		}
	}

	inline ComPointer<IDXGIFactory7>& GetFactory()
	{
		return m_dxgiFactory;
	}
	inline ComPointer<ID3D12Device8>& GetDevice()
	{
		return m_device;
	}
	inline ComPointer<ID3D12CommandQueue>& GetCommandQueue()
	{
		return m_commandQueue;
	}

private:
	ComPointer<IDXGIFactory7> m_dxgiFactory;

	ComPointer<ID3D12Device8> m_device;
	ComPointer<ID3D12CommandQueue> m_commandQueue;

	ComPointer<ID3D12CommandAllocator> m_commandAllocator;
	ComPointer<ID3D12GraphicsCommandList> m_commandList;

	ComPointer<ID3D12Fence1> m_fence;
	UINT64 m_fenceValue = 0;
	HANDLE m_fenceEvent = nullptr;

	//Singleton 
public:
	DXContext(const DXContext&) = delete;
	DXContext& operator=(const DXContext&) = delete;
	inline static DXContext& Get()
	{
		static DXContext instance;
		return instance;
	}

private:
	DXContext() = default;
};