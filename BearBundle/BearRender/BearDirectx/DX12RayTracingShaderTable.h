#pragma once
class DX12RayTracingShaderTable :public BearRHI::BearRHIRayTracingShaderTable
{
public:
	DX12RayTracingShaderTable(const BearRayTracingShaderTableDescription& Description);
	virtual ~DX12RayTracingShaderTable();
	ComPtr<ID3D12Resource> Buffer;
	bsize Size;


	D3D12_GPU_VIRTUAL_ADDRESS_RANGE RayGenerationShaderRecord;
	D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE MissShaderTable;
	D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE HitGroupTable;
	D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE CallableShaderTable;

};