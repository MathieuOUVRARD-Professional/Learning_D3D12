#pragma once

#include"imgui.h"
#include"backends/imgui_impl_dx12.h"
#include"backends/imgui_impl_win32.h"

#include<D3D/DXContext.h>
#include<D3D/ZBuffer.h>

#include <Support/WinInclude.h>
#include <Support/ComPointer.h>
#include <Support/Camera.h>

#include <Util/HRException.h>

class DXWindow
{
public:
	bool Init();
	void BindMainRenderTarget(ID3D12GraphicsCommandList*& cmdList);
	void SetViewPort();
	void Update();
	void Present();
	void Resize();
	void SetFullscreen(bool enabled);
	void Shutdown();

	bool MonitorHasChanged();

	void BeginFrame(ID3D12GraphicsCommandList*& cmdList);
	void EndFrame(ID3D12GraphicsCommandList*& cmdList);	

	inline HWND GetWindow() const
	{
		return m_window;
	}

	inline bool ShouldResize() const
	{
		return m_shouldResize;
	}

	inline bool ShouldClose() const
	{
		return m_shouldClose;
	}

	inline bool IsFullscreen() const
	{
		return m_isFullscreen;
	}

	inline UINT GetWidth() const
	{
		return m_width;
	}

	inline UINT GetHeigth() const
	{
		return m_height;
	}

	inline float GetWindowRatio() const
	{
		return ((float)GetWidth() / (float)GetHeigth());
	}

	inline void SetBackgroundColor(float color[4])
	{
		for (int i = 0; i < 4; i++)
		{
			m_backGroundColor[i] = color[i];
		}
	}
	inline void SetMainCamera(Camera& mainCamera)
	{
		m_camera = &mainCamera;
	}

	inline void SetZBuffer(ZBuffer* zBuffer)
	{
		m_ZBuffer = zBuffer;
	}

	static constexpr size_t FrameCount = 2;
	static constexpr size_t GetFrameCount()
	{
		return FrameCount;
	}	

private:
	bool GetBuffers();
	void ReleaseBuffers();
	static LRESULT CALLBACK OnWindowMessage(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

private:
	HMONITOR m_currentMonitor = nullptr;

	Camera* m_camera = nullptr;
	ATOM m_windowClass = 0;
	HWND m_window = nullptr;

	bool m_shouldClose = false;
	bool m_shouldResize = false;
	bool m_isFullscreen = false; 
	bool m_monitorHasChanged = false;

	UINT m_width = 1920;
	UINT m_height = 1080;

	float m_backGroundColor[4] = { 0.067f, 0.086f, 0.110f, 1.f };

	ComPointer<IDXGISwapChain3> m_swapChain;

	ComPointer<ID3D12Resource2> m_buffers[FrameCount];

	size_t m_currentBufferIndex = 0;

	ComPointer<ID3D12DescriptorHeap> m_rtvDescHeap = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE m_rtvHandles[FrameCount];
	D3D12_VIEWPORT m_mainViewport;
	RECT m_mainScissorRect;
	ZBuffer* m_ZBuffer = nullptr;

	//Singleton 
public:
	DXWindow(const DXWindow&) = delete;
	DXWindow& operator=(const DXWindow&) = delete;

	inline static DXWindow& Get()
	{
		static DXWindow instance;
		return instance;
	}

private:
	DXWindow() = default;
};