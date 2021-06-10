#include "DX12PCH.h"

#ifdef DEVELOPER_VERSION
bool GDebugRender;  
#endif

DX12Factory::DX12Factory()
{
	ComPtr<IDXGIAdapter1> HardwareAdapter;
	D3D_FEATURE_LEVEL Level;
	UINT DxGIFactoryFlags = 0;
#ifndef DX11
	DxcIncludeHandler = 0;
#endif
	m_Default_FenceEvent = 0;

#ifdef RTX
	bSupportRayTracing = false;
#endif
#ifdef MESH_SHADING
	bSupportMeshShader = false;
#endif

#ifdef DEVELOPER_VERSION
	GDebugRender = BearString::Find(GetCommandLine(), TEXT("-debugrender"));
	if(!GDebugRender)
		GDebugRender = BearString::Find(GetCommandLine(), TEXT("-drender"));
#if defined(_DEBUG)
	GDebugRender = true;
#endif

	if(GDebugRender)

	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
			DxGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif

	if (FAILED(CreateDXGIFactory2(DxGIFactoryFlags, IID_PPV_ARGS(&GIFactory))))
	{
		return;
	}

	GetHardwareAdapter(GIFactory.Get(), &HardwareAdapter, Level);
	if (HardwareAdapter.Get() == 0)
		return;
	IDXGIOutput *Output;
	if (FAILED(HardwareAdapter->EnumOutputs(0, &Output)))
		return;
	{
		UINT count = 0;

		Output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &count, 0);
		m_GIVideoMode.resize(count);
		Output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &count, &m_GIVideoMode[0]);

		Output->Release();
	}

	if (FAILED(D3D12CreateDevice(HardwareAdapter.Get(), Level, IID_PPV_ARGS(&Device))))
	{
		return;
	}

#ifdef DEVELOPER_VERSION
	if (GDebugRender)
	{

		ComPtr<ID3D12InfoQueue> InfoQueue;
		if (SUCCEEDED(Device.As(&InfoQueue)))
		{
			InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
			InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

			D3D12_MESSAGE_SEVERITY Severities[] =
				{
					D3D12_MESSAGE_SEVERITY_INFO};

			// Suppress individual messages by their ID
			D3D12_MESSAGE_ID DenyIds[] = {
				D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE,
				D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE, // I'm really not sure how to avoid this message.
				D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,						  // This warning occurs when using capture frame while graphics debugging.
				D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,					  // This warning occurs when using capture frame while graphics debugging.
			};

			D3D12_INFO_QUEUE_FILTER NewFilter = {};
			NewFilter.DenyList.NumSeverities = _countof(Severities);
			NewFilter.DenyList.pSeverityList = Severities;
			NewFilter.DenyList.NumIDs = _countof(DenyIds);
			NewFilter.DenyList.pIDList = DenyIds;

			R_CHK(InfoQueue->PushStorageFilter(&NewFilter));
		}

	}
#endif

	{
		CbvSrvUavDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		RtvDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		SamplerDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	}

#ifndef DX11
	{
		R_CHK(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&DxcCompiler)));
		R_CHK(DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&DxcLibrary)));
		R_CHK(DxcLibrary->CreateIncludeHandler(&DxcIncludeHandler));
	}
#endif

	{
		m_Default_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_Default_FenceEvent == nullptr)
		{
			R_CHK(HRESULT_FROM_WIN32(GetLastError()));
		}
		R_CHK(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_Default_CommandAllocator)));
		R_CHK(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_Default_CommandAllocator.Get(), 0, IID_PPV_ARGS(&CommandList)));
		CommandList->Close();
		{
			D3D12_COMMAND_QUEUE_DESC queueDesc = {};
			queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
			queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
			R_CHK(Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_Default_CommandQueue)));
		}
		R_CHK(Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Default_Fence)));
		m_Default_FenceValue = 1;
	}
#ifdef RTX
	if(bSupportRayTracing)
	{
		CD3DX12_ROOT_SIGNATURE_DESC RootSignatureDesc;
		RootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);

		ComPtr<ID3DBlob> Signature;
		ComPtr<ID3DBlob> Error;
		R_CHK(D3D12SerializeRootSignature(&RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &Signature, &Error));
		R_CHK(Device->CreateRootSignature(0, Signature->GetBufferPointer(), Signature->GetBufferSize(), IID_PPV_ARGS(&LocalRootSignatureDefault)));
	}
#endif
}

DX12Factory::~DX12Factory()
{
#ifndef DX11
	{
		if(DxcIncludeHandler)
		DxcIncludeHandler->Release();
	}
#endif
	{
		ShaderResourceHeapAllocator.clear();
		SamplersHeapAllocator.clear();
		ReserveResourceHeapAllocator.clear();
	}
	{

		if (m_Default_FenceEvent)
			CloseHandle(m_Default_FenceEvent);
		m_Default_FenceEvent = 0;
	}
}
BearRHI::BearRHIViewport *DX12Factory::CreateViewport(void* window_handle, bsize width, bsize height, bool Fullscreen, bool VSync, const BearViewportDescription&description)
{
	return bear_new<DX12Viewport>(window_handle, width,height,Fullscreen,VSync,description);
}

BearRHI::BearRHIContext *DX12Factory::CreateContext()
{
	return bear_new<DX12Context>();
}

BearRHI::BearRHIShader* DX12Factory::CreateShader(BearShaderType Type)
{
	return bear_new< DX12Shader>(Type);
}

BearRHI::BearRHIVertexBuffer* DX12Factory::CreateVertexBuffer()
{
	return bear_new<DX12VertexBuffer>();
}

BearRHI::BearRHIIndexBuffer* DX12Factory::CreateIndexBuffer()
{
	return bear_new<DX12IndexBuffer>();
}

BearRHI::BearRHIUniformBuffer* DX12Factory::CreateUniformBuffer(bsize Stride, bsize count, bool dynamic)
{
	return bear_new<DX12UniformBuffer>(Stride, count, dynamic);
}

BearRHI::BearRHIRootSignature* DX12Factory::CreateRootSignature(const BearRootSignatureDescription& description)
{
	return bear_new< DX12RootSignature>(description);
}

BearRHI::BearRHIDescriptorHeap* DX12Factory::CreateDescriptorHeap(const BearDescriptorHeapDescription& description)
{
	return  bear_new<DX12DescriptorHeap>(description);;
}

BearRHI::BearRHIPipelineGraphics* DX12Factory::CreatePipelineGraphics(const BearPipelineGraphicsDescription& description)
{
	return bear_new<DX12PipelineGraphics>(description);
}

BearRHI::BearRHIPipelineMesh* DX12Factory::CreatePipelineMesh(const BearPipelineMeshDescription& description)
{
#ifdef MESH_SHADING
	return bear_new<DX12PipelineMesh>(description);
#else
	return nullptr;
#endif
}

BearRHI::BearRHIPipelineRayTracing* DX12Factory::CreatePipelineRayTracing(const BearPipelineRayTracingDescription& description)
{
#ifdef RTX
	return bear_new<DX12PipelineRayTracing>(description);
#else
	return nullptr;
#endif
}

BearRHI::BearRHITexture2D* DX12Factory::CreateTexture2D(bsize width, bsize height, bsize mips, bsize count, BearTexturePixelFormat pixel_format, BearTextureUsage type_usage, void* data)
{
	return  bear_new<DX12Texture2D>(width,height,mips,count,pixel_format, type_usage,data);;
}

BearRHI::BearRHITextureCube* DX12Factory::CreateTextureCube(bsize width, bsize height, bsize mips, bsize count, BearTexturePixelFormat pixel_format, BearTextureUsage type_usage, void* data)
{
	return  bear_new<DX12TextureCube>(width, height, mips, count, pixel_format, type_usage, data);;
}

BearRHI::BearRHIStructuredBuffer* DX12Factory::CreateStructuredBuffer(bsize size, void* data,bool uav)
{
	return  bear_new<DX12StructuredBuffer>(size, data, uav);
}

BearRHI::BearRHITexture2D* DX12Factory::CreateTexture2D(bsize width, bsize height, BearRenderTargetFormat format)
{
	return bear_new<DX12Texture2D>(width, height, format);
}

BearRHI::BearRHITexture2D* DX12Factory::CreateTexture2D(bsize width, bsize height, BearDepthStencilFormat format)
{
	return bear_new<DX12Texture2D>(width, height, format);
}

BearRHI::BearRHIRenderPass *DX12Factory::CreateRenderPass(const BearRenderPassDescription& description)
{
	return bear_new<DX12RenderPass>(description);
}

BearRHI::BearRHIFrameBuffer* DX12Factory::CreateFrameBuffer(const BearFrameBufferDescription& description)
{
	return bear_new<DX12FrameBuffer>(description);
}

BearRHI::BearRHISampler* DX12Factory::CreateSampler(const BearSamplerDescription& description)
{
	return  bear_new<DX12SamplerState>(description);
}

BearRHI::BearRHIRayTracingBottomLevel* DX12Factory::CreateRayTracingBottomLevel(const BearRayTracingBottomLevelDescription& description)
{
#ifdef RTX
	return  bear_new<DX12RayTracingBottomLevel>(description);
#else
	return nullptr;
#endif
}

BearRHI::BearRHIRayTracingTopLevel* DX12Factory::CreateRayTracingTopLevel(const BearRayTracingTopLevelDescription& description)
{
#ifdef RTX
	return  bear_new<DX12RayTracingTopLevel>(description);
#else
	return nullptr;
#endif
}

BearRHI::BearRHIRayTracingShaderTable* DX12Factory::CreateRayTracingShaderTable(const BearRayTracingShaderTableDescription& description)
{
#ifdef RTX
	return bear_new<DX12RayTracingShaderTable>(description);
#else
	return nullptr;
#endif
}

void DX12Factory::GetHardwareAdapter(IDXGIFactoryX* factory, IDXGIAdapter1** pp_adapter, D3D_FEATURE_LEVEL& level)
{
	ComPtr<IDXGIAdapter1> Adapter;
	*pp_adapter = nullptr;

	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(adapterIndex, &Adapter); ++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 AdapterDesc;
		Adapter->GetDesc1(&AdapterDesc);

		if (AdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			continue;
		}

		level = CurrentFeatureLevel;

		ComPtr<ID3D12Device> LDevice;
		if (SUCCEEDED(D3D12CreateDevice(Adapter.Get(), level, IID_PPV_ARGS(&LDevice))))
		{
			{
				D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = { CurrentShadeModel };
				if (FAILED(LDevice->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel))))
				{
					continue;
				}
			}
#ifdef RTX
			{
				D3D12_FEATURE_DATA_D3D12_OPTIONS5 featureSupportData = {};
				if (SUCCEEDED(LDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &featureSupportData, sizeof(featureSupportData))))
				{
					bSupportRayTracing = featureSupportData.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
				}
				D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = { CurrentRTXShadeModel };
				if (FAILED(LDevice->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel))))
				{
					bSupportRayTracing = false;
				}
			}
#endif


#ifdef MESH_SHADING
			{
				D3D12_FEATURE_DATA_D3D12_OPTIONS7 features = {};
				if (SUCCEEDED(LDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &features, sizeof(features))) || (features.MeshShaderTier == D3D12_MESH_SHADER_TIER_NOT_SUPPORTED))
				{
					bSupportMeshShader = false;
				}
				D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = { CurrentMeshShadingShadeModel };
				if (FAILED(LDevice->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel))))
				{
					bSupportMeshShader = false;
				}
			}
#endif
			*pp_adapter = Adapter.Detach();
			break;
		}
	}
}
void DX12Factory::LockCommandList()
{
	m_Default_CommandMutex.Lock();
	R_CHK(m_Default_CommandAllocator->Reset());
	R_CHK(CommandList->Reset(m_Default_CommandAllocator.Get(), 0));
}

void DX12Factory::UnlockCommandList(ID3D12CommandQueue *command_queue)
{
	R_CHK(CommandList->Close());
	{
		ID3D12CommandList* CommandLists[] = { CommandList.Get() };
		if (command_queue)
			command_queue->ExecuteCommandLists(_countof(CommandLists), CommandLists);
		else
			m_Default_CommandQueue->ExecuteCommandLists(_countof(CommandLists), CommandLists);
	}
	const uint64 Fence = m_Default_FenceValue;
	if (command_queue)
	{
		R_CHK(command_queue->Signal(m_Default_Fence.Get(), Fence));
	}
	else
	{
		R_CHK(m_Default_CommandQueue->Signal(m_Default_Fence.Get(), Fence));
	}
	m_Default_FenceValue++;
	if (m_Default_Fence->GetCompletedValue() < Fence)
	{
		R_CHK(m_Default_Fence->SetEventOnCompletion(Fence, m_Default_FenceEvent));
		WaitForSingleObject(m_Default_FenceEvent, INFINITE);
	}
	m_Default_CommandMutex.Unlock();
}


bool DX12Factory::SupportRayTracing()
{
#ifdef RTX
	return bSupportRayTracing;
#else
	return false;
#endif
}

bool DX12Factory::SupportMeshShader()
{
#ifdef MESH_SAHDING
	return bSupportMeshShader;
#else
	return false;
#endif
}

DXGI_FORMAT DX12Factory::Translation(BearTexturePixelFormat format)
{
	switch (format)
	{
	case BearTexturePixelFormat::R8:
		return DXGI_FORMAT_R8_UNORM;
		break;
	case BearTexturePixelFormat::R8G8:
		return DXGI_FORMAT_R8G8_UNORM;
		break;
	case BearTexturePixelFormat::R8G8B8:
		BEAR_ASSERT(!"not support R8G8B8");
		break;
	case BearTexturePixelFormat::R8G8B8A8:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
		break;
	case BearTexturePixelFormat::R32F:
		return DXGI_FORMAT_R32_FLOAT;
		break;
	case BearTexturePixelFormat::R32G32F:
		return DXGI_FORMAT_R32G32_FLOAT;
		break;
	case BearTexturePixelFormat::R32G32B32F:
		return DXGI_FORMAT_R32G32B32_FLOAT;
		break;
	case BearTexturePixelFormat::R32G32B32A32F:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
		break;
	case BearTexturePixelFormat::BC1:
	case BearTexturePixelFormat::BC1a:
		return DXGI_FORMAT_BC1_UNORM;
	case BearTexturePixelFormat::BC2:
		return DXGI_FORMAT_BC2_UNORM;
	case BearTexturePixelFormat::BC3:
		return DXGI_FORMAT_BC3_UNORM;
	case BearTexturePixelFormat::BC4:
		return DXGI_FORMAT_BC4_UNORM;
	case BearTexturePixelFormat::BC5:
		return DXGI_FORMAT_BC5_UNORM;
	case BearTexturePixelFormat::BC6:
		return DXGI_FORMAT_BC6H_UF16;
	case BearTexturePixelFormat::BC7:
		return DXGI_FORMAT_BC7_UNORM;
	default:
		BEAR_CHECK(0);;
	}
	return DXGI_FORMAT_UNKNOWN;

}

D3D12_TEXTURE_ADDRESS_MODE DX12Factory::Translation(BearSamplerAddressMode format)
{
	switch (format)
	{
	case BearSamplerAddressMode::Wrap:
		return D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		break;
	case BearSamplerAddressMode::Mirror:
		D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
		break;
	case BearSamplerAddressMode::Clamp:
		D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		break;
	case BearSamplerAddressMode::Border:
		D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		break;
	default:
		BEAR_CHECK(0);;;
	}
	return D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_WRAP;
}



D3D12_BLEND DX12Factory::Translation(BearBlendFactor format)
{
	switch (format)
	{
	case BearBlendFactor::Zero:
		return D3D12_BLEND::D3D12_BLEND_ZERO;
		break;
	case BearBlendFactor::One:
		return D3D12_BLEND::D3D12_BLEND_ONE;
		break;
	case BearBlendFactor::SrcColor:
		return D3D12_BLEND::D3D12_BLEND_SRC_COLOR;
		break;
	case BearBlendFactor::InvSrcColor:
		return D3D12_BLEND::D3D12_BLEND_INV_SRC_COLOR;
		break;
	case BearBlendFactor::SrcAlpha:
		return D3D12_BLEND::D3D12_BLEND_SRC_ALPHA;
		break;
	case BearBlendFactor::InvSrcAlpha:
		return D3D12_BLEND::D3D12_BLEND_INV_SRC_ALPHA;
		break;
	case BearBlendFactor::DestAlpha:
		return D3D12_BLEND::D3D12_BLEND_DEST_ALPHA;
		break;
	case BearBlendFactor::InvDestAlpha:
		return D3D12_BLEND::D3D12_BLEND_INV_DEST_ALPHA;
		break;
	case BearBlendFactor::DestColor:
		return D3D12_BLEND::D3D12_BLEND_DEST_COLOR;
		break;
	case BearBlendFactor::InvDestColor:
		return D3D12_BLEND::D3D12_BLEND_INV_DEST_COLOR;
		break;
	case BearBlendFactor::BlendFactor:
		return D3D12_BLEND::D3D12_BLEND_BLEND_FACTOR;
		break;
	case BearBlendFactor::InvBlendFactor:
		return D3D12_BLEND::D3D12_BLEND_INV_BLEND_FACTOR;
		break;
	default:
		BEAR_CHECK(0);;
	}
	return D3D12_BLEND_ZERO;
}

D3D12_BLEND_OP DX12Factory::Translation(BearBlendOp format)
{
	switch (format)
	{
	case BearBlendOp::Add:
		return D3D12_BLEND_OP::D3D12_BLEND_OP_ADD;
		break;
	case BearBlendOp::Subtract:
		return D3D12_BLEND_OP::D3D12_BLEND_OP_SUBTRACT;
		break;
	case BearBlendOp::RevSubtract:
		return D3D12_BLEND_OP::D3D12_BLEND_OP_REV_SUBTRACT;
		break;
	case BearBlendOp::Min:
		return D3D12_BLEND_OP::D3D12_BLEND_OP_MIN;
		break;
	case BearBlendOp::Max:
		return D3D12_BLEND_OP::D3D12_BLEND_OP_MAX;
		break;
	default:
		BEAR_CHECK(0);;
	}
	return D3D12_BLEND_OP::D3D12_BLEND_OP_ADD;
}

D3D12_COMPARISON_FUNC DX12Factory::Translation(BearCompareFunction format)
{
	switch (format)
	{
	case BearCompareFunction::Never:
		return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_ALWAYS;
		break;
	case BearCompareFunction::Always:
		return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_ALWAYS;
		break;
	case BearCompareFunction::Equal:
		return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_EQUAL;
		break;
	case BearCompareFunction::NotEqual:
		return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_NOT_EQUAL;
		break;
	case BearCompareFunction::Less:
		return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_LESS;
		break;
	case BearCompareFunction::Greater:
		return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_GREATER;
		break;
	case BearCompareFunction::LessEqual:
		return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_LESS_EQUAL;
		break;
	case BearCompareFunction::GreaterEqual:
		return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		break;
	default:
		BEAR_CHECK(0);;
	}
	return  D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_NEVER;
}

D3D12_STENCIL_OP DX12Factory::Translation(BearStencilOp format)
{
	switch (format)
	{
	case BearStencilOp::Keep:
		return D3D12_STENCIL_OP::D3D12_STENCIL_OP_KEEP;
		break;
	case BearStencilOp::Zero:
		return D3D12_STENCIL_OP::D3D12_STENCIL_OP_ZERO;
		break;
	case BearStencilOp::Replace:
		return D3D12_STENCIL_OP::D3D12_STENCIL_OP_REPLACE;
		break;
	case BearStencilOp::IncrSat:
		return D3D12_STENCIL_OP::D3D12_STENCIL_OP_INCR_SAT;
		break;
	case BearStencilOp::DecrSat:
		return D3D12_STENCIL_OP::D3D12_STENCIL_OP_DECR_SAT;
		break;
	case BearStencilOp::Invert:
		return D3D12_STENCIL_OP::D3D12_STENCIL_OP_INVERT;
		break;
	case BearStencilOp::Incr:
		return D3D12_STENCIL_OP::D3D12_STENCIL_OP_INCR;
		break;
	case BearStencilOp::Decr:
		return D3D12_STENCIL_OP::D3D12_STENCIL_OP_DECR;
		break;
	default:
		BEAR_CHECK(0);;
	}
	return  D3D12_STENCIL_OP::D3D12_STENCIL_OP_KEEP;
}


D3D12_CULL_MODE DX12Factory::Translation(BearRasterizerCullMode format)
{
	switch (format)
	{
	case BearRasterizerCullMode::Front:
		return D3D12_CULL_MODE::D3D12_CULL_MODE_FRONT;
		break;
	case BearRasterizerCullMode::Back:
		return D3D12_CULL_MODE::D3D12_CULL_MODE_BACK;
		break;
	case BearRasterizerCullMode::None:
		return D3D12_CULL_MODE::D3D12_CULL_MODE_NONE;
		break;
	default:
		BEAR_CHECK(0);;
	}
	return D3D12_CULL_MODE::D3D12_CULL_MODE_NONE;
}

D3D12_FILL_MODE DX12Factory::Translation(BearRasterizerFillMode format)
{
	switch (format)
	{
	case BearRasterizerFillMode::Wireframe:
		return D3D12_FILL_MODE::D3D12_FILL_MODE_WIREFRAME;
		break;
	case BearRasterizerFillMode::Solid:
		return D3D12_FILL_MODE::D3D12_FILL_MODE_SOLID;
		break;
	default:
		BEAR_CHECK(0);;
	}
	return D3D12_FILL_MODE::D3D12_FILL_MODE_SOLID;
}

DXGI_FORMAT DX12Factory::Translation(BearRenderTargetFormat format)
{
	switch (format)
	{
	case BearRenderTargetFormat::None:
		BEAR_CHECK(0);
		break;
	case BearRenderTargetFormat::R8:
		return DXGI_FORMAT_R8_UNORM;
		break;
	case BearRenderTargetFormat::R8G8:
		return DXGI_FORMAT_R8G8_UNORM;
		break;

	case BearRenderTargetFormat::R8G8B8A8:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
		break;
	case BearRenderTargetFormat::B8G8R8A8:
		return DXGI_FORMAT_B8G8R8A8_UNORM;
		break;
	case BearRenderTargetFormat::R32F:
		return DXGI_FORMAT_R32_FLOAT;
		break;
	case BearRenderTargetFormat::R32G32F:
		return DXGI_FORMAT_R32G32_FLOAT;
		break;
	case BearRenderTargetFormat::R32G32B32F:
		return DXGI_FORMAT_R32G32B32_FLOAT;
		break;
	case BearRenderTargetFormat::R32G32B32A32F:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
		break;
	default:
		BEAR_CHECK(0);
		break;
	}
	return DXGI_FORMAT_R32G32B32A32_FLOAT;
}

DXGI_FORMAT DX12Factory::Translation(BearDepthStencilFormat format)
{
	switch (format)
	{
	case BearDepthStencilFormat::Depth16:
		return DXGI_FORMAT_D16_UNORM;
		break;
	case BearDepthStencilFormat::Depth32F:
		return DXGI_FORMAT_D32_FLOAT;
		break;
	case BearDepthStencilFormat::Depth24Stencil8:
		return DXGI_FORMAT_D24_UNORM_S8_UINT;
		break;
	case BearDepthStencilFormat::Depth32FStencil8:
		return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		break;
	default:
		BEAR_CHECK(0);
		break;
	}
	return DXGI_FORMAT_D24_UNORM_S8_UINT;
}

DXGI_FORMAT DX12Factory::TranslationForRayTracing(BearVertexFormat format)
{
	switch (format)
	{
	case BearVertexFormat::R16G16_FLOAT:
		return DXGI_FORMAT_R16G16_FLOAT;
		break;
	case BearVertexFormat::R16G16B16A16_FLOAT:
		return DXGI_FORMAT_R16G16B16A16_FLOAT;
		break;
	case BearVertexFormat::R32G32_FLOAT:
		return DXGI_FORMAT_R32G32_FLOAT;
		break;
	case BearVertexFormat::R32G32B32_FLOAT:
		return DXGI_FORMAT_R32G32B32_FLOAT;
		break;
	default:
		BEAR_CHECK(false);
		return DXGI_FORMAT_R16G16_FLOAT;
		break;
	}
}

DXGI_FORMAT DX12Factory::Translation(BearVertexFormat format)
{

	switch (format)
	{
	case BearVertexFormat::R16G16_SINT:
		return DXGI_FORMAT_R16G16_SINT;
	case BearVertexFormat::R16G16B16A16_SINT:
		return DXGI_FORMAT_R16G16B16A16_SINT;
	case BearVertexFormat::R16G16_FLOAT:
		return DXGI_FORMAT_R16G16_FLOAT;
	case BearVertexFormat::R16G16B16A16_FLOAT:
		return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case BearVertexFormat::R32G32B32A32_FLOAT:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case BearVertexFormat::R32G32B32_FLOAT:
		return DXGI_FORMAT_R32G32B32_FLOAT;
	case BearVertexFormat::R32G32_FLOAT:
		return DXGI_FORMAT_R32G32_FLOAT;

	case BearVertexFormat::R32_FLOAT:
		return DXGI_FORMAT_R32_FLOAT;

	case BearVertexFormat::R32_INT:
		return DXGI_FORMAT_R32_SINT;
	case BearVertexFormat::R8G8B8A8:
		return DXGI_FORMAT_R8G8B8A8_UINT;
	case BearVertexFormat::R8G8:
		return DXGI_FORMAT_R8G8_UINT;
	case BearVertexFormat::R8:
		return DXGI_FORMAT_R8_UINT;
	default:
		BEAR_CHECK(0);;
		return DXGI_FORMAT_UNKNOWN;
	}

}

void DX12Factory::Translation(BearTopologyType format, D3D_PRIMITIVE_TOPOLOGY& topology, D3D12_PRIMITIVE_TOPOLOGY_TYPE& topology_type)
{
	switch (format)
	{
	case    BearTopologyType::PointList:
		topology_type = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
		topology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
		break;
	case	  BearTopologyType::LintList:
		topology_type = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		topology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		break;
	case	  BearTopologyType::LineStrip:
		topology_type = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		topology = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
		break;
	case	  BearTopologyType::TriangleList:
		topology_type = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		break;
	case	  BearTopologyType::TriangleStrip:
		topology_type = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		break;
	default:
		BEAR_CHECK(false);
		topology_type = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		break;
	}
}

