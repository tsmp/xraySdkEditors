#include "DX12PCH.h"
bsize ViewportCounter = 0;
DX12Viewport::DX12Viewport(void * window_handle, bsize width, bsize height, bool fullscreen, bool vsync, const BearViewportDescription&description):Description(description), m_Fullscreen(fullscreen),m_VSync(vsync), Width(width),Height(height)
{
	ViewportCounter++;
	{
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		R_CHK(Factory->Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&CommandQueue)));

	}
	{
		m_SwapChainDesc.BufferCount = FrameCount;
		m_SwapChainDesc.Width = static_cast<UINT>(width);
		m_SwapChainDesc.Height = static_cast<UINT>(height);
		m_SwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		m_SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		m_SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		m_SwapChainDesc.SampleDesc.Count = 1;


		ComPtr<IDXGISwapChain1> SwapChain;
		R_CHK(Factory->GIFactory->CreateSwapChainForHwnd(CommandQueue.Get(),reinterpret_cast<HWND>(window_handle),&m_SwapChainDesc,nullptr,nullptr,&SwapChain));
		R_CHK(SwapChain.As(&m_SwapChain));
		m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();
	}
	// Create descriptor heaps.
	{
		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC RTVHeapDesc = {};
		RTVHeapDesc.NumDescriptors = FrameCount;
		RTVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		RTVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		R_CHK(Factory->Device->CreateDescriptorHeap(&RTVHeapDesc, IID_PPV_ARGS(&m_RtvHeap)));

		
	}

	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE Handle(m_RtvHeap->GetCPUDescriptorHandleForHeapStart());
		for (UINT n = 0; n < FrameCount; n++)
		{
			R_CHK(m_SwapChain->GetBuffer(n, IID_PPV_ARGS(&m_RenderTargets[n])));
			Factory->Device->CreateRenderTargetView(m_RenderTargets[n].Get(), nullptr, Handle);
			Handle.Offset(1,Factory-> RtvDescriptorSize);
		}
	}
}

DX12Viewport::~DX12Viewport()
{	
	ViewportCounter--;

}

void DX12Viewport::ReInit(bsize width, bsize height)
{
	

	for (UINT n = 0; n < FrameCount; n++)
	{
		m_RenderTargets[n].Reset();
	}
	m_SwapChainDesc.Width = static_cast<UINT>(width);
	m_SwapChainDesc.Height = static_cast<UINT>(height);
	R_CHK(m_SwapChain->ResizeBuffers(m_SwapChainDesc.BufferCount,m_SwapChainDesc.Width,m_SwapChainDesc.Height,m_SwapChainDesc.Format,DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();

	// Create descriptor heaps.
	{
		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC RTVHeapDesc = {};
		RTVHeapDesc.NumDescriptors = FrameCount;
		RTVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		RTVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		R_CHK(Factory->Device->CreateDescriptorHeap(&RTVHeapDesc, IID_PPV_ARGS(&m_RtvHeap)));


	}

	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE Handle(m_RtvHeap->GetCPUDescriptorHandleForHeapStart());
		for (UINT n = 0; n < FrameCount; n++)
		{
			R_CHK(m_SwapChain->GetBuffer(n, IID_PPV_ARGS(&m_RenderTargets[n])));
			Factory->Device->CreateRenderTargetView(m_RenderTargets[n].Get(), nullptr, Handle);
			Handle.Offset(1, Factory->RtvDescriptorSize);
		}
	}


	Width = width;
	Height = height;
}

void DX12Viewport::SetVSync(bool vsync)
{
	m_VSync = vsync;
}

void DX12Viewport::SetFullScreen(bool fullscreen)
{
	if (m_Fullscreen == fullscreen)return;
	if (fullscreen)
	{
		/*HMONITOR hMonitor = MonitorFromWindow((HWND)m_WindowHandle, MONITOR_DEFAULTTOPRIMARY);
		MONITORINFOEX MonitorInfo;
		memset(&MonitorInfo, 0, sizeof(MONITORINFOEX));
		MonitorInfo.cbSize = sizeof(MONITORINFOEX);
		GetMonitorInfo(hMonitor, &MonitorInfo);

		DEVMODE Mode;
		Mode.dmSize = sizeof(DEVMODE);
		Mode.dmBitsPerPel = 32;
		Mode.dmPelsWidth = static_cast<DWORD>(Width);
		Mode.dmPelsHeight = static_cast<DWORD>(Height);
		Mode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		ChangeDisplaySettingsEx(MonitorInfo.szDevice, &Mode, NULL, CDS_FULLSCREEN, NULL);*/
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = static_cast<DWORD>(Width);
		dmScreenSettings.dmPelsHeight = static_cast<DWORD>(Height);
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Change the display settings to full screen.
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);


	}
	else
	{
		ChangeDisplaySettings(NULL, 0);
	}
	m_Fullscreen = fullscreen;
	ReInit(Width, Height);
}

void DX12Viewport::Resize(bsize width, bsize height)
{
	if (height == Height && width == Width)return;
	ReInit(width, height);

}

CD3DX12_CPU_DESCRIPTOR_HANDLE DX12Viewport::GetHandle()
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE Handle(m_RtvHeap->GetCPUDescriptorHandleForHeapStart(), m_FrameIndex, Factory->RtvDescriptorSize);
	return Handle;
}

void DX12Viewport::Lock(ID3D12GraphicsCommandListX* CommandList)
{
	auto Transition = CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTargets[m_FrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	CommandList->ResourceBarrier(1, &Transition);
}

void DX12Viewport::Unlock(ID3D12GraphicsCommandListX * CommandList)
{
	auto Transition = CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTargets[m_FrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	CommandList->ResourceBarrier(1, &Transition);
}

void DX12Viewport::Swap()
{
	R_CHK(m_SwapChain->Present(m_VSync, 0));
	m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();
}

BearRenderTargetFormat DX12Viewport::GetFormat()
{
	return BearRenderTargetFormat::R8G8B8A8;
}

