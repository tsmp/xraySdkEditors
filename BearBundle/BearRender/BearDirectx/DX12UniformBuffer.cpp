#include "DX12PCH.h"
bsize UniformBufferCounter = 0;
DX12UniformBuffer::DX12UniformBuffer(bsize stride, bsize count, bool dynamic) :m_Dynamic(false)
{
	UniformBufferCounter++; m_Count = 0; m_Stride = 0;
	m_Count = count;
	m_Stride = (static_cast<uint64>((stride + 256 - 1) & ~(256 - 1)));
	m_Dynamic = dynamic;
	{
		auto Properties = CD3DX12_HEAP_PROPERTIES (dynamic ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT);
		auto ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_Stride * m_Count);
		R_CHK(Factory->Device->CreateCommittedResource(	&Properties,D3D12_HEAP_FLAG_NONE,&ResourceDesc,dynamic ? D3D12_RESOURCE_STATE_GENERIC_READ : D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,nullptr,IID_PPV_ARGS(&UniformBuffer)));

	}
}


DX12UniformBuffer::~DX12UniformBuffer()
{
	UniformBufferCounter--;
	m_Count = 0; m_Stride = 0;
	UniformBuffer.Reset();
	m_Dynamic = false;
}

void* DX12UniformBuffer::Lock()
{
	BEAR_CHECK(m_Dynamic);
	if (UniformBuffer.Get() == 0)return 0;
	void* Pointer;
	CD3DX12_RANGE ReadRange(0, 0);
	R_CHK(UniformBuffer->Map(0, &ReadRange, reinterpret_cast<void**>(&Pointer)));
	return Pointer;

}

void DX12UniformBuffer::Unlock()
{
	UniformBuffer->Unmap(0, nullptr);
}


