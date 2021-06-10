#include "DX12PCH.h"
bsize VertexBufferCounter = 0;
DX12VertexBuffer::DX12VertexBuffer() :m_Dynamic(false)
{
	VertexBufferView.SizeInBytes = 0;
	VertexBufferCounter++;
}

void DX12VertexBuffer::Create(bsize stride, bsize count, bool dynamic, void* data)
{

	Clear();
	m_Dynamic = dynamic;
	{
		auto Properties = CD3DX12_HEAP_PROPERTIES (dynamic ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT);
		auto ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(static_cast<uint64>(stride * count));
		R_CHK(Factory->Device->CreateCommittedResource(&Properties,D3D12_HEAP_FLAG_NONE,&ResourceDesc,dynamic ? D3D12_RESOURCE_STATE_GENERIC_READ : D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,nullptr,IID_PPV_ARGS(&VertexBuffer)));
	}

	VertexBufferView.SizeInBytes = static_cast<UINT>(stride * count);
	VertexBufferView.StrideInBytes = static_cast<UINT>(stride);
	VertexBufferView.BufferLocation = VertexBuffer->GetGPUVirtualAddress();

	if (data && !m_Dynamic)
	{
		ComPtr<ID3D12Resource> TempBuffer;
		auto Properties =  CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		auto ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(static_cast<uint64>(stride * count));
		R_CHK(Factory->Device->CreateCommittedResource(&Properties,D3D12_HEAP_FLAG_NONE,&ResourceDesc,D3D12_RESOURCE_STATE_GENERIC_READ,nullptr,IID_PPV_ARGS(&TempBuffer)));
		
		{
			void* Pointer;
			CD3DX12_RANGE ReadRange(0, 0);
			R_CHK(TempBuffer->Map(0, &ReadRange, reinterpret_cast<void**>(&Pointer)));
			bear_copy(Pointer, data, count * stride);
			TempBuffer->Unmap(0, nullptr);
		}

		Factory->LockCommandList();
		auto ResourceBarrier1 = CD3DX12_RESOURCE_BARRIER::Transition(VertexBuffer.Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
		Factory->CommandList->ResourceBarrier(1, &ResourceBarrier1);
		Factory->CommandList->CopyBufferRegion(VertexBuffer.Get(), 0, TempBuffer.Get(), 0, count * stride);
		auto ResourceBarrier2 = CD3DX12_RESOURCE_BARRIER::Transition(VertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		Factory->CommandList->ResourceBarrier(1, &ResourceBarrier2);
		Factory->UnlockCommandList();
	}
	else if (data)
	{
		bear_copy(Lock(), data, count * stride);
		Unlock();
	}
}

DX12VertexBuffer::~DX12VertexBuffer()
{
	VertexBufferCounter--;
	Clear();
}

void* DX12VertexBuffer::Lock()
{
	BEAR_CHECK(m_Dynamic);
	if (VertexBuffer.Get() == 0)return 0;
	void* Pointer;
	CD3DX12_RANGE ReadRange(0, 0);
	R_CHK(VertexBuffer->Map(0, &ReadRange, reinterpret_cast<void**>(&Pointer)));
	return Pointer;

}

void DX12VertexBuffer::Unlock()
{
	VertexBuffer->Unmap(0, nullptr);
}

void DX12VertexBuffer::Clear()
{
	VertexBufferView.SizeInBytes = 0;
	VertexBuffer.Reset();
	m_Dynamic = false;
}

bsize DX12VertexBuffer::GetCount()
{
	if (VertexBufferView.StrideInBytes == 0)return 0;
	return VertexBufferView.SizeInBytes / VertexBufferView.StrideInBytes;
}
