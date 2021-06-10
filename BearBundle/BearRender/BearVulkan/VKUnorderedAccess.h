#pragma once
class VKUnorderedAccess :public virtual  BearRHI::BearRHIUnorderedAccess, public VKShaderResource
{
public:
	VKUnorderedAccess() { bAllowUAV = false; }
	virtual void SetAsUAV(VkWriteDescriptorSet* heap, size_t offset) = 0;
	virtual void LockUAV(VkCommandBuffer  command_line) = 0;
	virtual void UnlockUAV(VkCommandBuffer  command_line) = 0;
protected:
	bool bAllowUAV;
};