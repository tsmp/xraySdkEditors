#include "DX12PCH.h"
bsize DescriptorHeapCounter = 0;
DX12DescriptorHeap::DX12DescriptorHeap(const BearDescriptorHeapDescription& description)
{
	DescriptorHeapCounter++;
	CountBuffers = 0;
	CountSamplers = 0;
	CountSRVs = 0;
	bear_fill(SRVs);
	bear_fill(UAVs);
	{	
		BEAR_ASSERT(!description.RootSignature.empty());

		CountBuffers = static_cast<const DX12RootSignature*>(description.RootSignature.get())->CountBuffers;
		CountSRVs = static_cast<const DX12RootSignature*>(description.RootSignature.get())->CountSRVs;
		CountSamplers = static_cast<const DX12RootSignature*>(description.RootSignature.get())->CountSamplers;
		CountUAVs = static_cast<const DX12RootSignature*>(description.RootSignature.get())->CountUAVs;

		memcpy(SlotBuffers, static_cast<const DX12RootSignature*>(description.RootSignature.get())->SlotBuffers, 16 * sizeof(bsize));
		memcpy(SlotSRVs, static_cast<const DX12RootSignature*>(description.RootSignature.get())->SlotSRVs, 16 * sizeof(bsize));
		memcpy(SlotSamplers, static_cast<const DX12RootSignature*>(description.RootSignature.get())->SlotSamplers, 16 * sizeof(bsize));
		memcpy(SlotUAVs, static_cast<const DX12RootSignature*>(description.RootSignature.get())->SlotSRVs, 16 * sizeof(bsize));

		if (CountSamplers)
		{
			SamplerHeap = Factory->SamplersHeapAllocator.allocate(CountSamplers,Factory->Device.Get());
		}
		if (CountBuffers + CountSRVs)
		{

			UniSRVHeap = Factory->ShaderResourceHeapAllocator.allocate(CountBuffers + CountSRVs + CountUAVs, Factory->Device.Get());
		}
	}
	RootSignature = description.RootSignature;
}

DX12DescriptorHeap::~DX12DescriptorHeap()
{
	Factory->ShaderResourceHeapAllocator.free(UniSRVHeap);
	Factory->SamplersHeapAllocator.free(SamplerHeap);
	DescriptorHeapCounter--;
}

void DX12DescriptorHeap::SetGraphics(ID3D12GraphicsCommandListX* command_list)
{
	ID3D12DescriptorHeap* Heaps[2];
	UINT Count = 0;
	if (UniSRVHeap.DescriptorHeap.Get())
	{
		Heaps[Count++] = UniSRVHeap.DescriptorHeap.Get();
	}

	if (SamplerHeap.DescriptorHeap.Get())
	{
		Heaps[Count++] = SamplerHeap.DescriptorHeap.Get();
	}
	command_list->SetDescriptorHeaps(Count, Heaps);
	bsize Offset = 0;
	if (UniSRVHeap.Size)
	{
		CD3DX12_GPU_DESCRIPTOR_HANDLE CbvHandle(UniSRVHeap.DescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		CbvHandle.Offset(Factory->CbvSrvUavDescriptorSize,static_cast<UINT>( UniSRVHeap.Id));
		for (bsize i = 0; i < CountBuffers+ CountSRVs+CountUAVs; i++)
		{
			if ((i >= CountSRVs+ CountBuffers) && UAVs[i - (CountBuffers + CountSRVs)])
			{
				command_list->SetGraphicsRootUnorderedAccessView(static_cast<UINT>(Offset++), UAVs[i - (CountBuffers + CountSRVs)]);
			}
			else if ((i>=CountBuffers&& i < CountBuffers+ CountSRVs)&&SRVs[i- CountBuffers])
			{
				command_list->SetGraphicsRootShaderResourceView(static_cast<UINT>(Offset++), SRVs[i-CountBuffers]);
			}
			else
			{
				command_list->SetGraphicsRootDescriptorTable(static_cast<UINT>(Offset++), CbvHandle);	
			}
			CbvHandle.Offset(Factory->CbvSrvUavDescriptorSize);
		}
	}
	else
	{
		if (CountSRVs)
		{
			for (bsize i = 0; i < CountSRVs; i++)
			{
				BEAR_CHECK(SRVs[i]);
				command_list->SetGraphicsRootShaderResourceView(static_cast<UINT>(Offset++), SRVs[i]);
			}
		}
		if (CountUAVs)
		{
			for (bsize i = 0; i < CountUAVs; i++)
			{
				BEAR_CHECK(UAVs[i]);
				command_list->SetGraphicsRootUnorderedAccessView(static_cast<UINT>(Offset++), UAVs[i]);
			}
		}
	}
	if (SamplerHeap.Size)
	{
		CD3DX12_GPU_DESCRIPTOR_HANDLE SamplersHandle(SamplerHeap.DescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		SamplersHandle.Offset(Factory->SamplerDescriptorSize, static_cast<UINT>(SamplerHeap.Id));
		for (bsize i = 0; i < CountSamplers; i++)
		{
		
			command_list->SetGraphicsRootDescriptorTable(static_cast<UINT>(Offset++), SamplersHandle);
			SamplersHandle.Offset(Factory->SamplerDescriptorSize);
		}
	}
}
void DX12DescriptorHeap::SetCompute(ID3D12GraphicsCommandListX* command_list)
{
	ID3D12DescriptorHeap* Heaps[2];
	UINT Count = 0;
	if (UniSRVHeap.DescriptorHeap.Get())
	{
		Heaps[Count++] = UniSRVHeap.DescriptorHeap.Get();
	}

	if (SamplerHeap.DescriptorHeap.Get())
	{
		Heaps[Count++] = SamplerHeap.DescriptorHeap.Get();
	}
	command_list->SetDescriptorHeaps(Count, Heaps);
	bsize Offset = 0;
	if (UniSRVHeap.Size)
	{
		CD3DX12_GPU_DESCRIPTOR_HANDLE CbvHandle(UniSRVHeap.DescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		CbvHandle.Offset(Factory->CbvSrvUavDescriptorSize, static_cast<UINT>(UniSRVHeap.Id));
		for (bsize i = 0; i < CountBuffers + CountSRVs + CountUAVs; i++)
		{
			if ((i >= CountSRVs + CountBuffers) && UAVs[i - (CountBuffers + CountSRVs)])
			{
				command_list->SetComputeRootUnorderedAccessView(static_cast<UINT>(Offset++), UAVs[i - (CountBuffers + CountSRVs)]);
			}
			else if ((i >= CountBuffers && i < CountBuffers + CountSRVs) && SRVs[i - CountBuffers])
			{
				command_list->SetComputeRootShaderResourceView(static_cast<UINT>(Offset++), SRVs[i - CountBuffers]);
			}
			else
			{
				command_list->SetComputeRootDescriptorTable(static_cast<UINT>(Offset++), CbvHandle);
			}
			CbvHandle.Offset(Factory->CbvSrvUavDescriptorSize);
		}
	}
	else
	{
		if (CountSRVs)
		{
			for (bsize i = 0; i < CountSRVs; i++)
			{
				BEAR_CHECK(SRVs[i]);
				command_list->SetComputeRootShaderResourceView(static_cast<UINT>(Offset++), SRVs[i]);
			}
		}
		if (CountUAVs)
		{
			for (bsize i = 0; i < CountUAVs; i++)
			{
				BEAR_CHECK(UAVs[i]);
				command_list->SetComputeRootUnorderedAccessView(static_cast<UINT>(Offset++), UAVs[i]);
			}
		}
	}
	if (SamplerHeap.Size)
	{
		CD3DX12_GPU_DESCRIPTOR_HANDLE SamplersHandle(SamplerHeap.DescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		SamplersHandle.Offset(Factory->SamplerDescriptorSize, static_cast<UINT>(SamplerHeap.Id));
		for (bsize i = 0; i < CountSamplers; i++)
		{

			command_list->SetComputeRootDescriptorTable(static_cast<UINT>(Offset++), SamplersHandle);
			SamplersHandle.Offset(Factory->SamplerDescriptorSize);
		}
	}
}
void DX12DescriptorHeap::SetUniformBuffer(bsize slot, BearFactoryPointer<BearRHI::BearRHIUniformBuffer> resource, bsize offset )
{
	if (resource.empty())return;
	BEAR_CHECK(slot < 16);
	slot = SlotBuffers[slot];
	BEAR_CHECK(slot < CountBuffers);


	if (UniformBuffers[slot] == resource)
	{
		if (UniformBufferOffsets[slot] == offset)
		{
			return;
		}
		
	}
	UniformBufferOffsets[slot] = offset;
	UniformBuffers[slot] = resource;

	auto* buffer = const_cast<DX12UniformBuffer*>(static_cast<const DX12UniformBuffer*>(resource.get()));
	CD3DX12_CPU_DESCRIPTOR_HANDLE CbvHandle(UniSRVHeap.DescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	CbvHandle.Offset(Factory->CbvSrvUavDescriptorSize , static_cast<UINT>( slot+ UniSRVHeap.Id));

	D3D12_CONSTANT_BUFFER_VIEW_DESC UniformBufferView;

	BEAR_CHECK(buffer->GetCount() > offset);
	UniformBufferView.BufferLocation = buffer->UniformBuffer->GetGPUVirtualAddress()+ offset* buffer->GetStride();
	UniformBufferView.SizeInBytes = static_cast<UINT>(buffer->GetStride());
	Factory->Device->CreateConstantBufferView(&UniformBufferView, CbvHandle);

}
void DX12DescriptorHeap::SetShaderResource(bsize slot, BearFactoryPointer<BearRHI::BearRHIShaderResource> resource, bsize offset)
{
	if (resource.empty())return;
	BEAR_CHECK(slot < 16);
	slot = SlotSRVs[slot];
	BEAR_ASSERT(slot < CountSRVs);
	if (ShaderResources[slot] == resource)
	{
		if(ShaderResourceOffsets[slot]==offset)
		return;
	}

	ShaderResources[slot] = resource;
	ShaderResourceOffsets[slot] = offset;
	auto* buffer = reinterpret_cast<DX12ShaderResource*>(resource.get()->QueryInterface(DX12Q_ShaderResource));
	BEAR_CHECK(buffer);

	if (!buffer->SetAsSRV(SRVs[slot], offset))
	{
		SRVs[slot] = 0;
		CD3DX12_CPU_DESCRIPTOR_HANDLE CbvHandle(UniSRVHeap.DescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		CbvHandle.Offset(Factory->CbvSrvUavDescriptorSize, static_cast<UINT>(slot + UniSRVHeap.Id+CountBuffers));
		if (!buffer->SetAsSRV(CbvHandle))
		{
			BEAR_CHECK(0);
		}
	}
}
void DX12DescriptorHeap::SetSampler(bsize slot, BearFactoryPointer<BearRHI::BearRHISampler> resource)
{
	if (resource.empty())return;
	BEAR_CHECK(slot < 16);
	slot = SlotSamplers[slot];
	BEAR_ASSERT(slot < CountSamplers);
	if (Samplers[slot] == resource)return;
	Samplers[slot] = resource;
	CD3DX12_CPU_DESCRIPTOR_HANDLE CbvHandle(SamplerHeap.DescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	CbvHandle.Offset(Factory->SamplerDescriptorSize, static_cast<UINT>(slot+ SamplerHeap.Id));
	auto* Buffer = static_cast<const DX12SamplerState*>(resource.get());
	CD3DX12_CPU_DESCRIPTOR_HANDLE ÑpuDescriptorHandle(Buffer->ShaderResource.DescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	ÑpuDescriptorHandle.Offset(Factory->CbvSrvUavDescriptorSize, static_cast<UINT>(Buffer->ShaderResource.Id));
	Factory->Device->CopyDescriptorsSimple(1, CbvHandle, ÑpuDescriptorHandle, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

}

void DX12DescriptorHeap::SetUnorderedAccess(bsize slot, BearFactoryPointer<BearRHI::BearRHIUnorderedAccess> resource, bsize offset)
{
	if (resource.empty())return;
	BEAR_CHECK(slot < 16);
	slot = SlotUAVs[slot];
	BEAR_ASSERT(slot < CountUAVs);
	if (UnorderedAccess[slot] == resource)
	{
		if (UnorderedAccessOffsets[slot] == offset)
			return;
	}

	UnorderedAccess[slot] = resource;
	UnorderedAccessOffsets[slot] = offset;
	auto* buffer = reinterpret_cast<DX12UnorderedAccess*>(resource.get()->QueryInterface(DX12Q_UnorderedAccess));
	BEAR_CHECK(buffer);

	if (!buffer->SetAsUAV(UAVs[slot], offset))
	{
		UAVs[slot] = 0;
		CD3DX12_CPU_DESCRIPTOR_HANDLE CbvHandle(UniSRVHeap.DescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		CbvHandle.Offset(Factory->CbvSrvUavDescriptorSize, static_cast<UINT>(slot + UniSRVHeap.Id + CountBuffers+CountUAVs));
		if (!buffer->SetAsUAV(CbvHandle, offset))
		{
			BEAR_CHECK(0);
		}
	}
}
