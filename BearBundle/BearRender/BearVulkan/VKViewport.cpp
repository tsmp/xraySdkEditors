#include "VKPCH.h"
size_t ViewportCounter = 0;
VKViewport::VKViewport(void * handle, size_t width, size_t height, bool fullscreen, bool vsync, const BearViewportDescription&description):Width(width),Height(height), Description(description)
{
	ViewportCounter++;
	m_FullScreen = 0;
	m_WindowHandle = (HWND)handle;
	ClearColor = Description.ClearColor;
	RenderPass = VK_NULL_HANDLE;
	SwapChain = VK_NULL_HANDLE;
	{
		VkWin32SurfaceCreateInfoKHR SurfaceCreateInfo = {};
		SurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		SurfaceCreateInfo.pNext = NULL;
		SurfaceCreateInfo.hinstance = GetModuleHandle(0);
		SurfaceCreateInfo.hwnd = (HWND)handle;
		V_CHK(vkCreateWin32SurfaceKHR(Factory->Instance, &SurfaceCreateInfo, NULL, &Surface));
	}

	m_PresentQueueFamilyIndex = FindQueueFamilies();


	


	CreateSwapChain(Width,Height, vsync);
	{
		VkSemaphoreCreateInfo SemaphoreCreateInfo = {};
		SemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		SemaphoreCreateInfo.pNext = NULL;
		SemaphoreCreateInfo.flags = 0;

		V_CHK( vkCreateSemaphore(Factory->Device, &SemaphoreCreateInfo, NULL, &Semaphore));

		// Get the index of the next available swapchain image:
		V_CHK(vkAcquireNextImageKHR(Factory->Device,SwapChain, UINT64_MAX, Semaphore, VK_NULL_HANDLE,
			&FrameIndex));
	}
	{

		if (m_PresentQueueFamilyIndex == Factory->QueueFamilyIndex)
		{
			PresentQueue = Factory->Queue;
		}
		else {
			vkGetDeviceQueue(Factory->Device, m_PresentQueueFamilyIndex, 0, &PresentQueue);
		}
		
	}
	{
		VkFenceCreateInfo FenceCreateInfo = {};
		FenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		FenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		vkCreateFence(Factory->Device, &FenceCreateInfo, nullptr, &PresentFence);
		V_CHK(vkWaitForFences(Factory->Device, 1, &PresentFence, true, UINT64_MAX));
		V_CHK(vkResetFences(Factory->Device, 1, &PresentFence));
	}
	SetFullScreen(fullscreen);
}

VKViewport::~VKViewport()
{
	ViewportCounter--;
	vkDestroyFence(Factory->Device, PresentFence,0);
	vkDestroySemaphore(Factory->Device, Semaphore, 0);
	DestroySwapChain(SwapChain);
	vkDestroySurfaceKHR(Factory->Instance, Surface, 0);
	vkDestroyRenderPass(Factory->Device, RenderPass,0);
}

void VKViewport::SetVSync(bool vsync)
{
	if (vsync != vsync)CreateSwapChain(Width, Height, vsync);
	VSync = vsync;
}

void VKViewport::SetFullScreen(bool FullScreen)
{
	if (m_FullScreen == FullScreen)return;
	if (FullScreen)
	{
		DEVMODE ScreenSettings = {};
		ScreenSettings.dmSize = sizeof(ScreenSettings);
		ScreenSettings.dmPelsWidth = static_cast<DWORD>(Width);
		ScreenSettings.dmPelsHeight = static_cast<DWORD>(Height);
		ScreenSettings.dmBitsPerPel = 32;
		ScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		ChangeDisplaySettings(&ScreenSettings, CDS_FULLSCREEN);
	}
	else
	{
		ChangeDisplaySettings(NULL, 0);
	}
	m_FullScreen = FullScreen;
}

void VKViewport::Resize(size_t width, size_t height)
{
	if (width != Width || Height != height)CreateSwapChain(Width, Height, VSync);
	Width = width;
	Height = height;
}




void VKViewport::Swap()
{

	VkPresentInfoKHR PresentInfo = {};
	PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	PresentInfo.pNext = NULL;
	PresentInfo.swapchainCount = 1;
	PresentInfo.pWaitSemaphores = &Semaphore;
	PresentInfo.waitSemaphoreCount = 1;
	PresentInfo.pSwapchains = &SwapChain;
	PresentInfo.pImageIndices = &FrameIndex;
	auto result = vkQueuePresentKHR(PresentQueue, &PresentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		CreateSwapChain(Width, Height, VSync);
	}
	else
	{
		V_CHK(result);
	}
	V_CHK(vkQueueSubmit(PresentQueue, 0, nullptr, PresentFence));
	V_CHK(vkWaitForFences(Factory->Device, 1, &PresentFence, true, UINT64_MAX));
	V_CHK(vkResetFences(Factory->Device, 1, &PresentFence));
	V_CHK(vkAcquireNextImageKHR(Factory->Device, SwapChain, UINT64_MAX, Semaphore, VK_NULL_HANDLE,&FrameIndex));

}

BearRenderTargetFormat VKViewport::GetFormat()
{
	switch (SwapChainImageFormat)
	{
	case VK_FORMAT_R8G8B8A8_UNORM:
		return  BearRenderTargetFormat::R8G8B8A8;
		break;
	case VK_FORMAT_B8G8R8A8_UNORM:
		return  BearRenderTargetFormat::B8G8R8A8;
		break;
	case VK_FORMAT_R32G32B32A32_SFLOAT:
		return  BearRenderTargetFormat::R32G32B32A32F;
		break;
	default:
		BEAR_CHECK(0);
		break;
	}
	return  BearRenderTargetFormat::B8G8R8A8;
}

VkRenderPassBeginInfo VKViewport::GetRenderPass()
{
	VkRenderPassBeginInfo RenderPassBeginInfo = {};
	RenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	RenderPassBeginInfo.pNext = NULL;
	RenderPassBeginInfo.renderPass = RenderPass;
	RenderPassBeginInfo.framebuffer = Framebuffers[FrameIndex];
	RenderPassBeginInfo.renderArea.offset.x = 0;
	RenderPassBeginInfo.renderArea.offset.y = 0;
	RenderPassBeginInfo.renderArea.extent.width = static_cast<uint32_t>( Width);
	RenderPassBeginInfo.renderArea.extent.height = static_cast<uint32_t>(Height);
	RenderPassBeginInfo.clearValueCount = 1;
	m_ClearValues[0].color.float32[0] = ClearColor.R32F;
	m_ClearValues[0].color.float32[1] = ClearColor.G32F;
	m_ClearValues[0].color.float32[2] = ClearColor.B32F;
	m_ClearValues[0].color.float32[3] = ClearColor.A32F;
	RenderPassBeginInfo.pClearValues = m_ClearValues;
	return RenderPassBeginInfo;
}

VKViewport::SwapChainSupportDetails VKViewport::QuerySwapChainSupport()
{
	SwapChainSupportDetails Details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Factory->PhysicalDevice, Surface, &Details.capabilities);
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(Factory->PhysicalDevice, Surface, &formatCount, nullptr);

	if (formatCount != 0) {
		Details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(Factory->PhysicalDevice, Surface, &formatCount, Details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(Factory->PhysicalDevice, Surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		Details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(Factory->PhysicalDevice, Surface, &presentModeCount, Details.presentModes.data());
	}

	return Details;
}

VkSurfaceFormatKHR VKViewport::ChooseSwapSurfaceFormat(const BearVector<VkSurfaceFormatKHR>& available_formats)
{
	for (const auto& a : available_formats) 
	{
		if (a.format == VK_FORMAT_B8G8R8A8_UNORM && a.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return a;
		}
	}

	return available_formats[0];
}

VkPresentModeKHR VKViewport::ChooseSwapPresentMode(const BearVector<VkPresentModeKHR>& available_present_modes, bool vsync)
{
	for (const auto& a : available_present_modes) 
	{
		if (a == VK_PRESENT_MODE_MAILBOX_KHR&& vsync)
		{
			return a;
		}
		if (a == VK_PRESENT_MODE_IMMEDIATE_KHR && !vsync)
		{
			return a;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VKViewport::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR & capabilities, size_t width, size_t height)
{
	if (capabilities.currentExtent.width != UINT32_MAX)
	{
		return capabilities.currentExtent;
	}
	else 
	{
		VkExtent2D ActualExtent = {static_cast<uint32_t>( width),static_cast<uint32_t>(height) };

		ActualExtent.width = BearMath::max(capabilities.minImageExtent.width, BearMath::min(capabilities.maxImageExtent.width, ActualExtent.width));
		ActualExtent.height = BearMath::max(capabilities.minImageExtent.height,BearMath::min(capabilities.maxImageExtent.height, ActualExtent.height));

		return ActualExtent;
	}
}

uint32_t VKViewport::FindQueueFamilies()
{
	uint32_t QueueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(Factory->PhysicalDevice, &QueueFamilyCount, nullptr);

	BearVector<VkQueueFamilyProperties> QueueFamilies(QueueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(Factory->PhysicalDevice, &QueueFamilyCount, QueueFamilies.data());
	{
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(Factory->PhysicalDevice, Factory->QueueFamilyIndex, Surface, &presentSupport);

		if (QueueFamilies[Factory->QueueFamilyIndex].queueCount > 0 && presentSupport)
		{
			return Factory->QueueFamilyIndex;
		}
	}
	int i = 0;
	int result = -1;
	for (const auto& a : QueueFamilies)
	{

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(Factory->PhysicalDevice, i, Surface, &presentSupport);

		if (a.queueCount > 0 && presentSupport)
		{
			result = i; break;
		}

	}

	BEAR_CHECK(result >= 0);
	return static_cast<uint32_t>( result);
}

void VKViewport::CreateSwapChain(size_t width, size_t height, bool vsync)
{
	SwapChainSupportDetails SwapChainSupport = QuerySwapChainSupport();

	VkSurfaceFormatKHR SurfaceFormat = ChooseSwapSurfaceFormat(SwapChainSupport.formats);
	VkPresentModeKHR PresentMode = ChooseSwapPresentMode(SwapChainSupport.presentModes, vsync);
	VkExtent2D Extent = ChooseSwapExtent(SwapChainSupport.capabilities, width, height);

	uint32_t ImageCount = SwapChainSupport.capabilities.minImageCount + 1;
	if (SwapChainSupport.capabilities.maxImageCount > 0 && ImageCount > SwapChainSupport.capabilities.maxImageCount)
	{
		ImageCount = SwapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR SwapchainCreateInfo = {};
	SwapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	SwapchainCreateInfo.surface = Surface;

	SwapchainCreateInfo.minImageCount = ImageCount;
	SwapchainCreateInfo.imageFormat = SurfaceFormat.format;
	SwapchainCreateInfo.imageColorSpace = SurfaceFormat.colorSpace;
	SwapchainCreateInfo.imageExtent = Extent;
	SwapchainCreateInfo.imageArrayLayers = 1;
	SwapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;


	uint32_t QueueFamilyIndices[] = { Factory->QueueFamilyIndex, m_PresentQueueFamilyIndex };

	if (Factory->QueueFamilyIndex != m_PresentQueueFamilyIndex)
	{
		SwapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		SwapchainCreateInfo.queueFamilyIndexCount = 2;
		SwapchainCreateInfo.pQueueFamilyIndices = QueueFamilyIndices;
	}
	else {
		SwapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	SwapchainCreateInfo.preTransform = SwapChainSupport.capabilities.currentTransform;
	SwapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	SwapchainCreateInfo.presentMode = PresentMode;
	SwapchainCreateInfo.clipped = VK_TRUE;

	SwapchainCreateInfo.oldSwapchain = SwapChain;
	
	V_CHK(vkCreateSwapchainKHR(Factory->Device, &SwapchainCreateInfo, nullptr, &SwapChain));
	DestroySwapChain(SwapchainCreateInfo.oldSwapchain);
	V_CHK(vkGetSwapchainImagesKHR(Factory->Device, SwapChain, &ImageCount, nullptr));
	SwapChainImages.resize(ImageCount);
	V_CHK(vkGetSwapchainImagesKHR(Factory->Device, SwapChain, &ImageCount, SwapChainImages.data()));
	SwapChainImageFormat = SurfaceFormat.format;
	SwapChainExtent = Extent;
	CreateImageView();

}

void VKViewport::CreateImageView()
{
	SwapChainImageViews.resize(SwapChainImages.size());
	for (uint32_t i = 0; i < SwapChainImages.size(); i++)
	{
		VkImageViewCreateInfo ImageViewCreateInfo = {};
		ImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ImageViewCreateInfo.pNext = NULL;
		ImageViewCreateInfo.flags = 0;
		ImageViewCreateInfo.image = SwapChainImages[i];
		ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ImageViewCreateInfo.format = SwapChainImageFormat;
		ImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
		ImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
		ImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
		ImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
		ImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		ImageViewCreateInfo.subresourceRange.levelCount = 1;
		ImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		ImageViewCreateInfo.subresourceRange.layerCount = 1;

		V_CHK(vkCreateImageView(Factory->Device, &ImageViewCreateInfo, NULL, &SwapChainImageViews[i]));
		
	}
	CreateFrameBuffers();
}

void VKViewport::DestroyImageView()
{
	for (uint32_t i = 0; i < SwapChainImageViews.size(); i++)
	{
		vkDestroyImageView(Factory->Device, SwapChainImageViews[i], NULL);
	}
}

void VKViewport::DestroySwapChain(VkSwapchainKHR swapChain)
{
	if (swapChain == VK_NULL_HANDLE) return;
	vkDeviceWaitIdle(Factory->Device);
	DestroyFrameBuffers();
	DestroyImageView();
	vkDestroySwapchainKHR(Factory->Device, swapChain, 0);
}

void VKViewport::CreateFrameBuffers()
{

	if(RenderPass==VK_NULL_HANDLE)

	{
		VkAttachmentDescription AttachmentDescription[2];
		AttachmentDescription[0].format = SwapChainImageFormat;
		AttachmentDescription[0].samples = VK_SAMPLE_COUNT_1_BIT;
		AttachmentDescription[0].loadOp = Description.Clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
		AttachmentDescription[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		AttachmentDescription[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		AttachmentDescription[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		AttachmentDescription[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		AttachmentDescription[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		AttachmentDescription[0].flags = 0;

		VkAttachmentReference ColorAttachmentReference = {};
		ColorAttachmentReference.attachment = 0;
		ColorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference DepthAttachmentReference = {};
		DepthAttachmentReference.attachment = 1;
		DepthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription SubpassDescription = {};
		SubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		SubpassDescription.flags = 0;
		SubpassDescription.inputAttachmentCount = 0;
		SubpassDescription.pInputAttachments = NULL;
		SubpassDescription.colorAttachmentCount = 1;
		SubpassDescription.pColorAttachments = &ColorAttachmentReference;
		SubpassDescription.pResolveAttachments = NULL;
		SubpassDescription.pDepthStencilAttachment = NULL;
		SubpassDescription.preserveAttachmentCount = 0;
		SubpassDescription.pPreserveAttachments = NULL;

		VkRenderPassCreateInfo RenderPassCreateInfo  = {};
		RenderPassCreateInfo .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		RenderPassCreateInfo .pNext = NULL;
		RenderPassCreateInfo .attachmentCount = 1;
		RenderPassCreateInfo .pAttachments = AttachmentDescription;
		RenderPassCreateInfo .subpassCount = 1;
		RenderPassCreateInfo .pSubpasses = &SubpassDescription;
		RenderPassCreateInfo .dependencyCount = 0;
		RenderPassCreateInfo .pDependencies = NULL;

		V_CHK(vkCreateRenderPass(Factory->Device, &RenderPassCreateInfo , NULL, &RenderPass));
	}
	VkImageView AttachmentDescription[1];

	VkFramebufferCreateInfo FramebufferCreateInfo = {};
	FramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	FramebufferCreateInfo.pNext = NULL;
	FramebufferCreateInfo.renderPass = RenderPass;
	FramebufferCreateInfo.attachmentCount = 1;
	FramebufferCreateInfo.pAttachments = AttachmentDescription;
	FramebufferCreateInfo.width = static_cast<uint32_t>(Width);
	FramebufferCreateInfo.height = static_cast<uint32_t>(Height); ;
	FramebufferCreateInfo.layers = 1;


	Framebuffers.resize(SwapChainImages.size());

	for (uint32_t i = 0; i < Framebuffers.size(); i++)
	{
		AttachmentDescription[0] = SwapChainImageViews[i];
		V_CHK(vkCreateFramebuffer(Factory->Device, &FramebufferCreateInfo, NULL, &Framebuffers[i]));
	}

}

void VKViewport::DestroyFrameBuffers()
{
	for (uint32_t i = 0; i < Framebuffers.size(); i++)
	{
		vkDestroyFramebuffer(Factory->Device, Framebuffers[i], 0);
	}

}

