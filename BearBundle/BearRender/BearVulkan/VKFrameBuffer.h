#pragma once 
class VKFrameBuffer :public BearRHI::BearRHIFrameBuffer
{
public:
	VKFrameBuffer(const BearFrameBufferDescription& description);
	virtual ~VKFrameBuffer();
	VkRenderPassBeginInfo GetRenderPass();
	void Unlock(VkCommandBuffer comand_list);
	void Lock(VkCommandBuffer comand_list);
	BearFrameBufferDescription Description;
	size_t CountRenderTarget;
	VkFramebuffer FrameBuffer;
	VKRenderPass *RenderPassRef;
	VkClearValue ClearValues[9];
	uint32_t Width;
	uint32_t Height;
};