#pragma once
class DX12Viewport :public BearRHI::BearRHIViewport
{
public:
	DX12Viewport(void * window_handle, bsize width, bsize height, bool fullscreen, bool vsync, const BearViewportDescription&description);
	virtual ~DX12Viewport();
	virtual void SetVSync(bool vsync);
	virtual void SetFullScreen(bool fullscreen);
	virtual void Resize(bsize width, bsize height);
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetHandle();
	void Swap();
	virtual BearRenderTargetFormat GetFormat();

	void Unlock(ID3D12GraphicsCommandListX*command_list);
	void Lock(ID3D12GraphicsCommandListX* command_list);

	BearViewportDescription Description;
	ComPtr<ID3D12CommandQueue> CommandQueue;

	bsize Width;
	bsize Height;
private:
	void ReInit(bsize Width, bsize Height);

	ComPtr<IDXGISwapChain3> m_SwapChain;
	ComPtr<ID3D12DescriptorHeap> m_RtvHeap;

	constexpr static bsize FrameCount = 2;
	UINT m_FrameIndex;
	ComPtr<ID3D12Resource> m_RenderTargets[FrameCount];

	CD3DX12_CPU_DESCRIPTOR_HANDLE m_RtvHandle;

	DXGI_SWAP_CHAIN_DESC1 m_SwapChainDesc = {};
	bool m_Fullscreen;
	bool m_VSync;
};