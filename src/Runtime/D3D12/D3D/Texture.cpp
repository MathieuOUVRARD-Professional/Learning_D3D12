#include "D3D/Texture.h"

Texture::Texture(std::string path, LPCWSTR name)
{
	// === Texture Data === //
	ImageLoader::LoadImageFromDisk(path, m_textureData);
	m_textureStride = m_textureData.width * ((m_textureData.bitPerPixel + 7) / 8);
	m_textureSize = m_textureData.height * m_textureStride;
	m_name = name;
}

void Texture::Init(D3D12_HEAP_PROPERTIES* defaultHeapProperties, ID3D12Resource* uploadBuffer, ID3D12GraphicsCommandList* cmdList)
{
	D3D12_RESOURCE_DESC rdt{};
	rdt.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	rdt.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	rdt.Width = m_textureData.width;
	rdt.Height = m_textureData.height;
	rdt.DepthOrArraySize = 1;
	rdt.MipLevels = 1;
	rdt.Format = m_textureData.giPixelFormat;
	rdt.SampleDesc.Count = 1;
	rdt.SampleDesc.Quality = 0;
	rdt.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	rdt.Flags = D3D12_RESOURCE_FLAG_NONE;

	DXContext::Get().GetDevice()->CreateCommittedResource(defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &rdt, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_texture));
	m_texture->SetName(m_name);

	//Texture
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDescText{};
	descriptorHeapDescText.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descriptorHeapDescText.NumDescriptors = 8;
	descriptorHeapDescText.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descriptorHeapDescText.NodeMask = 0;
	
	DXContext::Get().GetDevice()->CreateDescriptorHeap(&descriptorHeapDescText, IID_PPV_ARGS(&m_srvHeap));
	LPCWSTR descriptorHeapName = ((std::wstring)(m_name) + L" DescriptorHeap").c_str();
	m_srvHeap.Get()->SetName(descriptorHeapName);

	// === SRV === //
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = m_textureData.giPixelFormat;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.PlaneSlice = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	DXContext::Get().GetDevice()->CreateShaderResourceView(m_texture, &srvDesc, m_srvHeap->GetCPUDescriptorHandleForHeapStart());

	// Source
	D3D12_TEXTURE_COPY_LOCATION txtSrc;
	txtSrc.pResource = uploadBuffer;
	txtSrc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	txtSrc.PlacedFootprint.Offset = 0;
	txtSrc.PlacedFootprint.Footprint.Width = m_textureData.width;
	txtSrc.PlacedFootprint.Footprint.Height = m_textureData.height;
	txtSrc.PlacedFootprint.Footprint.Depth = 1;
	txtSrc.PlacedFootprint.Footprint.RowPitch = m_textureStride;
	txtSrc.PlacedFootprint.Footprint.Format = m_textureData.giPixelFormat;

	// Destination
	D3D12_TEXTURE_COPY_LOCATION txtDst;
	txtDst.pResource = m_texture;
	txtDst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	txtDst.SubresourceIndex = 0;

	// === COPY === //
	D3D12_RESOURCE_BARRIER transitionBarrier = {};
	transitionBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	transitionBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	transitionBarrier.Transition.pResource = m_texture;
	transitionBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	transitionBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_GENERIC_READ;
	transitionBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;

	D3D12_BOX textureSizeAsBox;
	textureSizeAsBox.left = textureSizeAsBox.top = textureSizeAsBox.front = 0;
	textureSizeAsBox.right = m_textureData.width;
	textureSizeAsBox.bottom = m_textureData.height;
	textureSizeAsBox.back = 1;

	cmdList->ResourceBarrier(1, &transitionBarrier);
	cmdList->CopyTextureRegion(&txtDst, 0, 0, 0, &txtSrc, &textureSizeAsBox);

	transitionBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	transitionBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;

	cmdList->ResourceBarrier(1, &transitionBarrier);
}

void Texture::AddCommands(ID3D12GraphicsCommandList* cmdList, UINT rootParameterIndex)
{
	cmdList->SetDescriptorHeaps(1, &m_srvHeap);
	cmdList->SetGraphicsRootDescriptorTable(rootParameterIndex, m_srvHeap->GetGPUDescriptorHandleForHeapStart());
}

