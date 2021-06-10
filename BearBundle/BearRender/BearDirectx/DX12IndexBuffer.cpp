#include "DX12PCH.h"
bsize IndexBufferCounter = 0;
DX12IndexBuffer::DX12IndexBuffer() :m_Dynamic(false)
{
	IndexBufferCounter++;
	IndexBufferView.SizeInBytes = 0;
}

void DX12IndexBuffer::Create(bsize count, bool dynamic, void* data)
{
	Clear();
	m_Dynamic = dynamic;
	{
		auto Properties = CD3DX12_HEAP_PROPERTIES(dynamic ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT);
		auto ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(static_cast<uint64>(sizeof(uint32) * count));
		D3D12_RESOURCE_STATES ResourceState = dynamic ? D3D12_RESOURCE_STATE_GENERIC_READ : D3D12_RESOURCE_STATE_INDEX_BUFFER;
		R_CHK(Factory->Device->CreateCommittedResource(&Properties,D3D12_HEAP_FLAG_NONE,&ResourceDesc,ResourceState,nullptr,IID_PPV_ARGS(&IndexBuffer)));
	}

	IndexBufferView.SizeInBytes = static_cast<UINT>(sizeof(uint32) * count);
	IndexBufferView.Format = DXGI_FORMAT::DXGI_FORMAT_R32_UINT;
	IndexBufferView.BufferLocation = IndexBuffer->GetGPUVirtualAddress();
	if (data && !dynamic)
	{
		ComPtr<ID3D12Resource> Temp;
		auto Properties =  CD3DX12_HEAP_PROPERTIES (D3D12_HEAP_TYPE_UPLOAD);
		auto ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(static_cast<uint64>(sizeof(uint32) * count));
		R_CHK(Factory->Device->CreateCommittedResource(&Properties,D3D12_HEAP_FLAG_NONE,&ResourceDesc,D3D12_RESOURCE_STATE_GENERIC_READ,nullptr,IID_PPV_ARGS(&Temp)));
		
		{
			void* Pointer;
			CD3DX12_RANGE ReadRange(0, 0);
			R_CHK(Temp->Map(0, &ReadRange, reinterpret_cast<void**>(&Pointer)));
			memcpy(Pointer, data, count * sizeof(uint32));
			Temp->Unmap(0, nullptr);
		}

		Factory->LockCommandList();

		auto ResourceBarrier1 = CD3DX12_RESOURCE_BARRIER::Transition(IndexBuffer.Get(), D3D12_RESOURCE_STATE_INDEX_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
		Factory->CommandList->ResourceBarrier(1, &ResourceBarrier1);

		Factory->CommandList->CopyBufferRegion(IndexBuffer.Get(), 0, Temp.Get(), 0, count * sizeof(uint32));

		auto ResourceBarrier2 = CD3DX12_RESOURCE_BARRIER::Transition(IndexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
		Factory->CommandList->ResourceBarrier(1, &ResourceBarrier2);

		Factory->UnlockCommandList();
	}
	else if (data)
	{
		memcpy(Lock(), data, count * sizeof(uint32));
		Unlock();
	}
}

DX12IndexBuffer::~DX12IndexBuffer()
{
	IndexBufferCounter--;
	Clear();
}

uint32* DX12IndexBuffer::Lock()
{
	BEAR_CHECK(m_Dynamic);
	if (IndexBuffer.Get() == 0)return 0;
	uint32* Pointer;
	CD3DX12_RANGE ReadRange(0, 0);
	R_CHK(IndexBuffer->Map(0, &ReadRange, reinterpret_cast<void**>(&Pointer)));
	return Pointer;

}

void DX12IndexBuffer::Unlock()
{
	if (IndexBuffer.Get() == 0)return;
	IndexBuffer->Unmap(0, nullptr);
}

void DX12IndexBuffer::Clear()
{
	IndexBufferView.SizeInBytes = 0;
	IndexBuffer.Reset();
	m_Dynamic = false;
}

bsize DX12IndexBuffer::GetCount()
{
	return IndexBufferView.SizeInBytes / sizeof(uint32);
}
