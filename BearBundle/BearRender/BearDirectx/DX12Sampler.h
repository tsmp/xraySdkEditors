#pragma once
class DX12SamplerState :public BearRHI::BearRHISampler
{
public:
	DX12SamplerState(const BearSamplerDescription& description);
	virtual ~DX12SamplerState();
	D3D12_SAMPLER_DESC SamplerDesc;
	DX12AllocatorHeapItem ShaderResource;
};