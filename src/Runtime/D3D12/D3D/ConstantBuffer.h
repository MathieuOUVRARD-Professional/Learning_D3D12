#pragma once

#include <Support/WinInclude.h>
#include <Support/ComPointer.h>

#include <D3D/DXContext.h>
#include <D3D/DescriptorHeapAllocator.h>

#include <directx/d3dx12.h>
#include <string>

class ConstantBuffer
{
public:
	ConstantBuffer();
	ConstantBuffer(UINT size, std::string name);

	void CreateResource();
	void CopyData(const void* dataPointer, UINT manualSize = 0);
	void CreateCBV(DescriptorHeapAllocator* heapAllocator = nullptr);
	void Update(const void* dataPointer, UINT dataSize);

	inline UINT GetSize()
	{
		return m_size;
	}
	inline ComPointer<ID3D12Resource> GetData()
	{
		return m_data;
	}
	inline D3D12_CPU_DESCRIPTOR_HANDLE & GetCBV_Handle()
	{
		return m_CBV_handle;
	}

	std::string m_name = "";
private:
	UINT m_size = 0;

	ComPointer<ID3D12Resource> m_data = nullptr;

	CD3DX12_CPU_DESCRIPTOR_HANDLE m_CBV_handle;

	ComPointer<ID3D12DescriptorHeap> m_CBV_Heap = nullptr;
	uint32_t m_CBV_heapIndex = 0;
};