#pragma once
class VKViewport :public BearRHI::BearRHIViewport
{
public:
	//BEAR_CLASS_WITHOUT_COPY(VKViewport);
	VKViewport(void * handle, size_t width, size_t height, bool fullscreen, bool vsync, const BearViewportDescription&description);
	virtual ~VKViewport();
	virtual void SetVSync(bool sync);
	virtual void SetFullScreen(bool fullscreen);
	virtual void Resize(size_t width, size_t height);
	void Swap();
	virtual BearRenderTargetFormat GetFormat();
	VkSemaphore Semaphore;

	VkSwapchainKHR SwapChain;
	BearVector<VkImage> SwapChainImages;
	BearVector<VkImageView> SwapChainImageViews;
	BearVector<VkFramebuffer> Framebuffers;
	VkFormat SwapChainImageFormat;
	VkExtent2D SwapChainExtent;
	VkRenderPass RenderPass;
	BearColor ClearColor;
	uint32_t FrameIndex;
	size_t Width;
	size_t Height;
	bool VSync;

	VkQueue PresentQueue;
	VkRenderPassBeginInfo GetRenderPass();
	VkFence PresentFence;
private:
	BearViewportDescription Description;
	uint32_t m_PresentQueueFamilyIndex;
	struct SwapChainSupportDetails 
	{
		VkSurfaceCapabilitiesKHR capabilities;
		BearVector<VkSurfaceFormatKHR> formats;
		BearVector<VkPresentModeKHR> presentModes;
	};
	VkClearValue m_ClearValues[1];
	SwapChainSupportDetails QuerySwapChainSupport();
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const BearVector<VkSurfaceFormatKHR>& available_formats);
	VkPresentModeKHR ChooseSwapPresentMode(const BearVector<VkPresentModeKHR>& available_present_modes,bool vsync);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, size_t width, size_t height);
	VkSurfaceKHR Surface;


	uint32_t FindQueueFamilies();

	void CreateSwapChain(size_t width, size_t height,bool vsync);
	void CreateImageView();
	void DestroyImageView();
	void DestroySwapChain(VkSwapchainKHR swap_chain);
	void CreateFrameBuffers();
	void DestroyFrameBuffers();
	bool m_FullScreen;
	HWND m_WindowHandle;

};