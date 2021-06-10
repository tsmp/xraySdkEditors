#pragma once
class DX12RootSignature :public BearRHI::BearRHIRootSignature
{
public:
	DX12RootSignature(const BearRootSignatureDescription&description);
	virtual ~DX12RootSignature();
	virtual void Set(ID3D12GraphicsCommandListX* command_list);
	ComPtr<ID3D12RootSignature> RootSignature;

	bsize SlotBuffers[16];
	bsize SlotSRVs[16];
	bsize SlotSamplers[16];
	bsize SlotUAVs[16];

	bsize CountBuffers;
	bsize CountSRVs;
	bsize CountSamplers;
	bsize CountUAVs;
};