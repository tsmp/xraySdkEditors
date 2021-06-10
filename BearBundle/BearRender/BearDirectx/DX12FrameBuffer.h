#pragma once 
class DX12FrameBuffer:public BearRHI::BearRHIFrameBuffer
{
public :
	DX12FrameBuffer(const BearFrameBufferDescription& description);
	virtual ~DX12FrameBuffer();
	void Unlock(ID3D12GraphicsCommandListX* command_list);
	void Lock(ID3D12GraphicsCommandListX* command_list);
	BearFrameBufferDescription Description;
	DX12RenderPass* RenderPassRef;
	ComPtr<ID3D12DescriptorHeap> RtvHeap;
	ComPtr<ID3D12DescriptorHeap> DsvHeap;
	CD3DX12_CPU_DESCRIPTOR_HANDLE RenderTargetRefs[8];
	CD3DX12_CPU_DESCRIPTOR_HANDLE DepthStencilRef;
	bsize Count;
	bsize Width;
	bsize Height;
};