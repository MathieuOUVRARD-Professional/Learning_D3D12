#include "DXContext.h"


/*
*	GPU
*	<--
*	Command Queue (WorkSet, WorkSet, Sync)
*	<--
*	CPU WorkSet(Setup, Setup, ChangeState, Setup, Draw)
*	
*/

bool DXContext::Init()
{
	if (SUCCEEDED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device))))
	{
		return true;
	}
	
	return false;
}

void DXContext::Shutdown()
{
	m_device.Release();
}
