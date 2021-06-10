#pragma once
class DX12UniformBuffer :public BearRHI::BearRHIUniformBuffer
{
public:
	DX12UniformBuffer(bsize stride, bsize count, bool dynamic);
	virtual ~DX12UniformBuffer();
	virtual void* Lock();
	virtual void Unlock();
	virtual bsize GetCount()
	{
		return m_Count;
	}
	virtual bsize GetStride()
	{
		return m_Stride;
	}
	ComPtr<ID3D12Resource> UniformBuffer;
	//ComPtr<ID3D12DescriptorHeap> Heap;
private:
	bool m_Dynamic;
	bsize m_Count;
	bsize m_Stride;
};