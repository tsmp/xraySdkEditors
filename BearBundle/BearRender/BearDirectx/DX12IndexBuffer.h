#pragma once
class DX12IndexBuffer :public BearRHI::BearRHIIndexBuffer
{
public:
	DX12IndexBuffer();
	virtual void Create(bsize count, bool dynamic,void*data);
	virtual ~DX12IndexBuffer();
	virtual uint32* Lock();
	virtual void Unlock();
	virtual void Clear();
	virtual bsize GetCount();
	ComPtr<ID3D12Resource> IndexBuffer;
	D3D12_INDEX_BUFFER_VIEW IndexBufferView;
private:
	bool m_Dynamic;

};