#include "VKPCH.h"
size_t VertexBufferCounter = 0;
VKVertexBuffer::VKVertexBuffer() :m_Dynamic(false)
{
	VertexBufferCounter++;
	Buffer = 0;
	m_Memory = 0;
	Size = 0;
}

void VKVertexBuffer::Create(size_t stride, size_t count, bool dynamic,void*data)
{
	Clear();
	m_Dynamic = dynamic;

	if (dynamic)
		CreateBuffer(Factory->PhysicalDevice, Factory->Device, count * stride, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT| VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, Buffer, m_Memory);
	else
		CreateBuffer(Factory->PhysicalDevice, Factory->Device, count * stride, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, Buffer, m_Memory);
	VertexDescription.binding = 0;
	VertexDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	VertexDescription.stride = static_cast<uint32_t>(stride);
	Size = stride * count;
	if (data && !dynamic)
	{
		VkBuffer TempBuffer;
		VkDeviceMemory TempMemory;
		CreateBuffer(Factory->PhysicalDevice, Factory->Device, Size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT , VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, TempBuffer, TempMemory);

		uint8_t* pData;
		V_CHK(vkMapMemory(Factory->Device, TempMemory, 0, Size, 0, (void**)&pData));
		memcpy(pData, data, Size);
		vkUnmapMemory(Factory->Device, TempMemory);

		Factory->LockCommandBuffer();
		CopyBuffer(Factory->CommandBuffer, TempBuffer, Buffer, Size);
		Factory->UnlockCommandBuffer();
		vkDestroyBuffer(Factory->Device, TempBuffer, 0);
		vkFreeMemory(Factory->Device, TempMemory, 0);
	}
	else if (data)
	{
		memcpy(Lock(), data, count * sizeof(uint32));
		Unlock();
	}
	BEAR_CHECK(m_Memory != 0);
}

VKVertexBuffer::~VKVertexBuffer()
{
	VertexBufferCounter--;
	Clear();
}

void* VKVertexBuffer::Lock()
{
	if (m_Memory == 0)return 0;
	BEAR_CHECK(m_Dynamic);
	uint8_t* pData;
	V_CHK(vkMapMemory(Factory->Device, m_Memory, 0, Size, 0, (void**)&pData));
	return (uint32*)pData;
}

void VKVertexBuffer::Unlock()
{
	if (m_Memory == 0)return;
	vkUnmapMemory(Factory->Device, m_Memory);
}

void VKVertexBuffer::Clear()
{
	Size = 0;
	m_Dynamic = false;
	if (Buffer)vkDestroyBuffer(Factory->Device, Buffer, 0);
	Buffer = 0;
	if (m_Memory)vkFreeMemory(Factory->Device, m_Memory, 0);
	m_Memory = 0;
}

size_t VKVertexBuffer::GetCount()
{
	if (VertexDescription.stride == 0)return 0;
	return Size/ VertexDescription.stride;
}
