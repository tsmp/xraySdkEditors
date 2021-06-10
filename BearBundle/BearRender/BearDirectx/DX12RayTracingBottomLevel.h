#pragma once
class DX12RayTracingBottomLevel :public BearRHI::BearRHIRayTracingBottomLevel
{
public:
	DX12RayTracingBottomLevel(const BearRayTracingBottomLevelDescription&description);
	virtual ~DX12RayTracingBottomLevel();
	ComPtr<ID3D12Resource> BottomLevelAccelerationStructure;
};

