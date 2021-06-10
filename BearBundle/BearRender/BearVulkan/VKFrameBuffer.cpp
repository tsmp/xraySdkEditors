#include "VKPCH.h"
size_t FrameBufferCounter = 0;
VKFrameBuffer::VKFrameBuffer(const BearFrameBufferDescription& description):Description(description)
{
	FrameBufferCounter++;
	BEAR_CHECK(!description.RenderPass.empty());
	RenderPassRef = static_cast<VKRenderPass*>(Description.RenderPass.get());
	 CountRenderTarget = 0;

	VkFramebufferCreateInfo FramebufferCreateInfo = {};
	VkImageView Attachments[9] = {};
	for (; CountRenderTarget < 8; CountRenderTarget++)
	{
		if (Description.RenderTargets[CountRenderTarget].empty())
		{
			break;
		}
		
		auto Texture = static_cast<VKTexture2D*>(Description.RenderTargets[CountRenderTarget].get());
		BEAR_CHECK(Texture->GetType() == BearTextureType::RenderTarget);
		FramebufferCreateInfo.width = BearMath::max(Texture->ImageCreateInfo.extent.width, FramebufferCreateInfo.width);
		FramebufferCreateInfo.height = BearMath::max(Texture->ImageCreateInfo.extent.height, FramebufferCreateInfo.height);
		Attachments[CountRenderTarget] = Texture->SRVImageView;
	
	}

	BEAR_CHECK(RenderPassRef->CountRenderTarget == CountRenderTarget);
	

	if (Description.DepthStencil.empty())
	{
		BEAR_CHECK(RenderPassRef->Description.DepthStencil.Format == BearDepthStencilFormat::None);
	}
	else
	{
		BEAR_CHECK(Description.DepthStencil.get()->GetType() == BearTextureType::DepthStencil);
		auto textures = static_cast<VKTexture2D*>(Description.DepthStencil.get());
		FramebufferCreateInfo.width = BearMath::max(textures->ImageCreateInfo.extent.width, FramebufferCreateInfo.width);
		FramebufferCreateInfo.height = BearMath::max(textures->ImageCreateInfo.extent.height, FramebufferCreateInfo.height);
		Attachments[CountRenderTarget] = textures->SRVImageView;
		
	}
	Width = FramebufferCreateInfo.width;
	Height = FramebufferCreateInfo.height;
	FramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	FramebufferCreateInfo.pNext = NULL;
	FramebufferCreateInfo.renderPass = RenderPassRef->RenderPass;
	FramebufferCreateInfo.attachmentCount = static_cast<uint32_t>(CountRenderTarget + (Description.DepthStencil.empty()?0:1));
	FramebufferCreateInfo.pAttachments = Attachments;
	FramebufferCreateInfo.layers = 1;
	V_CHK( vkCreateFramebuffer(Factory->Device, &FramebufferCreateInfo, 0, &FrameBuffer));
}

VKFrameBuffer::~VKFrameBuffer()
{
	vkDestroyFramebuffer(Factory->Device, FrameBuffer, 0);
	FrameBufferCounter--;
}
VkRenderPassBeginInfo VKFrameBuffer::GetRenderPass()
{
	VkRenderPassBeginInfo RenderPassBeginInfo = {};
	RenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	RenderPassBeginInfo.pNext = NULL;
	RenderPassBeginInfo.renderPass = RenderPassRef->RenderPass;
	RenderPassBeginInfo.framebuffer = FrameBuffer;
	RenderPassBeginInfo.renderArea.offset.x = 0;
	RenderPassBeginInfo.renderArea.offset.y = 0;
	RenderPassBeginInfo.renderArea.extent.width = static_cast<uint32_t>(Width);
	RenderPassBeginInfo.renderArea.extent.height = static_cast<uint32_t>(Height);
	RenderPassBeginInfo.clearValueCount = static_cast<uint32_t>(CountRenderTarget);
	for (size_t i = 0; i < CountRenderTarget; i++)
	{
		ClearValues[i].color.float32[0] = RenderPassRef->Description.RenderTargets[i].Color.R32F;
		ClearValues[i].color.float32[1] = RenderPassRef->Description.RenderTargets[i].Color.G32F;
		ClearValues[i].color.float32[2] = RenderPassRef->Description.RenderTargets[i].Color.B32F;
		ClearValues[i].color.float32[3] = RenderPassRef->Description.RenderTargets[i].Color.A32F;
	}
	if (!Description.DepthStencil.empty())
	{
		ClearValues[CountRenderTarget].depthStencil.depth = RenderPassRef->Description.DepthStencil.Depth;
		ClearValues[CountRenderTarget].depthStencil.stencil = RenderPassRef->Description.DepthStencil.Stencil;
		RenderPassBeginInfo.clearValueCount++;
	}
	RenderPassBeginInfo.pClearValues = ClearValues;
	return RenderPassBeginInfo;
}

void VKFrameBuffer::Unlock(VkCommandBuffer command_list)
{

	for (size_t i = 0; CountRenderTarget > i; i++)
	{
		auto texture = static_cast<VKTexture2D*>(Description.RenderTargets[i].get());
		TransitionImageLayout(command_list, texture->Image, texture->ImageCreateInfo.format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, texture->ImageCreateInfo.mipLevels, texture->ImageCreateInfo.arrayLayers,0);
	}
}

void VKFrameBuffer::Lock(VkCommandBuffer command_list)
{
	for (size_t i = 0; CountRenderTarget > i; i++)
	{
		auto texture = static_cast<VKTexture2D*>(Description.RenderTargets[i].get());
		TransitionImageLayout(command_list, texture->Image, texture->ImageCreateInfo.format, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, texture->ImageCreateInfo.mipLevels, texture->ImageCreateInfo.arrayLayers,0);
	}
}
