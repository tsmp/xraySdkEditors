#pragma once
class DX12StructuredBuffer:public BearRHI::BearRHIStructuredBuffer,public DX12UnorderedAccess
{
public:
	DX12StructuredBuffer(bsize size,void* data, bool uav);
	virtual ~DX12StructuredBuffer();
	ComPtr<ID3D12Resource> StructuredBuffer;

	virtual bool SetAsSRV(D3D12_GPU_VIRTUAL_ADDRESS& address, bsize offset);
	virtual bool SetAsUAV(D3D12_GPU_VIRTUAL_ADDRESS& address, bsize offset);
	virtual void* QueryInterface(int Type);
	virtual void LockUAV(ID3D12GraphicsCommandListX* command_list);
	virtual void UnlockUAV(ID3D12GraphicsCommandListX* command_list);
private:
	D3D12_RESOURCE_STATES m_CurrentStates;
	bsize m_Size;
};

