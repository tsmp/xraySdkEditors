#pragma once
class DX12RayTracingTopLevel :public DX12ShaderResource,public BearRHI::BearRHIRayTracingTopLevel
{
public:
	DX12RayTracingTopLevel(const BearRayTracingTopLevelDescription&desc);
	virtual ~DX12RayTracingTopLevel();
	virtual void* QueryInterface(int type);
	virtual bool SetAsSRV(D3D12_GPU_VIRTUAL_ADDRESS& address, bsize offset);
	ComPtr<ID3D12Resource> TopLevelAccelerationStructure;
};

