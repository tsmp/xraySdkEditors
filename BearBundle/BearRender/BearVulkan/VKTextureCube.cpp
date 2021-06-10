#include "VKPCH.h"
size_t TextureCubeCounter = 0;
VKTextureCube::VKTextureCube(size_t width, size_t height, size_t mips, size_t count, BearTexturePixelFormat pixel_format, BearTextureUsage type_usage, void* data)
{
    TextureCubeCounter++;
    m_Format = pixel_format;
    m_TextureUsage = type_usage;
    m_TextureType = BearTextureType::Default;
    {
        bear_fill(ImageCreateInfo);
        ImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        ImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        ImageCreateInfo.extent.width =static_cast<uint32_t>( width);
        ImageCreateInfo.extent.height = static_cast<uint32_t>(height);
        ImageCreateInfo.extent.depth = 1;;
        ImageCreateInfo.mipLevels = static_cast<uint32_t>(mips);
        ImageCreateInfo.arrayLayers = static_cast<uint32_t>(count)*6;
        ImageCreateInfo.format = VKFactory::Translation(pixel_format);
        ImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        ImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        ImageCreateInfo.usage =m_TextureUsage==BearTextureUsage::Stating ? VK_IMAGE_USAGE_TRANSFER_SRC_BIT| VK_IMAGE_USAGE_TRANSFER_DST_BIT : VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        ImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        ImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        ImageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        V_CHK(vkCreateImage(Factory->Device, &ImageCreateInfo, nullptr, &Image));
        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(Factory->Device, Image, &memRequirements);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(Factory->PhysicalDevice,memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        V_CHK(vkAllocateMemory(Factory->Device, &allocInfo, nullptr, &ImageMemory));
        vkBindImageMemory(Factory->Device, Image, ImageMemory, 0);
    }

  
   

    {
        VkImageViewCreateInfo ImageViewCreateInfo = {};
        ImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ImageViewCreateInfo.image = Image;
        ImageViewCreateInfo.viewType = count>1? VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : VK_IMAGE_VIEW_TYPE_CUBE;
        ImageViewCreateInfo.format = VKFactory::Translation(pixel_format);
        ImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        ImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        ImageViewCreateInfo.subresourceRange.levelCount = static_cast<uint32_t>(mips);
        ImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        ImageViewCreateInfo.subresourceRange.layerCount = static_cast<uint32_t>(count);

        V_CHK(vkCreateImageView(Factory->Device, &ImageViewCreateInfo, nullptr, &ImageView));
    }
    TransitionImageLayout(0, Image, ImageCreateInfo.format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, ImageCreateInfo.mipLevels,  ImageCreateInfo.arrayLayers,0);
    m_Buffer = 0;
    switch (m_TextureUsage)
    {
    case BearTextureUsage::Dynamic:
    case BearTextureUsage::Stating:
        AllocBuffer();
        break;
    }
    if (data)
    {
        auto ptr = reinterpret_cast<uint8*>(data);
        for (size_t x = 0; x < count; x++)
            for (size_t y = 0; y < mips; y++)
            {
                size_t  size = BearTextureUtils::GetSizeDepth(BearTextureUtils::GetMip(width, y), BearTextureUtils::GetMip(height, y), m_Format);
                memcpy(Lock(y, x), ptr, size);
                Unlock();
                ptr += size;
            }


    }
}

void VKTextureCube::SetAsSRV(VkWriteDescriptorSet* heap, size_t offset)
{
    BEAR_CHECK(ImageView);
    DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    DescriptorImageInfo.imageView = ImageView;
    DescriptorImageInfo.sampler = Factory->DefaultSampler;
    heap->descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    heap->pImageInfo = &DescriptorImageInfo;
}

void* VKTextureCube::QueryInterface(int Type)
{
    switch (Type)
    {
    case VKQ_ShaderResource:
        return reinterpret_cast<void*>(static_cast<VKShaderResource*>(this));
    default:
        return nullptr;
    }
}

VKTextureCube::~VKTextureCube()
{
    TextureCubeCounter--;
    if(ImageView)
    vkDestroyImageView(Factory->Device, ImageView, nullptr);
    vkDestroyImage(Factory->Device, Image, nullptr);
    vkFreeMemory(Factory->Device, ImageMemory, nullptr);
    switch (m_TextureUsage)
    {
    case BearTextureUsage::Dynamic:
    case BearTextureUsage::Stating:
        FreeBuffer();
        break;
    }
}


BearTextureType VKTextureCube::GetType()
{
    return m_TextureType;
}

void* VKTextureCube::Lock(size_t mip, size_t depth)
{
    m_Mip = mip;
    m_Depth = depth;
    if (m_TextureType != BearTextureType::Default)return 0;
    if (m_Buffer)Unlock();
    switch (m_TextureUsage)
    {
    case BearTextureUsage::Static:
        AllocBuffer();
        break;
    case BearTextureUsage::Dynamic:
        break;
    case BearTextureUsage::Stating:
        Factory->LockCommandBuffer();
        TransitionImageLayout(Factory->CommandBuffer, Image, VKFactory::Translation(m_Format), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, ImageCreateInfo.mipLevels, 1, static_cast<uint32_t>(m_Depth));
        CopyImageToBuffer(Factory->CommandBuffer, m_StagingBuffer, Image, ImageCreateInfo.extent.width, ImageCreateInfo.extent.height, static_cast<uint32_t>(m_Mip), static_cast<uint32_t>(m_Depth));
        TransitionImageLayout(Factory->CommandBuffer, Image, VKFactory::Translation(m_Format), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, ImageCreateInfo.mipLevels, 1, static_cast<uint32_t>(m_Depth));
        Factory->UnlockCommandBuffer();
        break;
    default:
        break;
    }
    vkMapMemory(Factory->Device, m_StagingBufferMemory, 0, BearTextureUtils::GetSizeInMemory(BearTextureUtils::GetMip( ImageCreateInfo.extent.width,mip), BearTextureUtils::GetMip(ImageCreateInfo.extent.height, mip), ImageCreateInfo.mipLevels, m_Format), 0, &m_Buffer);

    return m_Buffer;
}

void VKTextureCube::Unlock()
{
    vkUnmapMemory(Factory->Device, m_StagingBufferMemory);
    switch (m_TextureUsage)
    {
    case BearTextureUsage::Static:
        Factory->LockCommandBuffer();
        TransitionImageLayout(Factory->CommandBuffer, Image, VKFactory::Translation(m_Format), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, ImageCreateInfo.mipLevels,1, static_cast<uint32_t>(m_Depth));
        CopyBufferToImage(Factory->CommandBuffer, m_StagingBuffer, Image, ImageCreateInfo.extent.width, ImageCreateInfo.extent.height, static_cast<uint32_t>(m_Mip), static_cast<uint32_t>(m_Depth));
        TransitionImageLayout(Factory->CommandBuffer, Image, VKFactory::Translation(m_Format), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, ImageCreateInfo.mipLevels, 1, static_cast<uint32_t>(m_Depth));
        Factory->UnlockCommandBuffer();
        FreeBuffer();
        break;
    case BearTextureUsage::Dynamic:
        Factory->LockCommandBuffer();
        TransitionImageLayout(Factory->CommandBuffer, Image, VKFactory::Translation(m_Format), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, ImageCreateInfo.mipLevels, 1, static_cast<uint32_t>(m_Depth));
        CopyBufferToImage(Factory->CommandBuffer, m_StagingBuffer, Image, ImageCreateInfo.extent.width, ImageCreateInfo.extent.height, static_cast<uint32_t>(m_Mip), static_cast<uint32_t>(m_Depth));
        TransitionImageLayout(Factory->CommandBuffer, Image, VKFactory::Translation(m_Format), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, ImageCreateInfo.mipLevels,1, static_cast<uint32_t>(m_Depth));
        Factory->UnlockCommandBuffer();
        break;
    case BearTextureUsage::Stating:
        break;
    default:
        break;
    }
    m_Buffer = 0;
  
    
   

 
}

void VKTextureCube::AllocBuffer()
{
    VkDeviceSize ImageSize = BearTextureUtils::GetSizeInMemory(ImageCreateInfo.extent.width, ImageCreateInfo.extent.height, ImageCreateInfo.mipLevels,m_Format) *  ImageCreateInfo.arrayLayers;
    CreateBuffer(Factory->PhysicalDevice, Factory->Device, ImageSize, m_TextureUsage == BearTextureUsage::Stating ? VK_BUFFER_USAGE_TRANSFER_DST_BIT : VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_StagingBuffer, m_StagingBufferMemory);
}

void VKTextureCube::FreeBuffer()
{
    vkDestroyBuffer(Factory->Device, m_StagingBuffer, nullptr);
    vkFreeMemory(Factory->Device, m_StagingBufferMemory, nullptr);
}

