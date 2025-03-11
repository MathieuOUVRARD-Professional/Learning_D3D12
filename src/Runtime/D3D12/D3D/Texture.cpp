#include "D3D/Texture.h"
#include <iostream>

Texture::Texture(std::vector<std::string>& paths, std::vector<std::string>& names)
{
	// === Texture Data === //
	if (paths.size() < 1)
	{
		D3EZ::EzException::EzException("Texture::Texture", 1, "Texture cannot be created", "Texture count parameter < 1");
	}
	else
	{
		m_count = (uint32_t)paths.size();
		for (uint32_t i = 0; i < m_count; i++)
		{
			ImageLoader::ImageData imageData;
			ImageLoader::LoadImageFromDisk(paths[i], imageData);

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

void Texture::Init(D3D12_HEAP_PROPERTIES* defaultHeapProperties, ID3D12DescriptorHeap* bindlessSRVHeap, uint32_t bindlessSRVIndex)
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
		rdt.MipLevels = 1;
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
	if (bindlessSRVHeap == nullptr)
	{
		//Textures Descriptor Heap
		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDescText{};
		descriptorHeapDescText.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		descriptorHeapDescText.NumDescriptors = m_count;
		descriptorHeapDescText.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		descriptorHeapDescText.NodeMask = 0;

		DXContext::Get().GetDevice()->CreateDescriptorHeap(&descriptorHeapDescText, IID_PPV_ARGS(&m_srvHeap));
		std::string temp = m_names[0] + " DescriptorHeap";
		std::wstring wideHeapName = std::wstring(temp.begin(), temp.end());

		m_srvHeap.Get()->SetName(wideHeapName.c_str());
	}
	else
	{
		m_srvHeap = bindlessSRVHeap;
	}	

	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle;
	UINT descriptorSize = DXContext::Get().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	if (bindlessSRVHeap == nullptr)
	{
		srvHandle = m_srvHeap->GetCPUDescriptorHandleForHeapStart();
	}
	else
	{
		srvHandle = m_srvHeap->GetCPUDescriptorHandleForHeapStart();
		srvHandle.Offset(bindlessSRVIndex, descriptorSize);
	}

	for (unsigned int i = 0; i < m_count; i++)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = m_textureDatas[i].giPixelFormat;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.PlaneSlice = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

		DXContext::Get().GetDevice()->CreateShaderResourceView(m_textures[i], &srvDesc, srvHandle);
		srvHandle.Offset(1, descriptorSize);		
	}
	// =========================== //
}

UINT64 Texture::CopyToUploadBuffer(ID3D12Resource* uploadBuffer, UINT64 uploadBufferOffset, ID3D12GraphicsCommandList* cmdList)
{
	// === Copy void* --> CPU Resource === //
	char* uploadBufferAdress;
	D3D12_RANGE uploadRange;
	uploadRange.Begin = uploadBufferOffset;
	uploadRange.End = uploadBufferOffset + GetTotalTextureSize();
	uploadBuffer->Map(0, &uploadRange, (void**)&uploadBufferAdress);

	for (uint32_t i = 0; i < m_count; i++)
	{
		memcpy(&uploadBufferAdress
			[uploadBufferOffset],
			GetTextureData(i),
			GetTextureSize(i));
	
		// Source
		D3D12_TEXTURE_COPY_LOCATION txtSrc;
		txtSrc.pResource = uploadBuffer;
		txtSrc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		txtSrc.PlacedFootprint.Offset = uploadBufferOffset; //(uploadBufferOffset + 511) & ~511 ;
		txtSrc.PlacedFootprint.Footprint.Width = m_textureDatas[i].width;
		txtSrc.PlacedFootprint.Footprint.Height = m_textureDatas[i].height;
		txtSrc.PlacedFootprint.Footprint.Depth = 1;
		txtSrc.PlacedFootprint.Footprint.RowPitch = m_textureStrides[i]; //(m_textureStrides[i] + 255) & ~255 ;
		txtSrc.PlacedFootprint.Footprint.Format = m_textureDatas[i].giPixelFormat;

		// Destination
		D3D12_TEXTURE_COPY_LOCATION txtDst;
		txtDst.pResource = m_textures[i];
		txtDst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		txtDst.SubresourceIndex = 0;

		// Resource barrier
		D3D12_RESOURCE_BARRIER transitionBarrier = {};
		transitionBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		transitionBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		transitionBarrier.Transition.pResource = m_textures[i];
		transitionBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		transitionBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_GENERIC_READ;
		transitionBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;

		//Texture size
		D3D12_BOX textureSizeAsBox;
		textureSizeAsBox.left = textureSizeAsBox.top = textureSizeAsBox.front = 0;
		textureSizeAsBox.right = m_textureDatas[i].width;
		textureSizeAsBox.bottom = m_textureDatas[i].height;
		textureSizeAsBox.back = 1;

		// === COPY === //
		cmdList->ResourceBarrier(1, &transitionBarrier);
		cmdList->CopyTextureRegion(&txtDst, 0, 0, 0, &txtSrc, &textureSizeAsBox);

		transitionBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		transitionBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;

		cmdList->ResourceBarrier(1, &transitionBarrier);

		uploadBufferOffset += GetTextureSize(i);
	}
	return uploadBufferOffset;
}

void Texture::AddCommands(ID3D12GraphicsCommandList*& cmdList, UINT rootParameterIndex)
{
	cmdList->SetDescriptorHeaps(1, &m_srvHeap);
	cmdList->SetGraphicsRootDescriptorTable(rootParameterIndex, m_srvHeap->GetGPUDescriptorHandleForHeapStart());
}

