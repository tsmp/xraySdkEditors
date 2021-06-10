#pragma once
class DX12PipelineRayTracing :public BearRHI::BearRHIPipelineRayTracing,public DX12Pipeline
{
public:
	DX12PipelineRayTracing(const BearPipelineRayTracingDescription&description);
	virtual ~DX12PipelineRayTracing();
	virtual void* QueryInterface(int åype);
	virtual BearPipelineType GetType();

	virtual void Set(ID3D12GraphicsCommandListX*command_list);
	ComPtr<ID3D12StateObject> PipelineState;
	virtual bool IsComputePipeline() { return true; }
	BearFactoryPointer<BearRHI::BearRHIRootSignature> RootSignature;
	BearVector< BearFactoryPointer<BearRHI::BearRHIRootSignature>> LocalRootSignature;
	DX12RootSignature* RootSignaturePointer;
};
