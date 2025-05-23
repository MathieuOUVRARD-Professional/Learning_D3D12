#include "DebugLayer.h"

bool DXDebugLayer::Init()
{
	#ifdef _DEBUG
	// INIT D3D12 Debug layer
	if(SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_d3d12Debug))))
	{
		m_d3d12Debug->EnableDebugLayer();

		// Init DXGI Debug
		if(SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&m_dxgiDebug))))
		{
			m_dxgiDebug->EnableLeakTrackingForThread();

			return true;
		}
	}
	#endif // _DEBUG
	return false;
}

void DXDebugLayer::Shutdown()
{
	#ifdef _DEBUG
	if (m_dxgiDebug)
	{
		OutputDebugStringW(L"==============================================\r\nDXGI Reports living device objects:\r\n==============================================\r\n");

		m_dxgiDebug->ReportLiveObjects(
			DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL)
		);
		OutputDebugString(L"==============================================\r\n");
	}

	m_d3d12Debug.Release();
	m_dxgiDebug.Release();
	#endif // _DEBUG
}
