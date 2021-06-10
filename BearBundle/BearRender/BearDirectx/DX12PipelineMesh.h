#pragma once
class DX12PipelineMesh :public BearRHI::BearRHIPipelineMesh,public DX12Pipeline
{
public:
	DX12PipelineMesh(const BearPipelineMeshDescription&description);
	virtual ~DX12PipelineMesh();
	virtual void* QueryInterface(int type);
	virtual BearPipelineType GetType();
	virtual void Set(ID3D12GraphicsCommandListX* command_list);
	ComPtr<ID3D12PipelineState> PipelineState;
	D3D_PRIMITIVE_TOPOLOGY TopologyType;

	BearFactoryPointer<BearRHI::BearRHIRootSignature> RootSignature;
	DX12RootSignature* RootSignaturePointer;
};
