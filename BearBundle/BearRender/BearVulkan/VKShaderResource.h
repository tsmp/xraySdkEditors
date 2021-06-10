#pragma once
class VKShaderResource :public virtual BearRHI::BearRHIShaderResource
{
public:
	virtual void SetAsSRV(VkWriteDescriptorSet* heap,size_t offset) = 0;
};