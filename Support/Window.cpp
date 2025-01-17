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
	return m_window != nullptr;
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

void DXWindow::Shutdown()
{
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
