#pragma once
class DX12DescriptorHeap :public BearRHI::BearRHIDescriptorHeap
{
public:
	DX12DescriptorHeap(const BearDescriptorHeapDescription& description);
	virtual ~DX12DescriptorHeap();
	DX12AllocatorHeapItem UniSRVHeap;
	DX12AllocatorHeapItem SamplerHeap;
	D3D12_GPU_VIRTUAL_ADDRESS SRVs[16];
	D3D12_GPU_VIRTUAL_ADDRESS UAVs[16];

	BearFactoryPointer<BearRHI::BearRHIRootSignature> RootSignature;

	void SetGraphics(ID3D12GraphicsCommandListX* command_list);
	void SetCompute(ID3D12GraphicsCommandListX* command_list);

	virtual void SetUniformBuffer(bsize slot, BearFactoryPointer<BearRHI::BearRHIUniformBuffer> uniform_buffer, bsize offset = 0);
	virtual	void SetShaderResource(bsize slot, BearFactoryPointer<BearRHI::BearRHIShaderResource> shader_resource, bsize offset = 0);
	virtual	void SetSampler(bsize slot, BearFactoryPointer<BearRHI::BearRHISampler> sampler);
	virtual	void SetUnorderedAccess(bsize slot, BearFactoryPointer<BearRHI::BearRHIUnorderedAccess> unordered_access, bsize offset = 0);



	BearFactoryPointer<BearRHI::BearRHIUniformBuffer> UniformBuffers[16];
	bsize UniformBufferOffsets[16];
	bsize ShaderResourceOffsets[16];
	bsize UnorderedAccessOffsets[16];

	bsize CountBuffers;
	BearFactoryPointer<BearRHI::BearRHIShaderResource> ShaderResources[16];
	bsize CountSRVs;
	BearFactoryPointer<BearRHI::BearRHISampler> Samplers[16];
	bsize CountSamplers;
	BearFactoryPointer<BearRHI::BearRHIUnorderedAccess> UnorderedAccess[16];
	bsize CountUAVs;

	bsize SlotBuffers[16];
	bsize SlotSRVs[16];
	bsize SlotSamplers[16];
	bsize SlotUAVs[16];
};