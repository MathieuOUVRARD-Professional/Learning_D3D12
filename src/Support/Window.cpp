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
	wcex.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
	wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
	wcex.hbrBackground = nullptr;
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = L"D3D12ExWindowClass";	
	wcex.hIconSm = LoadIconW(nullptr, IDI_APPLICATION);

	m_windowClass = RegisterClassExW(&wcex);
	if (m_windowClass == 0)
	{
		return false;
	}

	// Place window on current cursor's screen
	POINT cursorPos{ 0,0 };
	GetCursorPos(&cursorPos);
	HMONITOR monitor = MonitorFromPoint(cursorPos, MONITOR_DEFAULTTOPRIMARY);
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

	if (FAILED(DXContext::Get().GetDevice()->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&m_rtvDescHeap))))
	{
		return false;
	}
	
	//Create handles to view
	auto firstHandle = m_rtvDescHeap->GetCPUDescriptorHandleForHeapStart();
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

	RECT clientRect;
	if (GetClientRect(m_window, &clientRect))
	{
		m_width = clientRect.right - clientRect.left;
		m_height = clientRect.bottom - clientRect.top;

		//TODO: Validate result of resizing
		m_swapChain->ResizeBuffers((UINT)GetFrameCount(), m_width, m_height, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING);
		m_shouldResize = false;
	}	

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

	m_rtvDescHeap.Release();

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

void DXWindow::BeginFrame(ID3D12GraphicsCommandList* cmdList)
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

	cmdList->ClearRenderTargetView(m_rtvHandles[m_currentBufferIndex], m_backGroundColor, 0, nullptr);

	cmdList->OMSetRenderTargets(1, &m_rtvHandles[m_currentBufferIndex], false, nullptr);
}

void DXWindow::EndFrame(ID3D12GraphicsCommandList* cmdList)
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

		case WM_KEYDOWN:
			if (wParam == VK_F11 || wParam == 0x46)
			{
				Get().SetFullscreen(!Get().IsFullscreen());
			}
			break;

		case WM_CLOSE:
			Get().m_shouldClose = true;
			return 0;
	}

	return DefWindowProc(window, message, wParam, lParam);
}
