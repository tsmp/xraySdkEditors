#pragma once
class DX12VertexBuffer :public BearRHI::BearRHIVertexBuffer
{
public:
	DX12VertexBuffer();
	virtual void Create(bsize stride,bsize count, bool dynamic, void* data);
	virtual ~DX12VertexBuffer();
	virtual void* Lock();
	virtual void Unlock();
	virtual void Clear();
	virtual bsize GetCount();
	ComPtr<ID3D12Resource> VertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView;
private:
	bool m_Dynamic;

};