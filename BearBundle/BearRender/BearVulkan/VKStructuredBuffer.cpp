#include "VKPCH.h"
size_t StructuredBufferCounter = 0;
VKStructuredBuffer::VKStructuredBuffer(size_t size, void* data, bool uac):Size(size)
{
    StructuredBufferCounter++;
    CreateBuffer(Factory->PhysicalDevice, Factory->Device, Size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, Buffer, Memory);
    if (data)
    {
        VkBuffer TempBuffer;
        VkDeviceMemory TempMemory;
        CreateBuffer(Factory->PhysicalDevice, Factory->Device, Size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, TempBuffer, TempMemory);

        uint8_t* Pointer;
        V_CHK(vkMapMemory(Factory->Device, TempMemory, 0, Size, 0, (void**)&Pointer));
        memcpy(Pointer, data, Size);
        vkUnmapMemory(Factory->Device, TempMemory);

        Factory->LockCommandBuffer();
        CopyBuffer(Factory->CommandBuffer, TempBuffer, Buffer, Size);
        Factory->UnlockCommandBuffer();
        vkDestroyBuffer(Factory->Device, TempBuffer, 0);
        vkFreeMemory(Factory->Device, TempMemory, 0);
    }
}

VKStructuredBuffer::~VKStructuredBuffer()
{
    StructuredBufferCounter--;
    if (Buffer)vkDestroyBuffer(Factory->Device, Buffer, 0);
    if (Memory)vkFreeMemory(Factory->Device, Memory, 0);
}

void VKStructuredBuffer::SetAsSRV(VkWriteDescriptorSet* heap, size_t offset)
{
    heap->descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    BufferInfo.buffer = Buffer;
    BufferInfo.offset = offset;
    BufferInfo.range = Size- offset;
    heap->pBufferInfo = &BufferInfo;
}

void* VKStructuredBuffer::QueryInterface(int Type)
{
    switch (Type)
    {
    case VKQ_ShaderResource:
        return reinterpret_cast<void*>(static_cast<VKShaderResource*>(this));
    case VKQ_UnorderedAccess:
        return reinterpret_cast<void*>(static_cast<VKShaderResource*>(this));
    default:
        return nullptr;
    }
}

void VKStructuredBuffer::SetAsUAV(VkWriteDescriptorSet* heap, size_t offset)
{
    heap->descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    BufferInfo.buffer = Buffer;
    BufferInfo.offset = offset;
    BufferInfo.range = Size - offset;
    heap->pBufferInfo = &BufferInfo;
}

void VKStructuredBuffer::LockUAV(VkCommandBuffer command_line)
{
}

void VKStructuredBuffer::UnlockUAV(VkCommandBuffer command_line)
{
}
