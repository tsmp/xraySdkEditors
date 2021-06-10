#include "VKPCH.h"
size_t Texture2DCounter = 0;
VKTexture2D::VKTexture2D(size_t width, size_t height, size_t mips, size_t count, BearTexturePixelFormat pixel_format, BearTextureUsage type_usage, void* data)
{
    Texture2DCounter++;
    m_Format = pixel_format;
    m_TextureUsage = type_usage;
    m_TextureType = BearTextureType::Default;

    switch (m_TextureUsage)
    {
    case BearTextureUsage::Static:
    case BearTextureUsage::Dynamic:
    case BearTextureUsage::Stating:
        ImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        break;
    case BearTextureUsage::Storage:
        ImageLayout = VK_IMAGE_LAYOUT_GENERAL;
        break;
    default:
        break;
    }
    bAllowUAV = m_TextureUsage == BearTextureUsage::Storage;
    {
        bear_fill(ImageCreateInfo);
        ImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        ImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        ImageCreateInfo.extent.width =static_cast<uint32_t>( width);
        ImageCreateInfo.extent.height = static_cast<uint32_t>(height);
        ImageCreateInfo.arrayLayers = static_cast<uint32_t>(count);;
        ImageCreateInfo.mipLevels = static_cast<uint32_t>(mips);
        ImageCreateInfo.extent.depth = 1;
        ImageCreateInfo.format = VKFactory::Translation(pixel_format);
        ImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        ImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        switch (m_TextureUsage)
        {
        case BearTextureUsage::Stating:
            ImageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            break;
        case BearTextureUsage::Storage:
            ImageCreateInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            break;
        default:
            ImageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            break;
        }

        
        ImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        ImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        V_CHK(vkCreateImage(Factory->Device, &ImageCreateInfo, nullptr, &Image));
        VkMemoryRequirements MemoryRequirement;
        vkGetImageMemoryRequirements(Factory->Device, Image, &MemoryRequirement);

        VkMemoryAllocateInfo MemoryAllocateInfo = {};
        MemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        MemoryAllocateInfo.allocationSize = MemoryRequirement.size;
        MemoryAllocateInfo.memoryTypeIndex = FindMemoryType(Factory->PhysicalDevice,MemoryRequirement.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        V_CHK(vkAllocateMemory(Factory->Device, &MemoryAllocateInfo, nullptr, &ImageMemory));
        vkBindImageMemory(Factory->Device, Image, ImageMemory, 0);
    }

  
   

    {
        VkImageViewCreateInfo ViewCreateInfo = {};
        ViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ViewCreateInfo.image = Image;
        ViewCreateInfo.viewType = count>1? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
        ViewCreateInfo.format = VKFactory::Translation(pixel_format);
        ViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        ViewCreateInfo.subresourceRange.baseMipLevel = 0;
        ViewCreateInfo.subresourceRange.levelCount = static_cast<uint32_t>(mips);
        ViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        ViewCreateInfo.subresourceRange.layerCount = static_cast<uint32_t>(count);

        V_CHK(vkCreateImageView(Factory->Device, &ViewCreateInfo, nullptr, &SRVImageView));
        if (bAllowUAV)
        {
            UAVImageViews.resize(mips);
            ViewCreateInfo.subresourceRange.levelCount = 1;
            for (size_t i = 0; i < mips; i++)
            {
                ViewCreateInfo.subresourceRange.baseMipLevel =static_cast<uint32_t>( i);
                V_CHK(vkCreateImageView(Factory->Device, &ViewCreateInfo, nullptr, &UAVImageViews[i]));
            }
        }
    }
    TransitionImageLayout(0, Image, ImageCreateInfo.format, VK_IMAGE_LAYOUT_UNDEFINED, ImageLayout, ImageCreateInfo.mipLevels,  ImageCreateInfo.arrayLayers,0);
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
        BEAR_CHECK(!bAllowUAV);
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

VKTexture2D::VKTexture2D(size_t width, size_t height, BearRenderTargetFormat format)
{
    Texture2DCounter++;
    m_RTVFormat = format;
    m_Buffer = 0;
    m_TextureUsage = BearTextureUsage::Static;
    m_TextureType = BearTextureType::RenderTarget;
    ZeroMemory(&ImageCreateInfo,sizeof(VkImageCreateInfo));
    ImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    ImageCreateInfo.extent.width = static_cast<uint32_t>(width);
    ImageCreateInfo.extent.height = static_cast<uint32_t>(height);
    ImageCreateInfo.extent.depth = static_cast<uint32_t>(1);;
    ImageCreateInfo.mipLevels = static_cast<uint32_t>(1);
    ImageCreateInfo.arrayLayers = 1;
    ImageCreateInfo.format = VKFactory::Translation(m_RTVFormat);
    ImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    ImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ImageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT| VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    ImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    ImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    V_CHK(vkCreateImage(Factory->Device, &ImageCreateInfo, nullptr, &Image));
    VkMemoryRequirements MemoryRequirement;
    vkGetImageMemoryRequirements(Factory->Device, Image, &MemoryRequirement);

    VkMemoryAllocateInfo MemoryAllocateInfo = {};
    MemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    MemoryAllocateInfo.allocationSize = MemoryRequirement.size;
    MemoryAllocateInfo.memoryTypeIndex = FindMemoryType(Factory->PhysicalDevice, MemoryRequirement.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    V_CHK(vkAllocateMemory(Factory->Device, &MemoryAllocateInfo, nullptr, &ImageMemory));
    vkBindImageMemory(Factory->Device, Image, ImageMemory, 0);
    {
        VkImageViewCreateInfo ViewCreateInfo = {};
        ViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ViewCreateInfo.image = Image;
        ViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D ;
        ViewCreateInfo.format = VKFactory::Translation(m_RTVFormat);
        ViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        ViewCreateInfo.subresourceRange.baseMipLevel = 0;
        ViewCreateInfo.subresourceRange.levelCount = static_cast<uint32_t>(1);
        ViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        ViewCreateInfo.subresourceRange.layerCount = static_cast<uint32_t>(1);

        V_CHK(vkCreateImageView(Factory->Device, &ViewCreateInfo, nullptr, &SRVImageView));
    }
    TransitionImageLayout(0, Image, ImageCreateInfo.format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, ImageCreateInfo.mipLevels,  ImageCreateInfo.arrayLayers,0);
}

VKTexture2D::VKTexture2D(size_t width, size_t height, BearDepthStencilFormat format)
{
    Texture2DCounter++;
    m_DSVFormat = format;
    m_Buffer = 0;
    m_TextureUsage = BearTextureUsage::Static;
    m_TextureType = BearTextureType::DepthStencil;

    ZeroMemory(&ImageCreateInfo, sizeof(VkImageCreateInfo));
    ImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    ImageCreateInfo.extent.width = static_cast<uint32_t>(width);
    ImageCreateInfo.extent.height = static_cast<uint32_t>(height);
    ImageCreateInfo.extent.depth = static_cast<uint32_t>(1);;
    ImageCreateInfo.mipLevels = static_cast<uint32_t>(1);
    ImageCreateInfo.arrayLayers = 1;
    ImageCreateInfo.format = VKFactory::Translation(m_DSVFormat);
    ImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    ImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    ImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    ImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    V_CHK(vkCreateImage(Factory->Device, &ImageCreateInfo, nullptr, &Image));
    VkMemoryRequirements MemoryRequirement;
    vkGetImageMemoryRequirements(Factory->Device, Image, &MemoryRequirement);

    VkMemoryAllocateInfo MemoryAllocateInfo = {};
    MemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    MemoryAllocateInfo.allocationSize = MemoryRequirement.size;
    MemoryAllocateInfo.memoryTypeIndex = FindMemoryType(Factory->PhysicalDevice, MemoryRequirement.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    V_CHK(vkAllocateMemory(Factory->Device, &MemoryAllocateInfo, nullptr, &ImageMemory));
    vkBindImageMemory(Factory->Device, Image, ImageMemory, 0);
    SRVImageView = 0;
    {
        VkImageViewCreateInfo ViewCreateInfo = {};
        ViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ViewCreateInfo.image = Image;
        ViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ViewCreateInfo.format = VKFactory::Translation(m_DSVFormat);
        ViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
        ViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
        ViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
        ViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
        switch (m_DSVFormat)
        {
        case BearDepthStencilFormat::Depth16:
        case BearDepthStencilFormat::Depth32F:
            ViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT ;
            break;
        default:
            ViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            break;
        }


        ViewCreateInfo.subresourceRange.baseMipLevel = 0;
        ViewCreateInfo.subresourceRange.levelCount = static_cast<uint32_t>(1);
        ViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        ViewCreateInfo.subresourceRange.layerCount = static_cast<uint32_t>(1);

        V_CHK(vkCreateImageView(Factory->Device, &ViewCreateInfo, nullptr, &SRVImageView));
    }

    TransitionImageLayout(0, Image, ImageCreateInfo.format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, ImageCreateInfo.mipLevels, ImageCreateInfo.arrayLayers, 0);
}

void VKTexture2D::SetAsSRV(VkWriteDescriptorSet* heap, size_t offset)
{
    BEAR_CHECK(SRVImageView);
    DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    DescriptorImageInfo.imageView = SRVImageView;
    DescriptorImageInfo.sampler = Factory->DefaultSampler;
    heap->descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    heap->pImageInfo = &DescriptorImageInfo;
}

void VKTexture2D::SetAsUAV(VkWriteDescriptorSet* heap, size_t offset)
{
    BEAR_CHECK(UAVImageViews[offset]);
    BEAR_CHECK(bAllowUAV);
    DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    DescriptorImageInfo.imageView = UAVImageViews[offset];
    DescriptorImageInfo.sampler = Factory->DefaultSampler;
    heap->descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    heap->pImageInfo = &DescriptorImageInfo;
}

void VKTexture2D::LockUAV(VkCommandBuffer command_line)
{
    BEAR_CHECK(bAllowUAV);
    TransitionImageLayout(command_line, Image, ImageCreateInfo.format, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, ImageCreateInfo.mipLevels, ImageCreateInfo.arrayLayers, 0);

}

void VKTexture2D::UnlockUAV(VkCommandBuffer command_line)
{
    BEAR_CHECK(bAllowUAV);
    TransitionImageLayout(command_line, Image, ImageCreateInfo.format, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, ImageCreateInfo.mipLevels, ImageCreateInfo.arrayLayers, 0);
}

void* VKTexture2D::QueryInterface(int type)
{
    switch (type)
    {
    case VKQ_ShaderResource:
        return reinterpret_cast<void*>(static_cast<VKShaderResource*>(this));
    case VKQ_UnorderedAccess:
        return reinterpret_cast<void*>(static_cast<VKUnorderedAccess*>(this));
    default:
        return nullptr;
    }
}

VKTexture2D::~VKTexture2D()
{
    Texture2DCounter--;
    if (SRVImageView)
    {
        vkDestroyImageView(Factory->Device, SRVImageView, nullptr);
    }
    for (auto& i : UAVImageViews)
    {
        vkDestroyImageView(Factory->Device, i, nullptr);
    }
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


BearTextureType VKTexture2D::GetType()
{
    return m_TextureType;
}

void* VKTexture2D::Lock(size_t mip, size_t depth)
{
    BEAR_CHECK(!bAllowUAV);
    m_Mip = mip;
    m_Depth = depth;
    if (bAllowUAV)return 0;
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
        TransitionImageLayout(Factory->CommandBuffer, Image, VKFactory::Translation(m_Format), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, ImageCreateInfo.mipLevels,  ImageCreateInfo.arrayLayers, static_cast<uint32_t>(m_Depth));
        CopyImageToBuffer(Factory->CommandBuffer, m_StagingBuffer, Image, ImageCreateInfo.extent.width, ImageCreateInfo.extent.height, static_cast<uint32_t>(m_Mip), static_cast<uint32_t>(m_Depth));
        TransitionImageLayout(Factory->CommandBuffer, Image, VKFactory::Translation(m_Format), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, ImageCreateInfo.mipLevels,  ImageCreateInfo.arrayLayers, static_cast<uint32_t>(m_Depth));
        Factory->UnlockCommandBuffer();
        break;
    default:
        break;
    }
    vkMapMemory(Factory->Device, m_StagingBufferMemory, 0, BearTextureUtils::GetSizeInMemory(BearTextureUtils::GetMip( ImageCreateInfo.extent.width,mip), BearTextureUtils::GetMip(ImageCreateInfo.extent.height, mip), ImageCreateInfo.mipLevels, m_Format), 0, &m_Buffer);

    return m_Buffer;
}

void VKTexture2D::Unlock()
{
    BEAR_CHECK(m_Buffer);
    BEAR_CHECK(!bAllowUAV);
    if (m_TextureType != BearTextureType::Default)return;
    vkUnmapMemory(Factory->Device, m_StagingBufferMemory);
    switch (m_TextureUsage)
    {
    case BearTextureUsage::Static:
        Factory->LockCommandBuffer();
        TransitionImageLayout(Factory->CommandBuffer, Image, VKFactory::Translation(m_Format), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, ImageCreateInfo.mipLevels,  ImageCreateInfo.arrayLayers, static_cast<uint32_t>(m_Depth));
        CopyBufferToImage(Factory->CommandBuffer, m_StagingBuffer, Image, ImageCreateInfo.extent.width, ImageCreateInfo.extent.height, static_cast<uint32_t>(m_Mip), static_cast<uint32_t>(m_Depth));
        TransitionImageLayout(Factory->CommandBuffer, Image, VKFactory::Translation(m_Format), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, ImageCreateInfo.mipLevels,  ImageCreateInfo.arrayLayers, static_cast<uint32_t>(m_Depth));
        Factory->UnlockCommandBuffer();
        FreeBuffer();
        break;
    case BearTextureUsage::Dynamic:
        Factory->LockCommandBuffer();
        TransitionImageLayout(Factory->CommandBuffer, Image, VKFactory::Translation(m_Format), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, ImageCreateInfo.mipLevels,  ImageCreateInfo.arrayLayers, static_cast<uint32_t>(m_Depth));
        CopyBufferToImage(Factory->CommandBuffer, m_StagingBuffer, Image, ImageCreateInfo.extent.width, ImageCreateInfo.extent.height, static_cast<uint32_t>(m_Mip), static_cast<uint32_t>(m_Depth));
        TransitionImageLayout(Factory->CommandBuffer, Image, VKFactory::Translation(m_Format), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, ImageCreateInfo.mipLevels,  ImageCreateInfo.arrayLayers, static_cast<uint32_t>(m_Depth));
        Factory->UnlockCommandBuffer();
        break;
    case BearTextureUsage::Stating:
        break;
    default:
        break;
    }
    m_Buffer = 0;
}

void VKTexture2D::AllocBuffer()
{
    VkDeviceSize ImageSize = BearTextureUtils::GetSizeInMemory(ImageCreateInfo.extent.width, ImageCreateInfo.extent.height, ImageCreateInfo.mipLevels, m_Format) *  ImageCreateInfo.arrayLayers;
    CreateBuffer(Factory->PhysicalDevice, Factory->Device, ImageSize, m_TextureUsage == BearTextureUsage::Stating ? VK_BUFFER_USAGE_TRANSFER_DST_BIT : VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_StagingBuffer, m_StagingBufferMemory);
}

void VKTexture2D::FreeBuffer()
{
    vkDestroyBuffer(Factory->Device, m_StagingBuffer, nullptr);
    vkFreeMemory(Factory->Device, m_StagingBufferMemory, nullptr);
}

