#pragma once 
class VKRenderPass :public BearRHI::BearRHIRenderPass
{
public:
	VKRenderPass(const BearRenderPassDescription& description);
	virtual ~VKRenderPass();
	BearRenderPassDescription Description;
	size_t CountRenderTarget;
	VkRenderPass RenderPass;
};