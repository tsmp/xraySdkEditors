#include "DX12PCH.h"
bsize PipelineMeshCounter = 0;
#ifdef MESH_SHADING
namespace
{
	typedef CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT< D3D12_SHADER_BYTECODE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_AS> CD3DX12_PIPELINE_STATE_STREAM_AS;
	typedef CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT< D3D12_SHADER_BYTECODE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_MS> CD3DX12_PIPELINE_STATE_STREAM_MS;

	struct MeshShaderPsoDesc
	{
		CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE        pRootSignature;
		CD3DX12_PIPELINE_STATE_STREAM_AS                    AS;
		CD3DX12_PIPELINE_STATE_STREAM_MS                    MS;
		CD3DX12_PIPELINE_STATE_STREAM_PS                    PS;
		CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC            BlendState;
		CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL         DepthStencilState;
		CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT  DepthFormat;
		CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER            RasterizerState;
		CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTFormats;
		CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC           SampleDesc;
		CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_MASK           SampleMask;
	};
}
DX12PipelineMesh::DX12PipelineMesh(const BearPipelineMeshDescription & description)
{
	PipelineMeshCounter++;
	MeshShaderPsoDesc Desc = {};

	{
		auto AmplificationShader = const_cast<DX12Shader*>(static_cast<const DX12Shader*>(description.Shaders.Amplification.get()));
		if (AmplificationShader && AmplificationShader->IsType(BearShaderType::Amplification))
			Desc.AS = CD3DX12_SHADER_BYTECODE(AmplificationShader->GetPointer(), AmplificationShader->GetSize());

		auto MeshSahder = const_cast<DX12Shader*>(static_cast<const DX12Shader*>(description.Shaders.Mesh.get()));
		if (MeshSahder && MeshSahder->IsType(BearShaderType::Mesh))
			Desc.MS = CD3DX12_SHADER_BYTECODE(MeshSahder->GetPointer(), MeshSahder->GetSize());

		auto PixelShader = const_cast<DX12Shader*>(static_cast<const DX12Shader*>(description.Shaders.Pixel.get()));
		if(PixelShader&&PixelShader->IsType(BearShaderType::Pixel))
			Desc.PS = CD3DX12_SHADER_BYTECODE(PixelShader->GetPointer(), PixelShader->GetSize());

	}
	Desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	{
		CD3DX12_RASTERIZER_DESC RasterizerState = CD3DX12_RASTERIZER_DESC(CD3DX12_DEFAULT());
		RasterizerState.AntialiasedLineEnable = false;
		RasterizerState.FrontCounterClockwise = false;
		RasterizerState.MultisampleEnable = false;

		RasterizerState.CullMode = DX12Factory::Translation(description.RasterizerState.CullMode);
		RasterizerState.FillMode = DX12Factory::Translation(description.RasterizerState.FillMode);;
		RasterizerState.FrontCounterClockwise = description.RasterizerState.FrontFace == BearRasterizerFrontFace::CounterClockwise;


		RasterizerState.SlopeScaledDepthBias = description.RasterizerState.DepthBiasEnable ? description.RasterizerState.DepthSlopeScaleBias:0;
		RasterizerState.DepthBias = description.RasterizerState.DepthBiasEnable ? (int)(description.RasterizerState.DepthBias * (float)(1 << 24)):0;

		RasterizerState.DepthBiasClamp = description.RasterizerState.DepthClampEnable ? description.RasterizerState.DepthClmapBias : 0.f;
		Desc.RasterizerState = RasterizerState;
	}
	{
		CD3DX12_BLEND_DESC BlendState = CD3DX12_BLEND_DESC(CD3DX12_DEFAULT());
		BlendState.IndependentBlendEnable = description.BlendState.IndependentBlendEnable;
		BlendState.AlphaToCoverageEnable = description.MultisampleState.AlphaToCoverageEnable;
		for (bsize i = 0; i < 8; i++)
		{
			BlendState.RenderTarget[i].BlendEnable = description.BlendState.RenderTarget[i].Enable;
			BlendState.RenderTarget[i].BlendOp = DX12Factory::Translation(description.BlendState.RenderTarget[i].Color);
			BlendState.RenderTarget[i].BlendOpAlpha = DX12Factory::Translation(description.BlendState.RenderTarget[i].Alpha);
			BlendState.RenderTarget[i].SrcBlend = DX12Factory::Translation(description.BlendState.RenderTarget[i].ColorSrc);
			BlendState.RenderTarget[i].DestBlend = DX12Factory::Translation(description.BlendState.RenderTarget[i].ColorDst);
			BlendState.RenderTarget[i].SrcBlendAlpha = DX12Factory::Translation(description.BlendState.RenderTarget[i].AlphaSrc);
			BlendState.RenderTarget[i].DestBlendAlpha = DX12Factory::Translation(description.BlendState.RenderTarget[i].AlphaDst);
			if (description.BlendState.RenderTarget[i].ColorWriteMask.test((int32)BearColorWriteMask::R))
				BlendState.RenderTarget[i].RenderTargetWriteMask |= D3D12_COLOR_WRITE_ENABLE_RED;
			if (description.BlendState.RenderTarget[i].ColorWriteMask.test((int32)BearColorWriteMask::G))
				BlendState.RenderTarget[i].RenderTargetWriteMask |= D3D12_COLOR_WRITE_ENABLE_GREEN;
			if (description.BlendState.RenderTarget[i].ColorWriteMask.test((int32)BearColorWriteMask::B))
				BlendState.RenderTarget[i].RenderTargetWriteMask |= D3D12_COLOR_WRITE_ENABLE_BLUE;
			if (description.BlendState.RenderTarget[i].ColorWriteMask.test((int32)BearColorWriteMask::A))
				BlendState.RenderTarget[i].RenderTargetWriteMask |= D3D12_COLOR_WRITE_ENABLE_ALPHA;
		}
		Desc.BlendState = BlendState;

	}
	{
		CD3DX12_DEPTH_STENCIL_DESC DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(CD3DX12_DEFAULT());
		DepthStencilState.DepthEnable = description.DepthStencilState.DepthEnable;
		DepthStencilState.DepthFunc = DX12Factory::Translation(description.DepthStencilState.DepthTest);
		DepthStencilState.DepthWriteMask = description.DepthStencilState.EnableDepthWrite ? D3D12_DEPTH_WRITE_MASK::D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK::D3D12_DEPTH_WRITE_MASK_ZERO;
		DepthStencilState.StencilEnable = description.DepthStencilState.StencillEnable;
		DepthStencilState.StencilReadMask = description.DepthStencilState.StencilReadMask;
		DepthStencilState.StencilWriteMask = description.DepthStencilState.StencilWriteMask;
		DepthStencilState.FrontFace.StencilDepthFailOp = DX12Factory::Translation(description.DepthStencilState.FrontFace.StencilDepthFailOp);
		DepthStencilState.FrontFace.StencilFailOp = DX12Factory::Translation(description.DepthStencilState.FrontFace.StencilFailOp);
		DepthStencilState.FrontFace.StencilPassOp = DX12Factory::Translation(description.DepthStencilState.FrontFace.StencilPassOp);
		DepthStencilState.FrontFace.StencilFunc = DX12Factory::Translation(description.DepthStencilState.FrontFace.StencilTest);
		if (description.DepthStencilState.BackStencillEnable)
		{
			DepthStencilState.BackFace.StencilDepthFailOp = DX12Factory::Translation(description.DepthStencilState.BackFace.StencilDepthFailOp);
			DepthStencilState.BackFace.StencilFailOp = DX12Factory::Translation(description.DepthStencilState.BackFace.StencilFailOp);
			DepthStencilState.BackFace.StencilPassOp = DX12Factory::Translation(description.DepthStencilState.BackFace.StencilPassOp);
			DepthStencilState.BackFace.StencilFunc = DX12Factory::Translation(description.DepthStencilState.BackFace.StencilTest);
		}
		else
		{
			DepthStencilState.BackFace = DepthStencilState.FrontFace;
		}
		Desc.DepthStencilState = DepthStencilState;
	}
	Desc.SampleMask = UINT_MAX;
	
	Desc.DepthFormat = DXGI_FORMAT_UNKNOWN;
	if (description.RenderPass.empty())
	{
		D3D12_RT_FORMAT_ARRAY DescRT;
		DescRT.NumRenderTargets = 1;
		DescRT.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		Desc.RTFormats = DescRT;
	}
	else
	{
		D3D12_RT_FORMAT_ARRAY DescRT;
		const DX12RenderPass* RenderPass = static_cast<const DX12RenderPass*>(description.RenderPass.get());
		DescRT.NumRenderTargets =  static_cast<UINT>(RenderPass->CountRenderTarget);
		for (bsize i = 0; i < DescRT.NumRenderTargets; i++)
		{
			DescRT.RTFormats[i] = DX12Factory::Translation(RenderPass->Description.RenderTargets[i].Format);
		}
		for (bsize i = DescRT.NumRenderTargets; i < 8; i++)
		{
			DescRT.RTFormats[i] = DXGI_FORMAT_UNKNOWN;
		}
		if (RenderPass->Description.DepthStencil.Format != BearDepthStencilFormat::None)
		{
			Desc.DepthFormat = DX12Factory::Translation(RenderPass->Description.DepthStencil.Format);
		}
		Desc.RTFormats = DescRT;
	}

	{
		DXGI_SAMPLE_DESC SampleDesc;
		SampleDesc.Count = 1;
		SampleDesc.Quality = 0;
		Desc.SampleDesc = SampleDesc;
	}


	RootSignature = description.RootSignature;
	BEAR_CHECK(RootSignature.empty() == false);
	RootSignaturePointer = static_cast<DX12RootSignature*>(RootSignature.get());
	Desc.pRootSignature = RootSignaturePointer->RootSignature.Get();

	D3D12_PIPELINE_STATE_STREAM_DESC StreamDesc;
	StreamDesc.pPipelineStateSubobjectStream = &Desc;
	StreamDesc.SizeInBytes = sizeof(Desc);

	R_CHK(Factory->Device->CreatePipelineState(&StreamDesc, IID_PPV_ARGS(&PipelineState)));
}

DX12PipelineMesh::~DX12PipelineMesh()
{
	PipelineMeshCounter--;
}

void* DX12PipelineMesh::QueryInterface(int type)
{
	switch (type)
	{
	case DX12Q_Pipeline:
		return reinterpret_cast<void*>(static_cast<DX12Pipeline*>(this));
	default:
		return nullptr;
	}
}

BearPipelineType DX12PipelineMesh::GetType()
{
	return BearPipelineType::Mesh;
}

void DX12PipelineMesh::Set(	ID3D12GraphicsCommandListX* command_list)
{
	command_list->SetPipelineState(PipelineState.Get());
	command_list->IASetPrimitiveTopology(TopologyType);
	RootSignaturePointer->Set(command_list);
}

#endif