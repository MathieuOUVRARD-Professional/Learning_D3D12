#include <iostream>

#include <Support/WinInclude.h>
#include <Support/ComPointer.h>
#include <Support/Window.h>
#include <Support/Shader.h>
#include <Support/ImageLoader.h>
#include <Support/Camera.h>
#include <Support/AssImpUsage.h>
#include <Support/ObjectList.h>

#include <Debug/DebugLayer.h>

#include <D3D/DXContext.h>
#include <D3D/PipelineState.h>
#include <D3D/Texture.h>
#include <D3D/ZBuffer.h>

#include <Util/LoggingProvider.h>
#include <Util/EzException.h>
#include <Util/HRException.h>

#include <glm/gtc/type_ptr.hpp>

#include <dxcapi.h>

#include <directx/d3dx12.h>

#include <ImGui/CustomImGui.h>

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
	D3EZ::LoggingProvider::Init();

	/*spdlog::info("Hello World !");
	spdlog::warn("Warning critical failure has been detected");
	D3EZ_CHECK_HR_D(S_OK,"Clean HResult");
	D3EZ_CHECK_HR_D(E_OUTOFMEMORY,"Unclean HResult");
	
	D3EZ_CHECK_D(false, "Code implemented bellow is unsafe");*/

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

		ObjectList mainObjList;

		C_AssImp::Import("Sponza/NewSponza_Main_glTF_003.gltf", mainObjList.GetList());

		// === Vertex data === //
		// PYRAMID DATA 
		Vertex vertices[] =
		{
			//  X	   Y	  Z	  ||  U	   V  ||  Nx	 Ny		Nz
			{ -0.5f,  0.0f,  0.5f , 0.0f, 1.0f,  0.0f, -1.0f,  0.0f },	// Bottom side
			{ -0.5f,  0.0f, -0.5f , 1.0f, 1.0f,  0.0f, -1.0f,  0.0f },	//
			{  0.5f,  0.0f, -0.5f , 0.0f, 1.0f,  0.0f, -1.0f,  0.0f },	//
			{  0.5f,  0.0f,  0.5f , 1.0f, 1.0f,  0.0f, -1.0f,  0.0f },	//

			{ -0.5f,  0.0f,  0.5f , 0.0f, 1.0f, -1.0f,  0.5f,  0.0f },	// Left side
			{ -0.5f,  0.0f, -0.5f , 1.0f, 1.0f, -1.0f,  0.5f,  0.0f },	//
			{  0.0f,  1.0f,  0.0f , 0.5f, 0.0f, -1.0f,  0.5f,  0.0f },	//

			{ -0.5f,  0.0f, -0.5f , 1.0f, 1.0f,  0.0f,  0.5f, -1.0f },	// Back side
			{  0.5f,  0.0f, -0.5f , 0.0f, 1.0f,  0.0f,  0.5f, -1.0f },	//
			{  0.0f,  1.0f,  0.0f , 0.5f, 0.0f,  0.0f,  0.5f, -1.0f },	//

			{  0.5f,  0.0f, -0.5f , 0.0f, 1.0f,  1.0f,  0.5f,  0.0f },	// Right side
			{  0.5f,  0.0f,  0.5f , 1.0f, 1.0f,  1.0f,  0.5f,  0.0f },	//
			{  0.0f,  1.0f,  0.0f , 0.5f, 0.0f,  1.0f,  0.5f,  0.0f },	//

			{  0.5f,  0.0f,  0.5f , 1.0f, 1.0f,  0.0f,  0.5f,  1.0f },	// Front side
			{ -0.5f,  0.0f,  0.5f , 0.0f, 1.0f,  0.0f,  0.5f,  1.0f },	//
			{  0.0f,  1.0f,  0.0f , 0.5f, 0.0f,  0.0f,  0.5f,  1.0f },	//
		};
		DWORD indexes[] = {
			0,1,2,		//Bottom faces
			0,2,3,		//

			4,5,6,		//Left face

			7,8,9,		//Back face

			10,11,12,	//Right face

			13,14,15	//Front face
		};
		D3D12_INPUT_ELEMENT_DESC vertexLayout[] =
		{
			{ "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{ "Texcoord", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(float) * 3, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{ "Normal", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(float) * 5, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};
		//============================//

		// CUBE DATA
		struct VertexWithoutUVs
		{
			float x, y, z;
		};
		VertexWithoutUVs cubeVertices[] =
		{
			//  X	   Y	  Z	  
			{ -0.1f, -0.1f,  0.1f},
			{ -0.1f, -0.1f, -0.1f},
			{  0.1f, -0.1f, -0.1f},
			{  0.1f, -0.1f,  0.1f},
			{ -0.1f,  0.1f,  0.1f},
			{ -0.1f,  0.1f, -0.1f},
			{  0.1f,  0.1f, -0.1f},
			{  0.1f,  0.1f,  0.1f},
		};
		DWORD cubeIndexes[] = {
			0,1,2,
			0,2,3,
			0,7,4,
			0,3,7,
			3,6,7,
			3,2,6,
			2,5,6,
			2,1,5,
			1,4,5,
			1,0,4,
			4,6,5,
			4,7,6
		};
		D3D12_INPUT_ELEMENT_DESC cubeVertexLayout[] =
		{
			{ "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
		};
		//============================//

		auto* cmdList = DXContext::Get().InitCommandList();
		std::string eyeTexturePaths[2] = { "Textures/auge_512_512_BGRA_32BPP.png", "Textures/auge_spec_512_512_BGRA_32BPP.png" };
		LPCWSTR eyeTextureNames[2] = { L"All_Seeing_Eye", L"All_Seeing_Eye_SPECULAR" };
		//=== Textures ===//
		Texture eyeTextures = Texture(2, eyeTexturePaths, eyeTextureNames);

		// === Upload, vertex & indexes buffers === //
		D3D12_RESOURCE_DESC rdu{};
		rdu.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		rdu.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		rdu.Width = eyeTextures.GetTotalTextureSize() + mainObjList.TotalSize() + 2048;
		rdu.Height = 1;
		rdu.DepthOrArraySize = 1;
		rdu.MipLevels = 1;
		rdu.Format = DXGI_FORMAT_UNKNOWN;
		rdu.SampleDesc.Count = 1;
		rdu.SampleDesc.Quality = 0;
		rdu.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		rdu.Flags = D3D12_RESOURCE_FLAG_NONE;

		D3D12_RESOURCE_DESC rdv{};
		rdv.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		rdv.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		rdv.Width = mainObjList.TotalVerticesSize() + 1024;
		rdv.Height = 1;
		rdv.DepthOrArraySize = 1;
		rdv.MipLevels = 1;
		rdv.Format = DXGI_FORMAT_UNKNOWN;
		rdv.SampleDesc.Count = 1;
		rdv.SampleDesc.Quality = 0;
		rdv.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		rdv.Flags = D3D12_RESOURCE_FLAG_NONE;

		D3D12_RESOURCE_DESC rdi{};
		rdi.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		rdi.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		rdi.Width = mainObjList.TotalIndicesSize() + 1024;
		rdi.Height = 1;
		rdi.DepthOrArraySize = 1;
		rdi.MipLevels = 1;
		rdi.Format = DXGI_FORMAT_UNKNOWN;
		rdi.SampleDesc.Count = 1;
		rdi.SampleDesc.Quality = 0;
		rdi.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		rdi.Flags = D3D12_RESOURCE_FLAG_NONE;

		// Creating GPU memory pointer
		ComPointer<ID3D12Resource> uploadBuffer, vertexBuffer, indexBuffer;

		DXContext::Get().GetDevice()->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &rdu, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer));
		DXContext::Get().GetDevice()->CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &rdv, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&vertexBuffer));
		DXContext::Get().GetDevice()->CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &rdi, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&indexBuffer));		

		uploadBuffer.Get()->SetName(L"Upload_Buffer");
		vertexBuffer.Get()->SetName(L"Vertex_Buffer");
		indexBuffer.Get()->SetName(L"Index_Buffer");

		eyeTextures.Init(&defaultHeapProperties, uploadBuffer, cmdList);
		ZBuffer zBuffer = ZBuffer(&defaultHeapProperties);

		// === Copy void* --> CPU Resource === //
		char* uploadBufferAdress;
		D3D12_RANGE uploadRange;
		uploadRange.Begin = 0;
		uploadRange.End = eyeTextures.GetTotalTextureSize() + 2048;
		uploadBuffer->Map(0, &uploadRange, (void**)&uploadBufferAdress);

		// Texture copy
		memcpy(&uploadBufferAdress[0], eyeTextures.GetTextureData(0), eyeTextures.GetTextureSize(0));
		memcpy(&uploadBufferAdress[eyeTextures.GetTextureSize(0)], eyeTextures.GetTextureData(1), eyeTextures.GetTextureSize(1));
		//Pyramid buffers copy
		memcpy(&uploadBufferAdress[eyeTextures.GetTotalTextureSize()], vertices, sizeof(vertices));
		memcpy(&uploadBufferAdress[eyeTextures.GetTotalTextureSize() + 1024], indexes, sizeof(indexes));
		//Cube buffers copy: vertices are placed right after pyramid's ones, same for indexes
		memcpy(&uploadBufferAdress[eyeTextures.GetTotalTextureSize() + sizeof(vertices)], cubeVertices, sizeof(cubeVertices));
		memcpy(&uploadBufferAdress[eyeTextures.GetTotalTextureSize() + 1024 + sizeof(indexes)], cubeIndexes, sizeof(cubeIndexes));

		uploadBuffer->Unmap(0, &uploadRange);

		// Async Copy CPU Resource --> GPU Resource
		cmdList->CopyBufferRegion(vertexBuffer, 0, uploadBuffer, eyeTextures.GetTotalTextureSize(), 1024);
		cmdList->CopyBufferRegion(indexBuffer, 0, uploadBuffer, eyeTextures.GetTotalTextureSize() + 1024, 1024);

		DXContext::Get().ExecuteCommandList();

		// === Shaders === //
		Shader rootSignatureShader("RootSignature.cso");
		Shader vertexShader("VertexShader.cso");
		Shader pixelShader("PixelShader.cso");

		Shader lightRootSignatureShader("LightRootSignature.cso");
		Shader lightVertexShader("LightVertexShader.cso");
		Shader lightPixelShader("LightPixelShader.cso");

		// === Create Root signature === //
		ComPointer<ID3D12RootSignature> rootSignature, lightRootSignature;
		DXContext::Get().GetDevice()->CreateRootSignature(0, rootSignatureShader.GetBuffer(), rootSignatureShader.GetSize(), IID_PPV_ARGS(&rootSignature));
		DXContext::Get().GetDevice()->CreateRootSignature(0, lightRootSignatureShader.GetBuffer(), lightRootSignatureShader.GetSize(), IID_PPV_ARGS(&lightRootSignature));
		rootSignature.Get()->SetName(L"Main_RootSignature");
		lightRootSignature.Get()->SetName(L"Light_RootSignature");

		// === Pipeline states === //
		DXPipelineState pso, lightPso;
		pso.Init(L"Main_PSO", rootSignature, vertexLayout, _countof(vertexLayout), vertexShader, pixelShader);
		lightPso.Init(L"Light_PSO", lightRootSignature, cubeVertexLayout, _countof(cubeVertexLayout), lightVertexShader, lightPixelShader);

		// === Vertex buffer view === //
		// Pyramid
		D3D12_VERTEX_BUFFER_VIEW vbv{};
		vbv.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
		vbv.SizeInBytes = sizeof(Vertex) * _countof(vertices);
		vbv.StrideInBytes = sizeof(Vertex);

		// Cube
		D3D12_VERTEX_BUFFER_VIEW cubeVbv{};
		cubeVbv.BufferLocation = vertexBuffer->GetGPUVirtualAddress() + (sizeof(Vertex) * _countof(vertices));
		cubeVbv.SizeInBytes = sizeof(VertexWithoutUVs) * _countof(cubeVertices);
		cubeVbv.StrideInBytes = sizeof(VertexWithoutUVs);

		// === Index buffer view === //
		//Pyramid
		D3D12_INDEX_BUFFER_VIEW ibv{};
		ibv.BufferLocation = indexBuffer->GetGPUVirtualAddress();
		ibv.SizeInBytes = sizeof(DWORD) * _countof(indexes);
		ibv.Format = DXGI_FORMAT_R32_UINT;
		//Cube
		D3D12_INDEX_BUFFER_VIEW cubeIbv{};
		cubeIbv.BufferLocation = indexBuffer->GetGPUVirtualAddress() + (sizeof(DWORD) * _countof(indexes));
		cubeIbv.SizeInBytes = sizeof(DWORD) * _countof(cubeIndexes);
		cubeIbv.Format = DXGI_FORMAT_R32_UINT;

		Camera camera(DXWindow::Get().GetWidth(), DXWindow::Get().GetHeigth(), glm::vec3(0.0f, 0.0f, 2.0f));

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
		g_pd3dSrvDescHeap.Get()->SetName(L"ImGui_DescriptorHeap");

		init_info.SrvDescriptorHeap = g_pd3dSrvDescHeap;
		init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle) { return g_pd3dSrvDescHeapAlloc.Alloc(out_cpu_handle, out_gpu_handle); };
		init_info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle) { return g_pd3dSrvDescHeapAlloc.Free(cpu_handle, gpu_handle); };
		ImGui_ImplDX12_Init(&init_info);
#endif // IMGUI

		float angle = 0.0f;

		// === MAIN LOOP=== //
		DXWindow::Get().SetFullscreen(true);
		while (!DXWindow::Get().ShouldClose())
		{
#ifdef IMGUI
			// Start the Dear ImGui frame
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			ImGui::ShowDemoWindow(); // Show demo window! :)
#endif // IMGUIS
			
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

			DXWindow::Get().BeginFrame(cmdList, zBuffer.GetDescriptorHeap());			

			// === PSO === //
			cmdList->SetPipelineState(pso.Get());
			cmdList->SetGraphicsRootSignature(rootSignature);

			// === IA == /
			cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			cmdList->IASetVertexBuffers(0, 1, &vbv);
			cmdList->IASetIndexBuffer(&ibv);			

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
			angle += 0.005f;

			glm::vec3 pyramidPosition = glm::vec3(0.0f, 0.0f, 0.0f);
			glm::mat4 pyramidModel = glm::translate(glm::mat4(1.0f), pyramidPosition);
			pyramidModel = glm::rotate(pyramidModel, angle, glm::vec3(0.0f, 1.0f, 0.0f));
			static float color[] = { 0.0f, 1.0f, 0.0f , 1.0f };
			ColorPuke(color);

			//float lightColor[] = { 1.0f, 1.0f, 1.0f };
			std::string colorPickerName = "Light color";
			std::vector<float> lightColor = ImGuiColorPicker(&colorPickerName, true);
			ImGuiPerfOverlay(true);

			glm::vec3 lightPosition = glm::vec3(1.0f, 1.5f, .5f);
			glm::mat4 lightModel = glm::translate(glm::mat4(1.0f), lightPosition);

			struct Light
			{
				glm::vec4 lightcolor = glm::vec4(1.0f, 1.0f, 1.0, 1.0f);
				glm::vec4 lightPosition;
			};
			Light cubeLight;
			//cubeLight.lightcolor = glm::vec4(color[0], color[1], color[2], 1.0f);
			cubeLight.lightcolor = glm::vec4(lightColor[0], lightColor[1], lightColor[2], lightColor[3]);
			cubeLight.lightPosition = glm::vec4(lightPosition, 1.0f);

			camera.UpdateWindowSize(DXWindow::Get().GetWidth(), DXWindow::Get().GetHeigth());
			camera.Inputs();
			camera.Matrix(45.0f, 0.01f, 100.0f);

			// === ROOT === //
			camera.UpdateMatrix(cmdList, 0, pyramidModel);
			cmdList->SetGraphicsRoot32BitConstants(1, 8, &cubeLight, 0);
			cmdList->SetGraphicsRoot32BitConstants(2, 4, &color, 0);
			cmdList->SetGraphicsRoot32BitConstants(3, 4, &camera.m_position, 0);
			eyeTextures.AddCommands(cmdList, 4);
			// === Draw === //
			// Object 1
			cmdList->DrawIndexedInstanced(_countof(indexes), 1, 0, 0, 0);

			// Object 2
			cmdList->SetPipelineState(lightPso.Get());
			cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			cmdList->IASetVertexBuffers(0, 1, &cubeVbv);
			cmdList->IASetIndexBuffer(&cubeIbv);
			cmdList->SetGraphicsRootSignature(lightRootSignature);

			camera.UpdateMatrix(cmdList, 0, lightModel);
			cmdList->SetGraphicsRoot32BitConstants(1, 4, &cubeLight.lightcolor, 0);

			cmdList->DrawIndexedInstanced(_countof(cubeIndexes), 1, 0, 0, 0);

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