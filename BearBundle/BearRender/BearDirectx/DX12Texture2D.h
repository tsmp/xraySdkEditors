#pragma once
class DX12Texture2D:public DX12UnorderedAccess,public BearRHI::BearRHITexture2D
{
public:
	DX12Texture2D(bsize width, bsize height, bsize mips, bsize count, BearTexturePixelFormat pixel_format, BearTextureUsage type_usage, void* data = 0);
	DX12Texture2D(bsize width, bsize height,BearRenderTargetFormat format);
	DX12Texture2D(bsize width, bsize height, BearDepthStencilFormat format);
	ComPtr<ID3D12Resource> TextureBuffer;
	virtual bool SetAsSRV(D3D12_CPU_DESCRIPTOR_HANDLE& heap);
	virtual bool SetAsUAV(D3D12_CPU_DESCRIPTOR_HANDLE& heap, bsize offset);
	virtual void* QueryInterface(int type);
	virtual ~DX12Texture2D();
public:
	virtual void* Lock(bsize mip, bsize depth);
	virtual void Unlock();
	virtual BearTextureType GetType();
private:
	uint8* m_LockBuffer;
	bsize m_LockMip;
	bsize m_LockDepth;
public:

	BearRenderTargetFormat RTVFormat;
	BearDepthStencilFormat DSVFormat;
	D3D12_RESOURCE_DESC TextureDesc;
private:
	BearTextureType  m_TextureType;
	BearTextureUsage m_TextureUsage;
	void AllocBuffer();
	void FreeBuffer();

	ComPtr<ID3D12Resource> m_Buffer;
	BearTexturePixelFormat m_Format;
private:
	D3D12_RESOURCE_STATES m_CurrentStates;
	virtual void LockUAV(ID3D12GraphicsCommandListX* command_list);
	virtual void UnlockUAV(ID3D12GraphicsCommandListX* command_list);
	DX12AllocatorHeapItem m_ShaderResource;
};