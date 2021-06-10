#include "DX12PCH.h"
bsize StructuredBufferCounter = 0;
DX12StructuredBuffer::DX12StructuredBuffer(bsize size, void* data, bool uav)
{
	bAllowUAV = uav;
	StructuredBufferCounter++;
	{
		m_CurrentStates = uav ? D3D12_RESOURCE_STATE_UNORDERED_ACCESS:(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		auto Properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		auto ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(static_cast<uint64>(size), uav?D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS: D3D12_RESOURCE_FLAG_NONE);
		R_CHK(Factory->Device->CreateCommittedResource(	&Properties,D3D12_HEAP_FLAG_NONE,&ResourceDesc, m_CurrentStates,nullptr,IID_PPV_ARGS(&StructuredBuffer)));

	}
	if (data)
	{
		ComPtr<ID3D12Resource> TempBuffer;
		auto Properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		auto ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(static_cast<uint64>(size));
		R_CHK(Factory->Device->CreateCommittedResource(	&Properties,D3D12_HEAP_FLAG_NONE,&ResourceDesc,D3D12_RESOURCE_STATE_GENERIC_READ,nullptr,IID_PPV_ARGS(&TempBuffer)));
		{
			void* Pointer;
			CD3DX12_RANGE ReadRange(0, 0);
			R_CHK(TempBuffer->Map(0, &ReadRange, reinterpret_cast<void**>(&Pointer)));
			bear_copy(Pointer, Pointer, size);
			TempBuffer->Unmap(0, nullptr);
		}

		Factory->LockCommandList();
		auto ResourceBarrier1 = CD3DX12_RESOURCE_BARRIER::Transition(StructuredBuffer.Get(), m_CurrentStates, D3D12_RESOURCE_STATE_COPY_DEST);
		Factory->CommandList->ResourceBarrier(1, &ResourceBarrier1);
		Factory->CommandList->CopyBufferRegion(StructuredBuffer.Get(), 0, TempBuffer.Get(), 0, size);
		auto ResourceBarrier2 = CD3DX12_RESOURCE_BARRIER::Transition(StructuredBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, m_CurrentStates);
		Factory->CommandList->ResourceBarrier(1, &ResourceBarrier2);
		Factory->UnlockCommandList();
	}
}

DX12StructuredBuffer::~DX12StructuredBuffer()
{
	StructuredBufferCounter--;
}

bool DX12StructuredBuffer::SetAsSRV(D3D12_GPU_VIRTUAL_ADDRESS& address, bsize offset)
{
	address = StructuredBuffer->GetGPUVirtualAddress()+ offset;
	return true;
}

bool DX12StructuredBuffer::SetAsUAV(D3D12_GPU_VIRTUAL_ADDRESS& address, bsize offset)
{
	if (!bAllowUAV)return false;
	address = StructuredBuffer->GetGPUVirtualAddress() + offset;
	return true;
}

void* DX12StructuredBuffer::QueryInterface(int type)
{
	switch (type)
	{
	case DX12Q_ShaderResource:
		return reinterpret_cast<void*>(static_cast<DX12ShaderResource*>(this));
	case DX12Q_UnorderedAccess:
		if (bAllowUAV)return nullptr;
		return reinterpret_cast<void*>(static_cast<DX12UnorderedAccess*>(this));
	default:
		return nullptr;
	}
}



void DX12StructuredBuffer::LockUAV(ID3D12GraphicsCommandListX*command_list)
{
	BEAR_CHECK(bAllowUAV);
	auto ResourceBarrier1 = CD3DX12_RESOURCE_BARRIER::Transition(StructuredBuffer.Get(), m_CurrentStates, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	command_list->ResourceBarrier(1, &ResourceBarrier1);
}

void DX12StructuredBuffer::UnlockUAV(ID3D12GraphicsCommandListX* command_list)
{
	BEAR_CHECK(bAllowUAV);
	auto ResourceBarrier1 = CD3DX12_RESOURCE_BARRIER::Transition(StructuredBuffer.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, m_CurrentStates);
	command_list->ResourceBarrier(1, &ResourceBarrier1);
}