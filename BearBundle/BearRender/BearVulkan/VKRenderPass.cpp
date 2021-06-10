#include "VKPCH.h"
size_t RenderPassCounter = 0;
VKRenderPass::VKRenderPass(const BearRenderPassDescription& description)
{
	RenderPassCounter++;
	Description = description;
	CountRenderTarget = 0;
	for (; description.RenderTargets[CountRenderTarget].Format != BearRenderTargetFormat::None && CountRenderTarget < 8; CountRenderTarget++) {}


	VkAttachmentReference AttachmentReference[8] = {};


	VkAttachmentReference DepthAttachmentReference = {};
	
	DepthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription AttachmentDescription[9] = {};
	VkSubpassDescription SubpassDescription = {};

	for (size_t i = 0; i < CountRenderTarget; i++)
	{
		AttachmentDescription[i].format = VKFactory::Translation(description.RenderTargets[i].Format);
		AttachmentDescription[i].samples = VK_SAMPLE_COUNT_1_BIT;
		AttachmentDescription[i].loadOp = description.RenderTargets[i].Clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
		AttachmentDescription[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		AttachmentDescription[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		AttachmentDescription[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		AttachmentDescription[i].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		AttachmentDescription[i].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		AttachmentDescription[i].flags = 0;
		AttachmentReference[i].attachment = static_cast<uint32_t>(i);
		AttachmentReference[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	}
	
	SubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	SubpassDescription.flags = 0;
	SubpassDescription.inputAttachmentCount = 0;
	SubpassDescription.pInputAttachments = NULL;
	SubpassDescription.colorAttachmentCount = static_cast<uint32_t>(CountRenderTarget); ;
	SubpassDescription.pColorAttachments = AttachmentReference;
	SubpassDescription.pResolveAttachments = NULL;
	SubpassDescription.pDepthStencilAttachment = NULL;
	SubpassDescription.preserveAttachmentCount = 0;
	SubpassDescription.pPreserveAttachments = NULL;



	VkRenderPassCreateInfo RenderPassCreateInfo = {};
	RenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	RenderPassCreateInfo.pNext = NULL;
	RenderPassCreateInfo.attachmentCount = static_cast<uint32_t>(CountRenderTarget); ;
	RenderPassCreateInfo.pAttachments = AttachmentDescription;
	RenderPassCreateInfo.subpassCount = 1;
	RenderPassCreateInfo.pSubpasses = &SubpassDescription;
	RenderPassCreateInfo.dependencyCount = 0;
	RenderPassCreateInfo.pDependencies = NULL;

	if (description.DepthStencil.Format != BearDepthStencilFormat::None)
	{
		AttachmentDescription[CountRenderTarget].format = VKFactory::Translation(description.DepthStencil.Format);
		AttachmentDescription[CountRenderTarget].samples = VK_SAMPLE_COUNT_1_BIT;
		AttachmentDescription[CountRenderTarget].loadOp = description.DepthStencil.Clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
		AttachmentDescription[CountRenderTarget].storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		switch (description.DepthStencil.Format)
		{
		case BearDepthStencilFormat::Depth24Stencil8:

		case BearDepthStencilFormat::Depth32FStencil8:
			AttachmentDescription[CountRenderTarget].stencilLoadOp = description.DepthStencil.Clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
			AttachmentDescription[CountRenderTarget].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
			break;
		default:
			AttachmentDescription[CountRenderTarget].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			AttachmentDescription[CountRenderTarget].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			break;
		}

		AttachmentDescription[CountRenderTarget].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		AttachmentDescription[CountRenderTarget].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		AttachmentDescription[CountRenderTarget].flags = 0;
		RenderPassCreateInfo.attachmentCount++;
		SubpassDescription.pDepthStencilAttachment = &DepthAttachmentReference;
		DepthAttachmentReference.attachment = static_cast<uint32_t>(CountRenderTarget); ;

	}
	V_CHK(vkCreateRenderPass(Factory->Device, &RenderPassCreateInfo, NULL, &RenderPass));
}

VKRenderPass::~VKRenderPass()
{
	vkDestroyRenderPass(Factory->Device, RenderPass, 0);
	RenderPassCounter--;
}
