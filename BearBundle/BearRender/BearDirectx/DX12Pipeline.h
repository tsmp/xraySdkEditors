#pragma once
class DX12Pipeline:public virtual BearRHI::BearRHIPipeline
{
public:
	virtual bool IsComputePipeline() {	return false;	}
	virtual void Set(ID3D12GraphicsCommandListX* command_list) = 0;
};
