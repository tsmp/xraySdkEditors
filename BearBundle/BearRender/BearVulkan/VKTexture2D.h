#pragma once
class VKTexture2D : public VKUnorderedAccess, public BearRHI::BearRHITexture2D
{
public:
	VKTexture2D(size_t width, size_t height, size_t mips, size_t count, BearTexturePixelFormat pixel_format, BearTextureUsage type_usage, void* data = 0);
	VKTexture2D(size_t width, size_t height, BearRenderTargetFormat format);
	VKTexture2D(size_t width, size_t height, BearDepthStencilFormat format);
	VkImage Image;
	VkDeviceMemory ImageMemory;
	VkImageView SRVImageView;
	BearVector<VkImageView> UAVImageViews;
	VkDescriptorImageInfo DescriptorImageInfo;
	VkImageCreateInfo ImageCreateInfo;
	VkImageLayout ImageLayout;
	virtual ~VKTexture2D();
public:
	virtual void SetAsSRV(VkWriteDescriptorSet* heap, size_t offset);
	virtual void SetAsUAV(VkWriteDescriptorSet* heap, size_t offset);
	virtual void LockUAV(VkCommandBuffer  command_line);
	virtual void UnlockUAV(VkCommandBuffer  command_line);
	virtual void*QueryInterface(int type);
	virtual BearTextureType GetType();
	virtual void* Lock(size_t mip, size_t depth);
	virtual void Unlock();
private:
	void* m_Buffer;
	size_t m_Mip;
	size_t m_Depth;
private:
	BearTextureType m_TextureType;
	BearTextureUsage m_TextureUsage;
	BearTexturePixelFormat m_Format;
	BearRenderTargetFormat m_RTVFormat;
	BearDepthStencilFormat m_DSVFormat;
private:
	void AllocBuffer();
	void FreeBuffer();
	VkBuffer m_StagingBuffer;
	VkDeviceMemory m_StagingBufferMemory;
};