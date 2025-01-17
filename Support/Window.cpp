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
	swapChainDescription.BufferCount		= GetFrameCount();
	swapChainDescription.Scaling			= DXGI_SCALING_STRETCH;
	swapChainDescription.SwapEffect			= DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDescription.AlphaMode			= DXGI_ALPHA_MODE_IGNORE;
	swapChainDescription.Flags				= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

	
	//swapChainFullscreenDescripton.RefreshRate;
	//swapChainFullscreenDescripton.ScanlineOrdering;
	//swapChainFullscreenDescripton.Scaling;
	swapChainFullscreenDescripton.Windowed = true;

	// Swap Chain
	auto& factory = DXContext::Get().GetFactory();
	ComPointer<IDXGISwapChain1> swapChain1;
	factory->CreateSwapChainForHwnd(DXContext::Get().GetCommandQueue(), m_window, &swapChainDescription, &swapChainFullscreenDescripton, nullptr, &swapChain1);
	if (!swapChain1.QueryInterface(m_swapChain))
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

void DXWindow::Shutdown()
{
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

LRESULT CALLBACK DXWindow::OnWindowMessage(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CLOSE:
		Get().m_shouldClose = true;
		return 0;
	}

	return DefWindowProc(window, message, wParam, lParam);
}
