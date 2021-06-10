#pragma once
class VKPipeline :public virtual BearRHI::BearRHIPipeline
{
public:
	virtual void Set(VkCommandBuffer command_buffer) = 0;
};
