#pragma once
class DX12TextureCube:public DX12ShaderResource,public BearRHI::BearRHITextureCube
{
public:
	DX12TextureCube(bsize width, bsize height, bsize mips, bsize count, BearTexturePixelFormat pixel_format, BearTextureUsage type_usage, void* data = 0);
	virtual bool SetAsSRV(D3D12_CPU_DESCRIPTOR_HANDLE& heap);
	virtual void* QueryInterface(int type);
	virtual ~DX12TextureCube();
	ComPtr<ID3D12Resource> TextureBuffer;
public:
	virtual void* Lock(bsize mip, bsize depth);
	virtual void Unlock();
private:
	uint8*m_LockBuffer;
	bsize m_LockMip;
	bsize m_LockDepth;
public:
	D3D12_RESOURCE_DESC TextureDesc;
private:
	BearTextureUsage m_TextureUsage;
	ComPtr<ID3D12Resource> m_Buffer;
	BearTexturePixelFormat m_Format;
	void AllocBuffer();
	void FreeBuffer();
	DX12AllocatorHeapItem m_ShaderResource;
};