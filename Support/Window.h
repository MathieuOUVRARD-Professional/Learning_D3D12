#pragma once

#include <Support/WinInclude.h>
#include <Support/ComPointer.h>

class DXWindow
{
public:
	bool Init();
	void Update();
	void Shutdown();

	inline bool ShouldClose() const
	{
		return m_shouldClose;
	}

private:
	static LRESULT CALLBACK OnWindowMessage(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

private:
	ATOM m_windowClass = 0;
	HWND m_window = nullptr;
	bool m_shouldClose = false;

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