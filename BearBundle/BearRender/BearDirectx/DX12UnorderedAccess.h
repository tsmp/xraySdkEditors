#pragma once
class DX12UnorderedAccess :public virtual BearRHI::BearRHIUnorderedAccess,public DX12ShaderResource
{
public:
	DX12UnorderedAccess() { bAllowUAV = false; }
	virtual bool SetAsUAV(D3D12_GPU_VIRTUAL_ADDRESS& address,bsize offset) { return false; }
	virtual bool SetAsUAV(D3D12_CPU_DESCRIPTOR_HANDLE& heap, bsize offset) { return false; }
	virtual void LockUAV(ID3D12GraphicsCommandListX*command_list) {}
	virtual void UnlockUAV(ID3D12GraphicsCommandListX* command_list) {}
protected:
	bool bAllowUAV;
	D3D12_UNORDERED_ACCESS_VIEW_DESC UAV;
};