#pragma once
class VKTextureCube : public VKShaderResource, public BearRHI::BearRHITextureCube
{
public:
	VKTextureCube(size_t width, size_t height, size_t mips, size_t vount, BearTexturePixelFormat pixel_format, BearTextureUsage type_usage , void* data = 0);

	VkImage Image;
	VkDeviceMemory ImageMemory;
	VkImageView ImageView;
	VkDescriptorImageInfo DescriptorImageInfo;
	VkImageCreateInfo ImageCreateInfo;
	virtual ~VKTextureCube();
public:
	virtual void SetAsSRV(VkWriteDescriptorSet* heap, size_t offset);
	virtual void* QueryInterface(int type);
	virtual BearTextureType GetType();
	virtual void* Lock(size_t mip, size_t depth);
	virtual void Unlock();
private:
	void* m_Buffer;
	size_t m_Mip;
	size_t m_Depth;
private:
	BearTextureType  m_TextureType;
	BearTextureUsage  m_TextureUsage;
	BearTexturePixelFormat  m_Format;
private:
	void AllocBuffer();
	void FreeBuffer();
	VkBuffer  m_StagingBuffer;
	VkDeviceMemory  m_StagingBufferMemory;
};