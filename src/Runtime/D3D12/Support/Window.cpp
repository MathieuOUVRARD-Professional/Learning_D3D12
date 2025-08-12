#include "Window.h"

bool DXWindow::Init()
{
	//Window class

	WNDCLASSEXW wcex{};
	wcex.cbSize = sizeof(wcex);
	wcex.style = CS_OWNDC;
	wcex.lpfnWndProc = &DXWindow::OnWindowMessage;
	// Extra memory for class and window
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	// Get current active module handler with nullptr 
	wcex.hInstance = GetModuleHandle(nullptr);
	wcex.hIcon = LoadIconW(nullptr, (LPCWSTR)IDI_APPLICATION);
	wcex.hCursor = LoadCursorW(nullptr, (LPCWSTR)IDC_ARROW);
	wcex.hbrBackground = nullptr;
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = L"D3D12ExWindowClass";	
	wcex.hIconSm = LoadIconW(nullptr, (LPCWSTR)IDI_APPLICATION);

	m_windowClass = RegisterClassExW(&wcex);
	if (m_windowClass == 0)
	{
		return false;
	}

	// Place window on current cursor's screen
	POINT cursorPos{ 0,0 };
	GetCursorPos(&cursorPos);
	HMONITOR monitor = MonitorFromPoint(cursorPos, MONITOR_DEFAULTTONEAREST);
	MONITORINFO monitorInfo{};
	monitorInfo.cbSize = sizeof(monitorInfo);
	GetMonitorInfoW(monitor, &monitorInfo);

	//Window
	m_window = CreateWindowExW(
		WS_EX_OVERLAPPEDWINDOW | WS_EX_APPWINDOW, 
		(LPCWSTR)m_windowClass, 
		L"D3D12_Learning", 
		WS_OVERLAPPEDWINDOW | WS_VISIBLE, 
		monitorInfo.rcWork.left + 100, 
		monitorInfo.rcWork.top + 100, 
		1920, 1080,
		nullptr,
		nullptr, 
		wcex.hInstance, 
		nullptr);

	if (m_window == nullptr)
	{
		return false;
	}

	// Describe Swap Chain
	DXGI_SWAP_CHAIN_DESC1 swapChainDescription = {};
	DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapChainFullscreenDescripton = {};

	swapChainDescription.Width				= 1920;
	swapChainDescription.Height				= 1080;
	swapChainDescription.Format				= DXGI_FORMAT_R8G8B8A8_UNORM; //Change here for HDR
	swapChainDescription.Stereo				= false;
	swapChainDescription.SampleDesc.Count	= 1;
	swapChainDescription.SampleDesc.Quality = 0;
	swapChainDescription.BufferUsage		= DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDescription.BufferCount		= (UINT)GetFrameCount();
	swapChainDescription.Scaling			= DXGI_SCALING_STRETCH;
	swapChainDescription.SwapEffect			= DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDescription.AlphaMode			= DXGI_ALPHA_MODE_IGNORE;
	swapChainDescription.Flags				= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

	
	//swapChainFullscreenDescripton.RefreshRate;
	//swapChainFullscreenDescripton.ScanlineOrdering;
	//swapChainFullscreenDescripton.Scaling;
	swapChainFullscreenDescripton.Windowed = true;

	// Create Swap Chain
	auto& factory = DXContext::Get().GetFactory();
	ComPointer<IDXGISwapChain1> swapChain1;
	factory->CreateSwapChainForHwnd(DXContext::Get().GetCommandQueue(), m_window, &swapChainDescription, &swapChainFullscreenDescripton, nullptr, &swapChain1);
	if (!swapChain1.QueryInterface(m_swapChain))
	{
		return false;
	}	

	//Create RTV Heap
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descHeapDesc.NumDescriptors = FrameCount;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	descHeapDesc.NodeMask = 0;

	if (FAILED(DXContext::Get().GetDevice()->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&m_rtvHeap))))
	{
		return false;
	}
	m_rtvHeap.Get()->SetName(L"Main_Swapchain");

	//Create handles to view
	auto firstHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	auto handleIncrement = DXContext::Get().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	for (size_t i = 0; i < FrameCount; ++i)
	{
		m_rtvHandles[i] = firstHandle;
		m_rtvHandles[i].ptr += handleIncrement * i;
	}	

	//Get Buffers
	if (!GetBuffers())
	{
		return false;
	}	
	return true;
}

void DXWindow::BindMainRenderTarget(ID3D12GraphicsCommandList*& cmdList)
{
	// Bind the main backbuffer
	if (m_ZBuffer != nullptr)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_ZBuffer->GetDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
		cmdList->OMSetRenderTargets(1, &m_rtvHandles[m_currentBufferIndex], false, &dsvHandle);
	}
	else
	{
		cmdList->OMSetRenderTargets(1, &m_rtvHandles[m_currentBufferIndex], false, nullptr);
	}

	// Restore main viewport & scissor rects
	cmdList->RSSetViewports(1, &m_mainViewport);
	cmdList->RSSetScissorRects(1, &m_mainScissorRect);	
}

void DXWindow::SetViewPort()
{
	//ViewPort
	m_mainViewport.TopLeftX = 0;
	m_mainViewport.TopLeftY = 0;
	m_mainViewport.Width = (FLOAT)DXWindow::Get().GetWidth();
	m_mainViewport.Height = (FLOAT)DXWindow::Get().GetHeigth();
	m_mainViewport.MinDepth = 0.0f;
	m_mainViewport.MaxDepth = 1.0f;

	// Screeen Rect	
	m_mainScissorRect.left = m_mainScissorRect.top = 0;
	m_mainScissorRect.right = DXWindow::Get().GetWidth();
	m_mainScissorRect.bottom = DXWindow::Get().GetHeigth();
}

void DXWindow::Update()
{
	MSG msg;
	while (PeekMessageW(&msg, m_window, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
}

void DXWindow::Present()
{
	m_swapChain->Present(1, 0);
}

void DXWindow::Resize()
{
	ReleaseBuffers();

	if (m_monitorHasChanged && m_isFullscreen)
	{
		MONITORINFO monitorInfo{};
		monitorInfo.cbSize = sizeof(monitorInfo);

		if (GetMonitorInfoW(m_currentMonitor, &monitorInfo))
		{
			m_width = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
			m_height = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;

			SetWindowPos(m_window, nullptr,
				monitorInfo.rcMonitor.left,
				monitorInfo.rcMonitor.top,
				m_width,
				m_height,
				SWP_NOZORDER
			);
		}
	}
	else
	{
		RECT clientRect;
		if (GetClientRect(m_window, &clientRect))
		{
			m_width = clientRect.right - clientRect.left;
			m_height = clientRect.bottom - clientRect.top;			
		}
	}	

	D3EZ_CHECK_HR_D(m_swapChain->ResizeBuffers((UINT)GetFrameCount(), m_width, m_height, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING), "m_swapChain->ResizeBuffers() FAILED");

	if (m_ZBuffer != nullptr)
	{
		m_ZBuffer->Resize(m_width, m_height);
	}
	SetViewPort();

	m_shouldResize = false;	
	GetBuffers();
}

void DXWindow::SetFullscreen(bool enabled)
{
	//Update window styling
	DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
	DWORD exStyle = WS_EX_OVERLAPPEDWINDOW | WS_EX_APPWINDOW;
	if (enabled)
	{
		style = WS_POPUP | WS_VISIBLE;
		exStyle = WS_EX_APPWINDOW;
	}

	SetWindowLongW(m_window, GWL_STYLE, style);
	SetWindowLongW(m_window, GWL_EXSTYLE, exStyle);

	//Adjust window size
	if (enabled)
	{		
		HMONITOR monitor = MonitorFromWindow(m_window, MONITOR_DEFAULTTONEAREST);
		MONITORINFO monitorInfo{};
		monitorInfo.cbSize = sizeof(monitorInfo);
		if (GetMonitorInfoW(monitor, &monitorInfo))
		{
			SetWindowPos(m_window, nullptr,
				monitorInfo.rcMonitor.left,
				monitorInfo.rcMonitor.top,
				monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
				monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
				SWP_NOZORDER
			);
			m_shouldResize = true;
		}
	}
	else
	{
		ShowWindow(m_window, SW_MAXIMIZE);
	}

	m_isFullscreen = enabled;
}

void DXWindow::Shutdown()
{
	ReleaseBuffers();

	m_rtvHeap.Release();

	m_swapChain.Release();

	if (m_window)
	{
		DestroyWindow(m_window);
	}

	if (m_windowClass)
	{
		UnregisterClassW((LPWSTR)m_windowClass, GetModuleHandle(nullptr));
	}
}

bool DXWindow::MonitorHasChanged()
{
	HMONITOR newMonitor = MonitorFromWindow(m_window, MONITOR_DEFAULTTONEAREST);
	if (newMonitor != m_currentMonitor)
	{
		m_currentMonitor = newMonitor;

		MONITORINFO monitorInfo = { sizeof(monitorInfo) };
		if (GetMonitorInfoW(newMonitor, &monitorInfo))
		{
			m_width = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
			m_height = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;
			m_monitorHasChanged = true;
			return true;
		}
	}
	
	return false;
}

void DXWindow::BeginFrame(ID3D12GraphicsCommandList*& cmdList)
{
	m_currentBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

	D3D12_RESOURCE_BARRIER barrier;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = m_buffers[m_currentBufferIndex];
	barrier.Transition.Subresource = 0;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

	cmdList->ResourceBarrier(1, &barrier);

	if (m_ZBuffer != nullptr)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_ZBuffer->GetDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();

		cmdList->OMSetRenderTargets(1, &m_rtvHandles[m_currentBufferIndex], false, &dsvHandle);

		cmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}	
	else
	{
		cmdList->OMSetRenderTargets(1, &m_rtvHandles[m_currentBufferIndex], false, nullptr);
	}

	cmdList->ClearRenderTargetView(m_rtvHandles[m_currentBufferIndex], m_backGroundColor, 0, nullptr);	

	cmdList->RSSetViewports(1, &m_mainViewport);

	cmdList->RSSetScissorRects(1, &m_mainScissorRect);
}

void DXWindow::EndFrame(ID3D12GraphicsCommandList*& cmdList)
{
	D3D12_RESOURCE_BARRIER barrier;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = m_buffers[m_currentBufferIndex];
	barrier.Transition.Subresource = 0;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

	cmdList->ResourceBarrier(1, &barrier);
}

bool DXWindow::GetBuffers()
{
	// Get buffers
	for (size_t i = 0; i < FrameCount; ++i)
	{
		if (FAILED(m_swapChain->GetBuffer((UINT)i, IID_PPV_ARGS(&m_buffers[i]))))
		{
			return false;
		}

		D3D12_RENDER_TARGET_VIEW_DESC rtv{};
		rtv.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtv.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		rtv.Texture2D.MipSlice = 0;
		rtv.Texture2D.PlaneSlice = 0;
		DXContext::Get().GetDevice()->CreateRenderTargetView(m_buffers[i], &rtv, m_rtvHandles[i]);
		m_buffers[i].Get()->SetName(L"Main_RTV");
	}

	return true;
}

void DXWindow::ReleaseBuffers()
{
	// Release buffers
	for (size_t i = 0; i < FrameCount; ++i)
	{
		m_buffers[i].Release();	
	}
	// Release ZBuffer
	if (m_ZBuffer != nullptr)
	{
		m_ZBuffer->Release();
	}
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK DXWindow::OnWindowMessage(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(window, message, wParam, lParam))
		return true;

	switch (message)
	{	
		case WM_SIZE:
			if (lParam && (HIWORD(lParam)!= Get().m_height || LOWORD(lParam) != Get().m_width))
			{
				Get().m_shouldResize = true;
			}
			break;

		case WM_MOVE:
			if (!Get().m_shouldResize && Get().MonitorHasChanged())
			{
				Get().m_shouldResize = true;
			}
			break;

		case WM_KEYDOWN:
			if (wParam == VK_F11 || wParam == 0x46)
			{
				Get().SetFullscreen(!Get().IsFullscreen());
			}
			break;

		case WM_SETFOCUS:
			if(Get().m_camera)
			{
				Get().m_camera->ControlMode(true);
			}
			break;

		case WM_KILLFOCUS:
			if (Get().m_camera)
			{
				Get().m_camera->ControlMode(false);
			}
			break;

		case WM_CLOSE:
			Get().m_shouldClose = true;
			return 0;
	}

	return DefWindowProc(window, message, wParam, lParam);
}
