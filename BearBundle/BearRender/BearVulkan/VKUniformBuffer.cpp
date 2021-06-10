#include "VKPCH.h"
size_t UniformBufferCounter = 0;

VKUniformBuffer::VKUniformBuffer(size_t stride, size_t count, bool dynamic)
{
	m_Dynamic = dynamic;

	Stride = (static_cast<size_t>((stride + Factory->PhysicalDeviceProperties.limits.minUniformBufferOffsetAlignment - 1) & ~(Factory->PhysicalDeviceProperties.limits.minUniformBufferOffsetAlignment - 1)));
	Count = count;
	if (dynamic)
		CreateBuffer(Factory->PhysicalDevice, Factory->Device, Stride* Count, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, Buffer, m_Memory);
	else
		CreateBuffer(Factory->PhysicalDevice, Factory->Device, Stride* Count, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, Buffer, m_Memory);
	BEAR_CHECK(m_Memory != 0);

}



VKUniformBuffer::~VKUniformBuffer()
{
	if (Buffer)vkDestroyBuffer(Factory->Device, Buffer, 0);
	if (m_Memory)vkFreeMemory(Factory->Device, m_Memory, 0);
	UniformBufferCounter--;
}

void* VKUniformBuffer::Lock()
{
	BEAR_CHECK(m_Memory != 0);
	if (m_Memory == 0)return 0;
	BEAR_CHECK(m_Dynamic);
	uint8_t* pData;
	V_CHK(vkMapMemory(Factory->Device, m_Memory, 0, Stride*Count, 0, (void**)&pData));
	BEAR_CHECK(pData);
	return (uint32*)pData;
}

void VKUniformBuffer::Unlock()
{
	if (m_Memory == 0)return;
	vkUnmapMemory(Factory->Device, m_Memory);
}

size_t VKUniformBuffer::GetStride()
{
	return Stride;
}

size_t VKUniformBuffer::GetCount()
{
	return Count;
}

