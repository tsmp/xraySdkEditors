#pragma once
inline bool TestBuffer(uint32_t type_bits, VkFlags requirements_mask, uint32_t& type_index) {

	for (uint32_t i = 0; i < Factory->PhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if ((type_bits & 1) == 1)
		{
			if ((Factory->PhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask) {
                type_index = i;
				return true;
			}
		}
        type_bits >>= 1;
	}
	return false;
}

inline int FindMemoryType(VkPhysicalDevice physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties PhysicalDeviceMemoryProperties;
	vkGetPhysicalDeviceMemoryProperties(physical_device, &PhysicalDeviceMemoryProperties);

	for (uint32_t i = 0; i < PhysicalDeviceMemoryProperties.memoryTypeCount; i++) {
		if ((type_filter & (1 << i)) && (PhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	BEAR_CHECK(false);
	return 0;
}
inline void CreateBuffer(VkPhysicalDevice physical_device,VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& buffer_memory)
{
	VkBufferCreateInfo BufferCreateInfo = {};
    BufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    BufferCreateInfo.size = size;
    BufferCreateInfo.usage = usage;
    BufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	V_CHK (vkCreateBuffer(device, &BufferCreateInfo, nullptr, &buffer))

	VkMemoryRequirements MemoryRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &MemoryRequirements);

	VkMemoryAllocateInfo MemoryAllocateInfo = {};
    MemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    MemoryAllocateInfo.allocationSize = MemoryRequirements.size;
    MemoryAllocateInfo.memoryTypeIndex = FindMemoryType(physical_device, MemoryRequirements.memoryTypeBits, properties);

	V_CHK(vkAllocateMemory(device, &MemoryAllocateInfo, nullptr, &buffer_memory));

	V_CHK(vkBindBufferMemory(device, buffer, buffer_memory, 0));
}

inline void CopyBuffer(VkCommandBuffer command_list, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkBufferCopy BufferCopy = {};
    BufferCopy.size = size;
	vkCmdCopyBuffer(command_list, srcBuffer, dstBuffer, 1, &BufferCopy);
}

inline void TransitionImageLayout(VkCommandBuffer command_list, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mips, uint32_t count_depth,uint32_t depth)
{
    VkImageMemoryBarrier ImageMemoryBarrier = {};
    ImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    ImageMemoryBarrier.oldLayout = oldLayout;
    ImageMemoryBarrier.newLayout = newLayout;
    ImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    ImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    ImageMemoryBarrier.image = image;

    ImageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ImageMemoryBarrier.subresourceRange.baseMipLevel = 0;
    ImageMemoryBarrier.subresourceRange.levelCount = mips;
    ImageMemoryBarrier.subresourceRange.baseArrayLayer =depth;
    ImageMemoryBarrier.subresourceRange.layerCount = count_depth;

    VkPipelineStageFlags SourceStage;
    VkPipelineStageFlags DestinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
        ImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        ImageMemoryBarrier.dstAccessMask = 0;

        SourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        DestinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else   if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        ImageMemoryBarrier.srcAccessMask = 0;
        ImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        SourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        DestinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
        ImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        ImageMemoryBarrier.dstAccessMask = 0;

        SourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        DestinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else   if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        ImageMemoryBarrier.srcAccessMask = 0;
        ImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        SourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        DestinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        ImageMemoryBarrier.srcAccessMask = 0;
        ImageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        ImageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        SourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        DestinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        ImageMemoryBarrier.srcAccessMask = 0;
        ImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        SourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        DestinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        ImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        ImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        SourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        DestinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        ImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        ImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        SourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        DestinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        ImageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        ImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        SourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        DestinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        ImageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        ImageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        SourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        DestinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        ImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;;
        ImageMemoryBarrier.dstAccessMask = 0;
        SourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        DestinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
        ImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        ImageMemoryBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        SourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        DestinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }

    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_GENERAL) 
    {
        ImageMemoryBarrier.srcAccessMask = 0;
        ImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;

        SourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        DestinationStage = VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_GENERAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        ImageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
        ImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        SourceStage = VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
        DestinationStage = VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_GENERAL)
    {
        ImageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        ImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;

        SourceStage = VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
        DestinationStage = VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
    }
    else {
		BEAR_CHECK(false); return;
    }
    if(command_list ==nullptr)
	Factory->LockCommandBuffer();
    vkCmdPipelineBarrier(
        command_list == nullptr?Factory->CommandBuffer: command_list,
        SourceStage, DestinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &ImageMemoryBarrier
    );
    if (command_list == nullptr)
	Factory->UnlockCommandBuffer();
}

inline void CopyBufferToImage(VkCommandBuffer command_list, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t mip, uint32_t depth)
{

        VkBufferImageCopy BufferImageCopy = {};
        BufferImageCopy.bufferOffset = 0;
        BufferImageCopy.bufferRowLength = 0;
        BufferImageCopy.bufferImageHeight = 0;
        BufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        BufferImageCopy.imageSubresource.mipLevel = mip;
        BufferImageCopy.imageSubresource.baseArrayLayer = depth;
        BufferImageCopy.imageSubresource.layerCount = 1;
      
        BufferImageCopy.imageOffset = { 0, 0, 0};
        BufferImageCopy.imageExtent = {
            static_cast<uint32_t>( BearTextureUtils::GetMip( width,mip)),
            static_cast<uint32_t>(BearTextureUtils::GetMip(height,mip)),
            1,
        };
        if (command_list == nullptr)
        Factory->LockCommandBuffer();
        vkCmdCopyBufferToImage(command_list == nullptr ? Factory->CommandBuffer : command_list, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &BufferImageCopy);
        if (command_list == nullptr)
        Factory->UnlockCommandBuffer();
    
    
}
inline void CopyImageToBuffer(VkCommandBuffer command_list, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t mip, uint32_t depth)
{

    VkBufferImageCopy BufferImageCopy = {};
    BufferImageCopy.bufferOffset = 0;
    BufferImageCopy.bufferRowLength = 0;
    BufferImageCopy.bufferImageHeight = 0;
    BufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    BufferImageCopy.imageSubresource.mipLevel = mip;
    BufferImageCopy.imageSubresource.baseArrayLayer = depth;
    BufferImageCopy.imageSubresource.layerCount = 1;

    BufferImageCopy.imageOffset = { 0, 0,0};
    BufferImageCopy.imageExtent = {
  static_cast<uint32_t>(BearTextureUtils::GetMip(width,mip)),
            static_cast<uint32_t>(BearTextureUtils::GetMip(height,mip)),
        1,
    };
    if (command_list == nullptr)
    Factory->LockCommandBuffer();
    vkCmdCopyImageToBuffer(command_list == nullptr ? Factory->CommandBuffer : command_list, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, buffer, 1, &BufferImageCopy);
    if (command_list == nullptr)
    Factory->UnlockCommandBuffer();


}