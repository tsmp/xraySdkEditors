#pragma once
class DX12Factory :public BearRHI::BearRHIFactory
{
public:
	DX12Factory();
	virtual ~DX12Factory();
	inline bool Empty() const { return Device.Get() == 0; }
	virtual BearRHI::BearRHIViewport*  CreateViewport(void* windows_handle, bsize width, bsize height, bool fullscreen, bool vsync, const BearViewportDescription& description);
	virtual BearRHI::BearRHIContext* CreateContext();
	virtual BearRHI::BearRHIShader* CreateShader(BearShaderType type);
	virtual BearRHI::BearRHIVertexBuffer* CreateVertexBuffer();
	virtual BearRHI::BearRHIIndexBuffer* CreateIndexBuffer();
	virtual BearRHI::BearRHIUniformBuffer* CreateUniformBuffer(bsize stride, bsize count, bool dynamic);
	virtual BearRHI::BearRHIRootSignature* CreateRootSignature(const BearRootSignatureDescription& description);
	virtual BearRHI::BearRHIDescriptorHeap* CreateDescriptorHeap(const BearDescriptorHeapDescription& description);
	virtual BearRHI::BearRHIPipelineGraphics* CreatePipelineGraphics(const BearPipelineGraphicsDescription& description);
	virtual BearRHI::BearRHIPipelineMesh* CreatePipelineMesh(const BearPipelineMeshDescription& description);
	virtual BearRHI::BearRHITexture2D* CreateTexture2D(bsize width, bsize height, bsize mips, bsize count, BearTexturePixelFormat pixel_format, BearTextureUsage type_usage, void* data = 0);
	virtual BearRHI::BearRHITextureCube* CreateTextureCube(bsize width, bsize height, bsize mips, bsize count, BearTexturePixelFormat pixel_format, BearTextureUsage type_usage, void* data = 0);
	virtual BearRHI::BearRHIStructuredBuffer* CreateStructuredBuffer(bsize size, void* data = 0, bool UAV=false);

	virtual BearRHI::BearRHITexture2D* CreateTexture2D(bsize width, bsize height, BearRenderTargetFormat format);
	virtual BearRHI::BearRHITexture2D* CreateTexture2D(bsize width, bsize height, BearDepthStencilFormat format);
	virtual BearRHI::BearRHIRenderPass *CreateRenderPass(const BearRenderPassDescription& description);
	virtual BearRHI::BearRHIFrameBuffer* CreateFrameBuffer(const BearFrameBufferDescription& description);
	virtual BearRHI::BearRHISampler* CreateSampler(const BearSamplerDescription& description);
	
	virtual BearRHI::BearRHIPipelineRayTracing* CreatePipelineRayTracing(const BearPipelineRayTracingDescription& description);
	virtual BearRHI::BearRHIRayTracingBottomLevel* CreateRayTracingBottomLevel(const BearRayTracingBottomLevelDescription& description);
	virtual BearRHI::BearRHIRayTracingTopLevel* CreateRayTracingTopLevel(const BearRayTracingTopLevelDescription& description);
	virtual BearRHI::BearRHIRayTracingShaderTable* CreateRayTracingShaderTable(const BearRayTracingShaderTableDescription& description);

	virtual bool SupportRayTracing();
	virtual bool SupportMeshShader();

	static DXGI_FORMAT Translation(BearTexturePixelFormat format);
	static D3D12_TEXTURE_ADDRESS_MODE Translation(BearSamplerAddressMode format);
	static D3D12_BLEND Translation(BearBlendFactor format);
	static D3D12_BLEND_OP Translation(BearBlendOp format);
	static D3D12_COMPARISON_FUNC Translation(BearCompareFunction format);
	static D3D12_STENCIL_OP Translation(BearStencilOp format);
	static D3D12_CULL_MODE Translation(BearRasterizerCullMode format);
	static D3D12_FILL_MODE Translation(BearRasterizerFillMode format);
	static DXGI_FORMAT Translation(BearRenderTargetFormat format);
	static DXGI_FORMAT Translation(BearDepthStencilFormat format);
	static DXGI_FORMAT TranslationForRayTracing(BearVertexFormat format);
	static DXGI_FORMAT Translation(BearVertexFormat format);
	static void Translation(BearTopologyType format, D3D_PRIMITIVE_TOPOLOGY&topology, D3D12_PRIMITIVE_TOPOLOGY_TYPE& topology_type);
public:
#ifdef RTX
	ComPtr<ID3D12RootSignature> LocalRootSignatureDefault;
	bool bSupportRayTracing;
#endif
#ifdef MESH_SHADING
	bool bSupportMeshShader;
#endif
	UINT SamplerDescriptorSize;
	UINT CbvSrvUavDescriptorSize;
	UINT RtvDescriptorSize;
	ComPtr<ID3D12DeviceX> Device;
	ComPtr<IDXGIFactoryX> GIFactory;

	DX12AllocatorHeap<D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,2048,false> ReserveResourceHeapAllocator;
	DX12AllocatorHeap<D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV> ShaderResourceHeapAllocator;
	DX12AllocatorHeap<D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 128, false> ReserveSamplersHeapAllocator;
	DX12AllocatorHeap<D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,128> SamplersHeapAllocator;
public:
	void LockCommandList();
	void UnlockCommandList(ID3D12CommandQueue*command_queue=0);
	ComPtr<ID3D12GraphicsCommandListX> CommandList;
public:
#ifndef DX11
	ComPtr < IDxcCompiler3> DxcCompiler;
	ComPtr < IDxcLibrary> DxcLibrary;
	IDxcIncludeHandler* DxcIncludeHandler;
#endif
private:
	void GetHardwareAdapter(IDXGIFactoryX* factory, IDXGIAdapter1** pp_adapter, D3D_FEATURE_LEVEL&level);
	BearVector<DXGI_MODE_DESC> m_GIVideoMode;
private:
	HANDLE m_Default_FenceEvent;
	uint64 m_Default_FenceValue;
	ComPtr<ID3D12Fence> m_Default_Fence;
	ComPtr<ID3D12CommandAllocator> m_Default_CommandAllocator;
	ComPtr<ID3D12CommandQueue> m_Default_CommandQueue;
	BearMutex m_Default_CommandMutex;
};

extern DX12Factory* Factory;