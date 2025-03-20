#include <Support/ObjectList.h>

UINT64 ObjectList::TotalTexturesSize()
{
	UINT64 size = 0;
	for (Material& material : m_materials)
	{
		size += material.TextureSize();
	}
	return size;
}

UINT64 ObjectList::TotalVerticesSize()
{
	UINT64 size = 0;
	for (SceneObject& object : m_list)
	{
		size += object.m_mesh.VerticesSize();
	}
	return size;
}

UINT64 ObjectList::TotalIndicesSize()
{
	UINT64 size = 0;
	for (SceneObject& object : m_list)
	{
		size += object.m_mesh.IndicesSize();
	}
	return size;
}

UINT64 ObjectList::TotalMatDataSize()
{
	return sizeof(MaterialData) * MaterialsCount();
}

UINT64 ObjectList::TotalSize()
{
	UINT64 size = 0;
	for(SceneObject& object : m_list)
	{
		size += object.m_mesh.Size();
	}
	for (Material& material : m_materials)
	{
		size += material.TextureSize();
	}
	return size;
}

uint32_t ObjectList::TextureCount()
{
	uint32_t count = 0;
	for (Material& material : m_materials)
	{
		count += material.GetTextures().m_count;
	}
	return count;
}

void ObjectList::BindDescriptorHeaps(ID3D12GraphicsCommandList* cmdList, uint32_t rootParameterIndex)
{
	ID3D12DescriptorHeap* myHeap = m_bindlessHeapAllocator->GetHeap();
	ID3D12DescriptorHeap* const* heapPtr = &myHeap;

	cmdList->SetDescriptorHeaps(1, heapPtr);
	cmdList->SetGraphicsRootDescriptorTable(rootParameterIndex, m_bindlessHeapAllocator->GetGPUHandle(0));
}

void ObjectList::ShadowPassDraw(ID3D12GraphicsCommandList* cmdList, Light& light)
{
	for (SceneObject& object : m_list)
	{
		object.m_mesh.ShadowPassDraw(cmdList, light.m_viewProjMatrix, object.m_transform.m_matrix);
	}
}

void ObjectList::Draw(ID3D12GraphicsCommandList* cmdList, Camera& camera)
{
	for (SceneObject& object : m_list)
	{
		object.m_mesh.Draw(cmdList, camera.m_viewProjMatrix, object.m_transform.m_matrix, camera.m_position);
	}
}

void ObjectList::CopyTextures(ID3D12GraphicsCommandList* cmdList, D3D12_HEAP_PROPERTIES* defaultHeapProperties, ID3D12Resource* uploadBuffer, UINT64 destBufferOffset)
{
	for (Material& material : m_materials)
	{
		material.GetTextures().Init(defaultHeapProperties, m_bindlessHeapAllocator);

		if (destBufferOffset + material.TextureSize() > uploadBuffer->GetDesc().Width)
		{
			//Not enough space for next material in the upload buffer
			DXContext::Get().ExecuteCommandList();	// Fence synchronization
			cmdList = DXContext::Get().InitCommandList();
			destBufferOffset = 0;
		}
		material.GetTextures().CopyToUploadBuffer(uploadBuffer, destBufferOffset, cmdList);
		destBufferOffset += material.TextureSize();
	}

	// Waiting for the buffer to get back before writing meshes
	DXContext::Get().ExecuteCommandList();
	cmdList = DXContext::Get().InitCommandList();
}

void ObjectList::CopyMeshes(ID3D12Resource* uploadBuffer, UINT64 destOffsetVertex, UINT64 destOffsetIndex)
{
	char* uploadBufferAdress;
	D3D12_RANGE uploadRange;
	uploadRange.Begin = 0;
	uploadRange.End = TotalVerticesSize() + TotalIndicesSize();
	uploadBuffer->Map(0, &uploadRange, (void**)&uploadBufferAdress);

	destOffsetIndex = destOffsetVertex + TotalVerticesSize() + destOffsetIndex;

	UINT64 objectVertexOffset = 0;
	UINT64 objectIndexOffset = 0;
	for (SceneObject& object : m_list)
	{
		if (object.m_mesh.m_nSubmeshes > 0)
		{
			UINT64 submeshVertexOffset = 0;
			UINT64 submeshIndexOffset = 0;
			for (unsigned int i = 0; i < object.m_mesh.m_nSubmeshes; i++)
			{
				memcpy(&uploadBufferAdress
					[destOffsetVertex + objectVertexOffset + submeshVertexOffset],
					object.m_mesh.GetSubmesh(i).GetVertices().data(),
					object.m_mesh.GetSubmesh(i).VerticesSize());
				object.m_mesh.GetSubmesh(i).m_vertexBufferOffset = destOffsetVertex + objectVertexOffset + submeshVertexOffset;

				memcpy(&uploadBufferAdress
					[destOffsetIndex + objectIndexOffset + submeshIndexOffset],
					object.m_mesh.GetSubmesh(i).GetIndices().data(),
					object.m_mesh.GetSubmesh(i).IndicesSize());
				object.m_mesh.GetSubmesh(i).m_indexBufferOffset = (destOffsetIndex - destOffsetVertex - TotalVerticesSize()) + objectIndexOffset + submeshIndexOffset;

				submeshVertexOffset += object.m_mesh.GetSubmesh(i).VerticesSize();
				submeshIndexOffset += object.m_mesh.GetSubmesh(i).IndicesSize();
			}
			objectVertexOffset += submeshVertexOffset;
			objectIndexOffset += submeshIndexOffset;
		}
		else
		{
			memcpy(
				&uploadBufferAdress[destOffsetVertex + objectVertexOffset],
				object.m_mesh.GetVertices().data(),
				object.m_mesh.VerticesSize());
			object.m_mesh.m_vertexBufferOffset = destOffsetVertex + objectVertexOffset;

			memcpy(&uploadBufferAdress
				[destOffsetIndex + objectIndexOffset],
				object.m_mesh.GetIndices().data(),
				object.m_mesh.IndicesSize());
			object.m_mesh.m_indexBufferOffset = (destOffsetIndex - destOffsetVertex - TotalVerticesSize()) + objectIndexOffset;

			objectVertexOffset += object.m_mesh.VerticesSize();
			objectIndexOffset += object.m_mesh.IndicesSize();
		}
	}
	uploadBuffer->Unmap(0, &uploadRange);
}


void ObjectList::CopyMaterialData()
{
	for (uint32_t i = 0; i < MaterialsCount(); i++)
	{
		UINT cbSize = (sizeof(MaterialData) + 255) & ~255;

		// Describe the constant buffer
		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_UPLOAD; // Upload heap (CPU -> GPU)

		D3D12_RESOURCE_DESC bufferDesc = {};
		bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufferDesc.Width = cbSize;
		bufferDesc.Height = 1;
		bufferDesc.DepthOrArraySize = 1;
		bufferDesc.MipLevels = 1;
		bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		bufferDesc.SampleDesc.Count = 1;
		bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		// Create the buffer resource
		DXContext::Get().GetDevice()->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_materials[i].m_dataBuffer)
		);

		std::string name = m_materials[i].m_name + "_CB";
		m_materials[i].m_dataBuffer.Get()->SetName(std::wstring(name.begin(), name.end()).c_str());
		MaterialData materialData  = m_materials[i].GetData();

		// Map the constant buffer
		void* mappedData;
		m_materials[i].m_dataBuffer->Map(0, nullptr, &mappedData);
		// Copy
		memcpy(mappedData, &materialData, sizeof(MaterialData));
		//Unmap
		m_materials[i].m_dataBuffer->Unmap(0, nullptr);

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = m_materials[i].m_dataBuffer->GetGPUVirtualAddress(); // GPU address of buffer
		cbvDesc.SizeInBytes = cbSize; // Must be 256-byte aligned

		// Get CPU handle of the descriptor heap
		CD3DX12_CPU_DESCRIPTOR_HANDLE cbvHandle;
		uint32_t descriptorSize = DXContext::Get().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		cbvHandle = m_cbvHeap->GetCPUDescriptorHandleForHeapStart();
		cbvHandle.Offset(i, descriptorSize);

		// Create the CBV in the descriptor heap
		DXContext::Get().GetDevice()->CreateConstantBufferView(&cbvDesc, cbvHandle);
	}
}

void ObjectList::CopyToUploadBuffer(ID3D12GraphicsCommandList* cmdList, D3D12_HEAP_PROPERTIES* defaultHeapProperties, ID3D12Resource* uploadBuffer, UINT64 destOffsetTexture, UINT64 destOffsetVertex, UINT64 destOffsetIndex)
{
	if (m_bindlessHeapAllocator == nullptr)
	{
		D3EZ::EzException("ObjectList::CopyToUploadBuffer", 231, "No SRV Heap allocator !");
	}

	// Bindless MaterialData Descriptor Heap
	D3D12_DESCRIPTOR_HEAP_DESC bindlessMaterialDataHeapDesc = {};
	bindlessMaterialDataHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	bindlessMaterialDataHeapDesc.NumDescriptors = MaterialsCount();  
	bindlessMaterialDataHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	DXContext::Get().GetDevice()->CreateDescriptorHeap(&bindlessMaterialDataHeapDesc, IID_PPV_ARGS(&m_cbvHeap));
	std::string cbvHeapName = m_name + "_CBV";
	m_cbvHeap.Get()->SetName(std::wstring(cbvHeapName.begin(), cbvHeapName.end()).c_str());

	// Textures copy
	CopyTextures(cmdList, defaultHeapProperties, uploadBuffer, destOffsetTexture);

	// Meshes copy 
	CopyMeshes(uploadBuffer, destOffsetVertex, destOffsetIndex);

	CopyMaterialData();
}

void ObjectList::CreateBufferViews(ID3D12Resource* vertexBuffer, ID3D12Resource* indexBuffer)
{
	for (SceneObject& object : m_list)
	{
		if (object.m_mesh.m_nSubmeshes > 0)
		{
			for (unsigned int i = 0; i < object.m_mesh.m_nSubmeshes; i++)
			{
				// === Vertex buffer view === //
				D3D12_VERTEX_BUFFER_VIEW vbv{};
				vbv.BufferLocation = vertexBuffer->GetGPUVirtualAddress() + object.m_mesh.GetSubmesh(i).m_vertexBufferOffset;
				vbv.SizeInBytes = object.m_mesh.GetSubmesh(i).VerticesSize();
				vbv.StrideInBytes = sizeof(Vertex);

				object.m_mesh.GetSubmesh(i).SetVBV(vbv);

				// === Index buffer view === //	
				D3D12_INDEX_BUFFER_VIEW ibv{};
				ibv.BufferLocation = indexBuffer->GetGPUVirtualAddress() + object.m_mesh.GetSubmesh(i).m_indexBufferOffset;
				ibv.SizeInBytes = object.m_mesh.GetSubmesh(i).IndicesSize();
				ibv.Format = DXGI_FORMAT_R32_UINT;

				object.m_mesh.GetSubmesh(i).SetIBV(ibv);

				// Cleaning
				object.m_mesh.GetSubmesh(i).ClearVectors();
			}
		}
		else
		{
			// === Vertex buffer view === //
			D3D12_VERTEX_BUFFER_VIEW vbv{};
			vbv.BufferLocation = vertexBuffer->GetGPUVirtualAddress() + object.m_mesh.m_vertexBufferOffset;
			vbv.SizeInBytes = object.m_mesh.VerticesSize();
			vbv.StrideInBytes = sizeof(Vertex);

			object.m_mesh.SetVBV(vbv);

			// === Index buffer view === //
			D3D12_INDEX_BUFFER_VIEW ibv{};
			ibv.BufferLocation = indexBuffer->GetGPUVirtualAddress() + object.m_mesh.m_indexBufferOffset;
			ibv.SizeInBytes = object.m_mesh.IndicesSize();
			ibv.Format = DXGI_FORMAT_R32_UINT;

			object.m_mesh.SetIBV(ibv);

			// Cleaning
			object.m_mesh.ClearVectors();
		}	
	}
}
