#pragma once
class VKFactory :public BearRHI::BearRHIFactory
{
	BEAR_CLASS_WITHOUT_COPY(VKFactory);
private:
	bool LoadFunctions();
	bool CreateInstance();
	bool CreateDevice();
	bool CreateGPU(uint32_t& queue_family_index);
public:
	VKFactory();
	virtual ~VKFactory();
	virtual BearRHI::BearRHIContext* CreateContext();
	virtual BearRHI::BearRHIViewport* CreateViewport( void* handle, size_t width, size_t height, bool fullscreen, bool vsync, const BearViewportDescription& description);
	virtual BearRHI::BearRHIShader* CreateShader(BearShaderType Type);
	virtual BearRHI::BearRHIVertexBuffer* CreateVertexBuffer();
	virtual BearRHI::BearRHIIndexBuffer* CreateIndexBuffer();
	virtual BearRHI::BearRHIPipelineGraphics* CreatePipelineGraphics(const BearPipelineGraphicsDescription& description);
	virtual BearRHI::BearRHIPipelineMesh* CreatePipelineMesh(const BearPipelineMeshDescription& description);
	virtual BearRHI::BearRHIUniformBuffer* CreateUniformBuffer(size_t Stride, size_t Count, bool Dynamic);
	virtual BearRHI::BearRHIRootSignature* CreateRootSignature(const BearRootSignatureDescription& description);
	virtual BearRHI::BearRHIDescriptorHeap* CreateDescriptorHeap(const BearDescriptorHeapDescription& description);
	virtual BearRHI::BearRHITexture2D* CreateTexture2D(size_t width, size_t height, size_t mips, size_t count, BearTexturePixelFormat pixel_format, BearTextureUsage type_usage, void* data = 0);
	virtual BearRHI::BearRHITextureCube* CreateTextureCube(size_t width, size_t height, size_t mips, size_t count, BearTexturePixelFormat pixel_format, BearTextureUsage type_usage, void* data = 0);
	virtual BearRHI::BearRHIStructuredBuffer* CreateStructuredBuffer(size_t size, void* data = 0, bool UAV = false);

	virtual BearRHI::BearRHITexture2D* CreateTexture2D(size_t width, size_t height, BearRenderTargetFormat format);
	virtual BearRHI::BearRHITexture2D* CreateTexture2D(size_t width, size_t height, BearDepthStencilFormat format);
	virtual BearRHI::BearRHISampler* CreateSampler(const BearSamplerDescription& description);
	virtual BearRHI::BearRHIRenderPass* CreateRenderPass(const BearRenderPassDescription& description);
	virtual BearRHI::BearRHIFrameBuffer* CreateFrameBuffer(const BearFrameBufferDescription& description);

	virtual BearRHI::BearRHIPipelineRayTracing* CreatePipelineRayTracing(const BearPipelineRayTracingDescription& description);
	virtual BearRHI::BearRHIRayTracingBottomLevel* CreateRayTracingBottomLevel(const BearRayTracingBottomLevelDescription& description);
	virtual BearRHI::BearRHIRayTracingTopLevel* CreateRayTracingTopLevel(const BearRayTracingTopLevelDescription& description);
	virtual BearRHI::BearRHIRayTracingShaderTable* CreateRayTracingShaderTable(const BearRayTracingShaderTableDescription& description);

	static VkSamplerAddressMode Translation(BearSamplerAddressMode format);
	static VkCullModeFlagBits Translation(BearRasterizerCullMode format);
	static VkPolygonMode Translation(BearRasterizerFillMode format);
	static VkFrontFace Translation(BearRasterizerFrontFace format);

	static VkBlendFactor Translation(BearBlendFactor format);
	static VkBlendOp Translation(BearBlendOp format);
	static VkCompareOp Translation(BearCompareFunction format);
	static VkStencilOp Translation(BearStencilOp format);

	static VkFormat  Translation(BearTexturePixelFormat format);
	static VkFormat  Translation(BearRenderTargetFormat format);
	static VkFormat  Translation(BearDepthStencilFormat format);

	static VkFormat Translation(BearVertexFormat format);
	static bsize TranslationInSize(BearVertexFormat format);
	static VkFormat TranslationForRayTracing(BearVertexFormat format);
	static VkGeometryTypeKHR Translation(BearRaytracingGeometryType format);
	static VkPrimitiveTopology Translation(BearTopologyType format);
	static VkColorComponentFlags Translation(BearColorWriteFlags flags);
public:
	inline bool Empty()const { return Instance==0; }
	virtual bool SupportRayTracing();
	virtual bool SupportMeshShader();
public:
	VkCommandBuffer CommandBuffer;
	void LockCommandBuffer();
	void UnlockCommandBuffer();
public:
	VkSampler DefaultSampler;
private:
	BearMutex m_CommandMutex;
	VkCommandPool m_CommandPool;
public:
	VkPhysicalDeviceFeatures DeviceFeatures;
	VkPhysicalDeviceProperties PhysicalDeviceProperties;
#ifdef RTX
	VkPhysicalDeviceRayTracingPropertiesNV PhysicalDeviceRayTracingProperties;
#endif
	VkPhysicalDeviceMemoryProperties PhysicalDeviceMemoryProperties;
public:
	VkInstance Instance;
	VkPhysicalDevice PhysicalDevice;
	VkDevice Device;
public:
	uint32_t QueueFamilyIndex;
	VkQueue Queue;
	VkSemaphore SemaphoreWait;
	VkFence Fence;
private:
#ifdef DEVELOPER_VERSION
	VkDebugUtilsMessengerEXT m_DebugMessenger;
#endif
public:
#ifdef RTX
#ifdef DEVELOPER_VERSION
	IDxcCompiler3* DxcCompiler;
	IDxcLibrary* DxcLibrary;
#endif
#endif
public:
#ifdef RTX
	bool bSupportRayTracing;
#endif
};

extern VKFactory* Factory;