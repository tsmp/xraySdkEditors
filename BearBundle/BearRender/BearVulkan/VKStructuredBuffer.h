#pragma once
class VKStructuredBuffer : public VKUnorderedAccess, public BearRHI::BearRHIStructuredBuffer
{
public:
	VKStructuredBuffer(size_t size, void* data,bool uav);
	virtual ~VKStructuredBuffer();
	virtual void SetAsSRV(VkWriteDescriptorSet* heap, size_t offset);
	virtual void SetAsUAV(VkWriteDescriptorSet* heap, size_t offset);
	virtual void LockUAV(VkCommandBuffer  command_line);
	virtual void UnlockUAV(VkCommandBuffer  command_line);
	virtual void* QueryInterface(int type);
	VkBuffer Buffer;
	VkDescriptorBufferInfo BufferInfo;
	VkDeviceMemory Memory;
	size_t Size;
};

