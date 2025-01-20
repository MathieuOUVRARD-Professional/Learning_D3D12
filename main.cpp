#include <iostream>

#include <Support/WinInclude.h>
#include <Support/ComPointer.h>
#include <Support/Window.h>

#include <Debug/DebugLayer.h>

#include <D3D/DXContext.h>

int main()
{
	DXDebugLayer::Get().Init();

	if (DXContext::Get().Init() && DXWindow::Get().Init())
	{
		DXWindow::Get().SetFullscreen(true);

		while (!DXWindow::Get().ShouldClose())
		{
			// Process pending window messages
			DXWindow::Get().Update();

			//Handle resizing
			if (DXWindow::Get().ShouldResize())
			{
				// Flushing (command queue). (As much as buffer)
				DXContext::Get().Flush(DXWindow::Get().GetFrameCount());
				DXWindow::Get().Resize();
			}

			//Begin drawing
			auto* cmdList = DXContext::Get().InitCommandList();

			// A lot of setup 			

			// TODO: draw

			DXContext::Get().ExecuteCommandList();
			
			// Show me the stuff
			
			//Finish drawing and present
			DXWindow::Get().Present();
		}
		
		// Flushing (command queue). (As much as buffer)
		DXContext::Get().Flush(DXWindow::Get().GetFrameCount());

		DXWindow::Get().Shutdown();
		DXContext::Get().Shutdown();
	}

	DXDebugLayer::Get().Shutdown();
}