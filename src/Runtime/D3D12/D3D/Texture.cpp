#include "D3D/Texture.h"

Texture::Texture(std::vector<std::string>& paths, std::vector<std::string>& names, bool useMips)
{
	// === Texture Data === //
	if (paths.size() < 1)
	{
		D3EZ_EXCEPTION_WW("Texture cannot be created", "Texture count parameter < 1");
	}
	else
	{
		m_count = (uint32_t)paths.size();
		for (uint32_t i = 0; i < m_count; i++)
		{
			ImageLoader::ImageData imageData;
			ImageLoader::LoadImageFromDisk(paths[i], imageData, useMips);

			m_textures.push_back(nullptr);
			m_textureDatas.push_back(imageData);
			m_textureStrides.push_back(m_textureDatas[i].width * ((m_textureDatas[i].bitPerPixel + 7) / 8));
			m_textureSizes.push_back(m_textureDatas[i].height * m_textureStrides[i]);
			m_names.push_back(names[i]);
		}
	}	
}

Texture::Texture()
{
}

UINT Texture::GetMipSize(int textureIndex, int mipIndex)
{
	return (UINT)(m_textureSizes[textureIndex] / pow(4, mipIndex));
}

UINT64 Texture::Align(UINT64 value, UINT64 alignment)
{
	return (value + (alignment - 1)) & ~(alignment - 1);
}

void Texture::Init(D3D12_HEAP_PROPERTIES* defaultHeapProperties, DescriptorHeapAllocator* srvHeapAllocator)
{
	// Create D3D12 resource for each texture
	for (unsigned int i = 0; i < m_count; i++)
	{
		D3D12_RESOURCE_DESC rdt{};
		rdt.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		rdt.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		rdt.Width = m_textureDatas[i].width;
		rdt.Height = m_textureDatas[i].height;
		rdt.DepthOrArraySize = 1;
		rdt.MipLevels = m_textureDatas[i].mipsLevels;
		rdt.Format = m_textureDatas[i].giPixelFormat;
		rdt.SampleDesc.Count = 1;
		rdt.SampleDesc.Quality = 0;
		rdt.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		rdt.Flags = D3D12_RESOURCE_FLAG_NONE;
		
		DXContext::Get().GetDevice()->CreateCommittedResource(defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &rdt, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_textures[i]));

		std::wstring wideName = std::wstring(m_names[i].begin(), m_names[i].end());

		m_textures[i].Get()->SetName(wideName.c_str());
	}	

	// === SRV === //
	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle;
	uint32_t descriptorSize = DXContext::Get().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	if (srvHeapAllocator == nullptr)
	{
		//Textures Descriptor Heap
		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDescText{};
		descriptorHeapDescText.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		descriptorHeapDescText.NumDescriptors = m_count;
		descriptorHeapDescText.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		descriptorHeapDescText.NodeMask = 0;

		DXContext::Get().GetDevice()->CreateDescriptorHeap(&descriptorHeapDescText, IID_PPV_ARGS(&m_srvHeap));

		srvHandle = m_srvHeap->GetCPUDescriptorHandleForHeapStart();

		std::string temp = m_names[0] + " DescriptorHeap";
		std::wstring wideHeapName = std::wstring(temp.begin(), temp.end());

		m_srvHeap.Get()->SetName(wideHeapName.c_str());		
	}
	else
	{
		m_srvHeap = srvHeapAllocator->GetHeap();
	}	

	for (unsigned int i = 0; i < m_count; i++)
	{
		if (srvHeapAllocator != nullptr)
		{
			UINT heapIndex = srvHeapAllocator->Allocate();
			srvHandle = srvHeapAllocator->GetCPUHandle(heapIndex);
		}

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = m_textureDatas[i].giPixelFormat;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D.MipLevels = m_textureDatas[i].mipsLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.PlaneSlice = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

		DXContext::Get().GetDevice()->CreateShaderResourceView(m_textures[i], &srvDesc, srvHandle);
		if (srvHeapAllocator == nullptr)
		{
			srvHandle.Offset(1, descriptorSize);
		}		
	}
	// =========================== //
}

UINT64 Texture::CopyToGPU(ID3D12Resource* uploadBuffer, UINT64 uploadBufferOffset, ID3D12GraphicsCommandList* cmdList)
{
	// === Copy void* --> CPU Resource === //
	char* uploadBufferAdress;
	D3D12_RANGE uploadRange;
	uploadRange.Begin = uploadBufferOffset;
	uploadRange.End = uploadBufferOffset + GetTotalTextureSize();
	uploadBuffer->Map(0, &uploadRange, (void**)&uploadBufferAdress);

	for (uint32_t i = 0; i < m_count; i++)
	{
		for (unsigned int j = 0; j < m_textureDatas[i].mipsLevels; j++)
		{
			D3D12_SUBRESOURCE_DATA subresource = {};
			subresource.pData = GetTextureData(i, j);
			subresource.RowPitch = m_textureStrides[i] / pow(2, j);
			subresource.SlicePitch = (UINT)(subresource.RowPitch * (m_textureDatas[i].height / pow(2, j)));

			UINT mipHeight = (UINT)(m_textureDatas[i].height / pow(2, j));

			CHAR* srcPtr = GetTextureData(i, j);
			CHAR* dstPtr = &uploadBufferAdress[uploadBufferOffset];

			for (UINT row = 0; row < mipHeight; row++)
			{
				memcpy(dstPtr, srcPtr, subresource.RowPitch);

				srcPtr += subresource.RowPitch;
				dstPtr += Align(subresource.RowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
			}

			// Source
			D3D12_TEXTURE_COPY_LOCATION txtSrc;
			txtSrc.pResource = uploadBuffer;
			txtSrc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			txtSrc.PlacedFootprint.Offset = uploadBufferOffset;
			txtSrc.PlacedFootprint.Footprint.Width = (UINT)(m_textureDatas[i].width / pow(2, j));
			txtSrc.PlacedFootprint.Footprint.Height = mipHeight;
			txtSrc.PlacedFootprint.Footprint.Depth = 1;
			txtSrc.PlacedFootprint.Footprint.RowPitch = (UINT)max((m_textureStrides[i] / pow(2, j)), D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
			txtSrc.PlacedFootprint.Footprint.Format = m_textureDatas[i].giPixelFormat;

			// Destination
			D3D12_TEXTURE_COPY_LOCATION txtDst;
			txtDst.pResource = m_textures[i];
			txtDst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			txtDst.SubresourceIndex = j;
			
			////Box for when only copying part of the texture 
			//D3D12_BOX textureSizeAsBox;
			//textureSizeAsBox.left = textureSizeAsBox.top = textureSizeAsBox.front = 0;
			//textureSizeAsBox.right = m_textureDatas[i].width;
			//textureSizeAsBox.bottom = m_textureDatas[i].height;
			//textureSizeAsBox.back = 1;

			// === COPY === //
			// If Mip 0
			if (j == 0)
			{
				D3D12_RESOURCE_BARRIER transitionBarrier = {};
				transitionBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				transitionBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				transitionBarrier.Transition.pResource = m_textures[i];
				transitionBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				transitionBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_GENERIC_READ;
				transitionBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;

				cmdList->ResourceBarrier(1, &transitionBarrier);
			}

			cmdList->CopyTextureRegion(&txtDst, 0, 0, 0, &txtSrc, nullptr);

			//If last Mip 
			if (j == m_textureDatas[i].mipsLevels - 1)
			{
				D3D12_RESOURCE_BARRIER transitionBarrier = {};
				transitionBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				transitionBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				transitionBarrier.Transition.pResource = m_textures[i];
				transitionBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				transitionBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
				transitionBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;

				cmdList->ResourceBarrier(1, &transitionBarrier);
			}

			//Data offset
			//uploadBufferOffset += GetMipSize(i, j);
			uploadBufferOffset += Align(txtSrc.PlacedFootprint.Footprint.RowPitch * mipHeight, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
		}	
	}
	uploadBuffer->Unmap(0, &uploadRange);

	return uploadBufferOffset;
}

void Texture::AddCommands(ID3D12GraphicsCommandList*& cmdList, uint32_t rootParameterIndex)
{
	cmdList->SetDescriptorHeaps(1, &m_srvHeap);
	cmdList->SetGraphicsRootDescriptorTable(rootParameterIndex, m_srvHeap->GetGPUDescriptorHandleForHeapStart());
}

