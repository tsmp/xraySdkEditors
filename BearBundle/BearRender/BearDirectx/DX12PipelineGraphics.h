#pragma once
class DX12PipelineGraphics :public BearRHI::BearRHIPipelineGraphics,public DX12Pipeline
{
public:
	DX12PipelineGraphics(const BearPipelineGraphicsDescription&description);
	virtual ~DX12PipelineGraphics();
	virtual void* QueryInterface(int type);
	virtual BearPipelineType GetType();
	virtual void Set(ID3D12GraphicsCommandListX* command_list);
	ComPtr<ID3D12PipelineState> PipelineState;
	D3D_PRIMITIVE_TOPOLOGY TopologyType;
	BearFactoryPointer<BearRHI::BearRHIRootSignature> RootSignature;
	DX12RootSignature* RootSignaturePointer;
};
