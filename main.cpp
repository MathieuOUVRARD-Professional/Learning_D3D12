#include <iostream>

#include <Support/WinInclude.h>
#include <Support/ComPointer.h>
#include <Support/Window.h>
#include <Support/Shader.h>
#include <Support/ImageLoader.h>

#include <Debug/DebugLayer.h>

#include <D3D/DXContext.h>
#include <D3D/PipelineState.h>

#define IMGUI

void ColorPuke(float* color)
{
	static int pukeState = 0;

	switch (pukeState)
	{
	case 0://RED+
		color[pukeState] += 0.01f;
		if (color[pukeState] >= 1.0f)
		{
			pukeState++;
		}
		break;
	case 1://BLUE-
		color[pukeState + 1] -= 0.01f;
		if (color[pukeState + 1] <= 0.0f)
		{
			pukeState++;
		}
		break;
	case 2://GREEN+
		color[pukeState-1] += 0.01f;
		if (color[pukeState-1] >= 1.0f)
		{
			pukeState++;
		}
		break;
	case 3://RED-
		color[pukeState - 3] -= 0.01f;
		if (color[pukeState - 3] <= 0.0)
		{
			pukeState++;
		}
		break;
	case 4://BLUE+
		color[pukeState - 2] += 0.01f;
		if (color[pukeState - 2] >= 1.0f)
		{
			pukeState++;
		}
		break;
	case 5://GREEN-
		color[pukeState - 4] -= 0.01f;
		if (color[pukeState - 4] <= 0.0f)
		{
			pukeState++;
		}
		break;
	case 6:
		color[pukeState - 4] += 0.01f;
		if (color[pukeState - 4] >= 1.0f)
		{
			pukeState++;
		}
		break;
	}
	if(pukeState > 6)
	{
		pukeState = 0;
		color[0] = 0;
		color[1] = 0;
		color[2] = 1;
	}
}

#ifdef IMGUI
// Simple free list based allocator
struct ExampleDescriptorHeapAllocator
{
	ID3D12DescriptorHeap* Heap = nullptr;
	D3D12_DESCRIPTOR_HEAP_TYPE  HeapType = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
	D3D12_CPU_DESCRIPTOR_HANDLE HeapStartCpu;
	D3D12_GPU_DESCRIPTOR_HANDLE HeapStartGpu;
	UINT                        HeapHandleIncrement;
	ImVector<int>               FreeIndices;

	void Create(ID3D12Device* device, ID3D12DescriptorHeap* heap)
	{
		IM_ASSERT(Heap == nullptr && FreeIndices.empty());
		Heap = heap;
		D3D12_DESCRIPTOR_HEAP_DESC desc = heap->GetDesc();
		HeapType = desc.Type;
		HeapStartCpu = Heap->GetCPUDescriptorHandleForHeapStart();
		HeapStartGpu = Heap->GetGPUDescriptorHandleForHeapStart();
		HeapHandleIncrement = device->GetDescriptorHandleIncrementSize(HeapType);
		FreeIndices.reserve((int)desc.NumDescriptors);
		for (int n = desc.NumDescriptors; n > 0; n--)
			FreeIndices.push_back(n);
	}
	void Destroy()
	{
		Heap = nullptr;
		FreeIndices.clear();
	}
	void Alloc(D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_desc_handle)
	{
		IM_ASSERT(FreeIndices.Size > 0);
		int idx = FreeIndices.back();
		FreeIndices.pop_back();
		out_cpu_desc_handle->ptr = HeapStartCpu.ptr + (idx * HeapHandleIncrement);
		out_gpu_desc_handle->ptr = HeapStartGpu.ptr + (idx * HeapHandleIncrement);
	}
	void Free(D3D12_CPU_DESCRIPTOR_HANDLE out_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE out_gpu_desc_handle)
	{
		int cpu_idx = (int)((out_cpu_desc_handle.ptr - HeapStartCpu.ptr) / HeapHandleIncrement);
		int gpu_idx = (int)((out_gpu_desc_handle.ptr - HeapStartGpu.ptr) / HeapHandleIncrement);
		IM_ASSERT(cpu_idx == gpu_idx);
		FreeIndices.push_back(cpu_idx);
	}
};
static ExampleDescriptorHeapAllocator g_pd3dSrvDescHeapAlloc;
#endif // IMGUI

int main()
{
	DXDebugLayer::Get().Init();

	if (DXContext::Get().Init() && DXWindow::Get().Init())
	{
		// Upload heap CPU --> GPU
		D3D12_HEAP_PROPERTIES uploadHeapProperties{};
		uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
		uploadHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		uploadHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		uploadHeapProperties.CreationNodeMask = 0;
		uploadHeapProperties.VisibleNodeMask = 0;

		// Default heap, GPU only
		D3D12_HEAP_PROPERTIES defaultHeapProperties{};
		defaultHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		defaultHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		defaultHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		defaultHeapProperties.CreationNodeMask = 0;
		defaultHeapProperties.VisibleNodeMask = 0;

		// === Vertex data === //
		struct Vertex
		{
			float x, y;
			float u, v;
		};
		Vertex vertices[] =
		{
			//T1
			{ -1.0f, -1.0f, 0.0f , 1.0f },
			{  0.0f,  1.0f, 0.5f , 0.0f },
			{  1.0f, -1.0f, 1.0f , 1.0f }
		};
		D3D12_INPUT_ELEMENT_DESC vertexLayout[] =
		{
			{ "Position", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{ "Texcoord", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(float) * 2, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
		};

		// === Texture Data === //
		ImageLoader::ImageData textureData;
		ImageLoader::LoadImageFromDisk("./auge_512_512_BGRA_32BPP.png", textureData);
		uint32_t textureStride = textureData.width * ((textureData.bitPerPixel + 7) / 8);
		uint32_t textureSize = textureData.height * textureStride;

		// === Upload & vertex buffer === //
		D3D12_RESOURCE_DESC rdv{};
		rdv.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		rdv.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		rdv.Width = 1024;
		rdv.Height = 1;
		rdv.DepthOrArraySize = 1;
		rdv.MipLevels = 1;
		rdv.Format = DXGI_FORMAT_UNKNOWN;
		rdv.SampleDesc.Count = 1;
		rdv.SampleDesc.Quality = 0;
		rdv.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		rdv.Flags = D3D12_RESOURCE_FLAG_NONE;

		D3D12_RESOURCE_DESC rdu{};
		rdu.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		rdu.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		rdu.Width = textureSize + 1024;
		rdu.Height = 1;
		rdu.DepthOrArraySize = 1;
		rdu.MipLevels = 1;
		rdu.Format = DXGI_FORMAT_UNKNOWN;
		rdu.SampleDesc.Count = 1;
		rdu.SampleDesc.Quality = 0;
		rdu.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		rdu.Flags = D3D12_RESOURCE_FLAG_NONE;

		// Creating GPU memory pointer
		ComPointer<ID3D12Resource> uploadBuffer, vertexBuffer;

		DXContext::Get().GetDevice()->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &rdu, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer));
		DXContext::Get().GetDevice()->CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &rdv, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&vertexBuffer));

		D3D12_RESOURCE_DESC rdt{};
		rdt.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		rdt.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		rdt.Width = textureData.width;
		rdt.Height = textureData.height;
		rdt.DepthOrArraySize = 1;
		rdt.MipLevels = 1;
		rdt.Format = textureData.giPixelFormat;
		rdt.SampleDesc.Count = 1;
		rdt.SampleDesc.Quality = 0;
		rdt.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		rdt.Flags = D3D12_RESOURCE_FLAG_NONE;

		ComPointer<ID3D12Resource> texture;
		DXContext::Get().GetDevice()->CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &rdt, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&texture));
		texture->SetName(L"Texture");

		// === Descriptor Heap for Texture(s) === //
		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDescText{};
		descriptorHeapDescText.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		descriptorHeapDescText.NumDescriptors = 8;
		descriptorHeapDescText.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		descriptorHeapDescText.NodeMask = 0;

		ComPointer<ID3D12DescriptorHeap> srvHeap;
		DXContext::Get().GetDevice()->CreateDescriptorHeap(&descriptorHeapDescText, IID_PPV_ARGS(&srvHeap));

		// === SRV === //
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = textureData.giPixelFormat;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.PlaneSlice = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

		DXContext::Get().GetDevice()->CreateShaderResourceView(texture, &srvDesc, srvHeap->GetCPUDescriptorHandleForHeapStart());

		// Copy void* --> CPU Resource
		char* uploadBufferAdress;
		D3D12_RANGE uploadRange;
		uploadRange.Begin = 0;
		uploadRange.End = 1024 + textureSize;
		uploadBuffer->Map(0, &uploadRange, (void**)&uploadBufferAdress);
		memcpy(&uploadBufferAdress[0], textureData.content.data(), textureSize);
		memcpy(&uploadBufferAdress[textureSize], vertices, sizeof(vertices));
		uploadBuffer->Unmap(0, &uploadRange);

		// Async Copy CPU Resource --> GPU Resource
		auto* cmdList = DXContext::Get().InitCommandList();
		cmdList->CopyBufferRegion(vertexBuffer, 0, uploadBuffer, textureSize, 1024);
		D3D12_BOX textureSizeAsBox;
		textureSizeAsBox.left = textureSizeAsBox.top = textureSizeAsBox.front = 0;
		textureSizeAsBox.right = textureData.width;
		textureSizeAsBox.bottom = textureData.height;
		textureSizeAsBox.back = 1;

		// Source
		D3D12_TEXTURE_COPY_LOCATION txtSrc;
		txtSrc.pResource = uploadBuffer;
		txtSrc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		txtSrc.PlacedFootprint.Offset = 0;
		txtSrc.PlacedFootprint.Footprint.Width = textureData.width;
		txtSrc.PlacedFootprint.Footprint.Height = textureData.height;
		txtSrc.PlacedFootprint.Footprint.Depth = 1;
		txtSrc.PlacedFootprint.Footprint.RowPitch = textureStride;
		txtSrc.PlacedFootprint.Footprint.Format = textureData.giPixelFormat;
		// Destination
		D3D12_TEXTURE_COPY_LOCATION txtDst;
		txtDst.pResource = texture;
		txtDst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		txtDst.SubresourceIndex = 0;

		// === COPY === //
		D3D12_RESOURCE_BARRIER transitionBarrier = {};
		transitionBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		transitionBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		transitionBarrier.Transition.pResource = texture;
		transitionBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		transitionBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_GENERIC_READ;
		transitionBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;

		cmdList->ResourceBarrier(1, &transitionBarrier);
		cmdList->CopyTextureRegion(&txtDst, 0, 0, 0, &txtSrc, &textureSizeAsBox);

		transitionBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		transitionBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;

		cmdList->ResourceBarrier(1, &transitionBarrier);

		DXContext::Get().ExecuteCommandList();

		// === Shaders === //
		Shader rootSignatureShader("RootSignature.cso");
		Shader vertexShader("VertexShader.cso");
		Shader pixelShader("PixelShader.cso");

		// === Create Root signature === //
		ComPointer<ID3D12RootSignature> rootSignature;
		DXContext::Get().GetDevice()->CreateRootSignature(0, rootSignatureShader.GetBuffer(), rootSignatureShader.GetSize(), IID_PPV_ARGS(&rootSignature));

		// === Pipeline state === //
		DXPipelineState pso;
		pso.Init(rootSignature, vertexLayout, _countof(vertexLayout), vertexShader, pixelShader);

		// === Vertex buffer view == /
		D3D12_VERTEX_BUFFER_VIEW vbv{};
		vbv.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
		vbv.SizeInBytes = sizeof(Vertex) * _countof(vertices);
		vbv.StrideInBytes = sizeof(Vertex);

		// === ImGui SetUp === //
#ifdef IMGUI
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();

		ImGui_ImplWin32_Init(DXWindow::Get().GetWindow());

		// Setup Platform/Renderer backends
		ImGui_ImplDX12_InitInfo init_info = {};
		init_info.Device = DXContext::Get().GetDevice();
		init_info.CommandQueue = DXContext::Get().GetCommandQueue();
		init_info.NumFramesInFlight = 1;
		init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM; // Or your render target format.
		// Allocating SRV descriptors (for textures) is up to the application, so we provide callbacks.
		// The example_win32_directx12/main.cpp application include a simple free-list based allocator.
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NumDescriptors = 64;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		ComPointer<ID3D12DescriptorHeap> g_pd3dSrvDescHeap = nullptr;

		DXContext::Get().GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dSrvDescHeap));
		g_pd3dSrvDescHeapAlloc.Create(DXContext::Get().GetDevice(), g_pd3dSrvDescHeap);


		init_info.SrvDescriptorHeap = g_pd3dSrvDescHeap;
		init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle) { return g_pd3dSrvDescHeapAlloc.Alloc(out_cpu_handle, out_gpu_handle); };
		init_info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle) { return g_pd3dSrvDescHeapAlloc.Free(cpu_handle, gpu_handle); };
		ImGui_ImplDX12_Init(&init_info);
#endif // IMGUI

		// === Main loop === //
		DXWindow::Get().SetFullscreen(true);
		while (!DXWindow::Get().ShouldClose())
		{
#ifdef IMGUI
			// Start the Dear ImGui frame
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			ImGui::ShowDemoWindow(); // Show demo window! :)
#endif // IMGUI
			
			// Process pending window messages
			DXWindow::Get().Update();

			// Handle resizing
			if (DXWindow::Get().ShouldResize())
			{
				// Flushing (command queue). (As much as buffer)
				DXContext::Get().Flush(DXWindow::Get().GetFrameCount());
				DXWindow::Get().Resize();
			}

			// Begin drawing
			cmdList = DXContext::Get().InitCommandList();

			DXWindow::Get().BeginFrame(cmdList);			

			// === PSO === //
			cmdList->SetPipelineState(pso.Get());
			cmdList->SetGraphicsRootSignature(rootSignature);
			cmdList->SetDescriptorHeaps(1, &srvHeap);

			// === Input Assembler == /
			cmdList->IASetVertexBuffers(0, 1, &vbv);
			cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			// === RS == //
			// View Port
			D3D12_VIEWPORT vp;
			vp.TopLeftX = 0;
			vp.TopLeftY = 0;
			vp.Width = (FLOAT)DXWindow::Get().GetWidth();
			vp.Height = (FLOAT)DXWindow::Get().GetHeigth();
			vp.MinDepth = 1.0f;
			vp.MaxDepth = 0.0f;
			cmdList->RSSetViewports(1, &vp);
			// Screeen Rect
			RECT scRect;
			scRect.left = scRect.top = 0;
			scRect.right = DXWindow::Get().GetWidth();
			scRect.bottom = DXWindow::Get().GetHeigth();
			cmdList->RSSetScissorRects(1, &scRect);

			// === UPDATE === //
			static float color[] = { 0.0f, 1.0f, 0.0f };
			ColorPuke(color);


			static float angle = 0.0f;
			angle += 0.01f;
			struct Correction
			{
				float aspect_ratio;
				float zoom;
				float sinAngle;
				float cosAngle;
			};
			Correction correction
			{
				.aspect_ratio = ((float)DXWindow::Get().GetHeigth() / (float)DXWindow::Get().GetWidth()),
				.zoom = 0.8f,
				.sinAngle = sinf(angle),
				.cosAngle = cosf(angle),
			};
			if (DXWindow::Get().GetHeigth() > DXWindow::Get().GetWidth())
			{
				// Divide zoom by aspect ratio to avoid cropping due to aspect ratio > 1
				correction.zoom /= correction.aspect_ratio;
			}

			// === ROOT === //
			cmdList->SetGraphicsRoot32BitConstants(0, 3, &color, 0);
			cmdList->SetGraphicsRoot32BitConstants(1, 4, &correction, 0);
			cmdList->SetGraphicsRootDescriptorTable(2, srvHeap->GetGPUDescriptorHandleForHeapStart());

			// === Draw === //
			cmdList->DrawInstanced(_countof(vertices), 1, 0, 0);

#ifdef IMGUI
			ImGui::Render();
			cmdList->SetDescriptorHeaps(1, &g_pd3dSrvDescHeap);
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdList);
#endif // IMGUI

			DXWindow::Get().EndFrame(cmdList);

			// Finish drawing and present
			DXContext::Get().ExecuteCommandList();
			DXWindow::Get().Present();
		}

		// Flushing (command queue). (As much as buffer)
		DXContext::Get().Flush(DXWindow::Get().GetFrameCount());

		// Close
		vertexBuffer.Release();
		uploadBuffer.Release();

#ifdef IMGUI
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
#endif // IMGUI

		DXWindow::Get().Shutdown();
		DXContext::Get().Shutdown();
	}

	DXDebugLayer::Get().Shutdown();
}