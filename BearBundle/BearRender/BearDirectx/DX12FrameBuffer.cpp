#include "DX12PCH.h"
bsize FrameBufferCounter = 0;
DX12FrameBuffer::DX12FrameBuffer(const BearFrameBufferDescription& description) :Description(description)
{
	FrameBufferCounter++;
	BEAR_CHECK(!description.RenderPass.empty());
	RenderPassRef = static_cast<DX12RenderPass*>(Description.RenderPass.get());
	Width = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
	Height = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
	for (Count = 0; Count < 8; Count++)
	{
		if (Description.RenderTargets[Count].empty())
		{
			break;
		}
		BEAR_CHECK(Description.RenderTargets[Count]->GetType() == BearTextureType::RenderTarget);
		auto texture = static_cast<DX12Texture2D*>(Description.RenderTargets[Count].get());
		Width =BearMath::min(Width,bsize(texture->TextureDesc.Width));
		Height = BearMath::min(Height, bsize(texture->TextureDesc.Height));
		BEAR_CHECK(texture->RTVFormat == RenderPassRef->Description.RenderTargets[Count].Format);

	}

	BEAR_CHECK(RenderPassRef->CountRenderTarget == Count);
	{
		{
			D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
			rtvHeapDesc.NumDescriptors = static_cast<UINT>( Count);
			rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

			R_CHK(Factory->Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&RtvHeap)));

		}
		CD3DX12_CPU_DESCRIPTOR_HANDLE RTVHandle(RtvHeap->GetCPUDescriptorHandleForHeapStart());
		for (bsize i = 0; Count > i; i++)
		{
			D3D12_RENDER_TARGET_VIEW_DESC DESC = {};


			auto texture = static_cast<DX12Texture2D*>(Description.RenderTargets[i].get());
			DESC.Format = texture->TextureDesc.Format;
			DESC.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			Factory->Device->CreateRenderTargetView(texture->TextureBuffer.Get(), &DESC, RTVHandle);

			RTVHandle.Offset(1, Factory->RtvDescriptorSize);
		}
	}

	if (Description.DepthStencil.empty())
	{
		BEAR_CHECK(RenderPassRef->Description.DepthStencil.Format == BearDepthStencilFormat::None);
	}
	else
	{
		BEAR_CHECK(Description.DepthStencil.get()->GetType() == BearTextureType::DepthStencil);
		BEAR_CHECK(RenderPassRef->Description.DepthStencil.Format == static_cast<DX12Texture2D*>(Description.DepthStencil.get())->DSVFormat);
		{
			D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
			rtvHeapDesc.NumDescriptors = 1;
			rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

			R_CHK(Factory->Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&DsvHeap)));

		}
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(DsvHeap->GetCPUDescriptorHandleForHeapStart());
		D3D12_DEPTH_STENCIL_VIEW_DESC DESC = {};
		DESC.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		DESC.Format = static_cast<DX12Texture2D*>(Description.DepthStencil.get())->TextureDesc.Format;
		Factory->Device->CreateDepthStencilView(static_cast<DX12Texture2D*>(Description.DepthStencil.get())->TextureBuffer.Get(), &DESC, rtvHandle);
		DepthStencilRef = rtvHandle;
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE RTVHandle(RtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (bsize i = 0; Count > i; i++)
	{
		RenderTargetRefs[i] = RTVHandle;
		RTVHandle.Offset(1, Factory->RtvDescriptorSize);
	}


}

DX12FrameBuffer::~DX12FrameBuffer()
{
	FrameBufferCounter--;

}

void DX12FrameBuffer::Unlock(ID3D12GraphicsCommandListX* command_list)
{
	for (bsize i = 0; Count > i; i++)
	{
		auto Texture = static_cast<DX12Texture2D*>(Description.RenderTargets[i].get());
		auto ResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(Texture->TextureBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE| D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		command_list->ResourceBarrier(1, &ResourceBarrier);
	}
}

void DX12FrameBuffer::Lock(ID3D12GraphicsCommandListX* command_list)
{
	for (bsize i = 0; Count > i; i++)
	{
		auto Texture = static_cast<DX12Texture2D*>(Description.RenderTargets[i].get());
		auto ResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(Texture->TextureBuffer.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
		command_list->ResourceBarrier(1, &ResourceBarrier);
	}
}
