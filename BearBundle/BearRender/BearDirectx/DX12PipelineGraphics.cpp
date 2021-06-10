#include "DX12PCH.h"
#include "DX12PipelineGraphics.h"
bsize PipelineGraphicsCounter = 0;
DX12PipelineGraphics::DX12PipelineGraphics(const BearPipelineGraphicsDescription & description)
{
	PipelineGraphicsCounter++;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC Desc = {};
	D3D12_INPUT_ELEMENT_DESC Elemets[16];
	{
		bsize Count = 0;
		
		for (bsize i = 0; i < 16 && !description.InputLayout.Elements[i].empty(); i++)
		{
			auto& cElement = description.InputLayout.Elements[i];
			Elemets[i].Format = DX12Factory::Translation(cElement.Type);
			Elemets[i].SemanticName = *description.InputLayout.Elements[i].Name;
			Elemets[i].InputSlot = 0;

			Elemets[i].AlignedByteOffset = static_cast<UINT>(description.InputLayout.Elements[i].Offset);
			Elemets[i].InputSlotClass = description.InputLayout.Elements[i].IsInstance ? D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA : D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			Elemets[i].InstanceDataStepRate = description.InputLayout.Elements[i].IsInstance ? 1 : 0;
			Elemets[i].SemanticIndex = static_cast<UINT>(description.InputLayout.Elements[i].SemanticIndex);
			Count++;
		}
		Desc.InputLayout.pInputElementDescs = Elemets;
		Desc.InputLayout.NumElements =static_cast<UINT>(Count);
	};
	{
		auto VertexShader = const_cast<DX12Shader*>(static_cast<const DX12Shader*>(description.Shaders.Vertex.get()));
		if (VertexShader && VertexShader->IsType(BearShaderType::Vertex))
			Desc.VS = CD3DX12_SHADER_BYTECODE(VertexShader->GetPointer(), VertexShader->GetSize());

		auto HullShader = const_cast<DX12Shader*>(static_cast<const DX12Shader*>(description.Shaders.Vertex.get()));
		if (HullShader && HullShader->IsType(BearShaderType::Hull))
			Desc.HS = CD3DX12_SHADER_BYTECODE(HullShader->GetPointer(), HullShader->GetSize());

		auto DomainShader = const_cast<DX12Shader*>(static_cast<const DX12Shader*>(description.Shaders.Vertex.get()));
		if (DomainShader && DomainShader->IsType(BearShaderType::Domain))
			Desc.DS = CD3DX12_SHADER_BYTECODE(DomainShader->GetPointer(), DomainShader->GetSize());

		auto GeometryShader = const_cast<DX12Shader*>(static_cast<const DX12Shader*>(description.Shaders.Vertex.get()));
		if (GeometryShader && GeometryShader->IsType(BearShaderType::Geometry))
			Desc.GS = CD3DX12_SHADER_BYTECODE(GeometryShader->GetPointer(), GeometryShader->GetSize());

		auto PixelShader = const_cast<DX12Shader*>(static_cast<const DX12Shader*>(description.Shaders.Pixel.get()));
		if(PixelShader && PixelShader->IsType(BearShaderType::Pixel))
			Desc.PS = CD3DX12_SHADER_BYTECODE(PixelShader->GetPointer(), PixelShader->GetSize());

	}
	Desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	{
		ZeroMemory(&Desc.RasterizerState, sizeof(D3D12_RASTERIZER_DESC));
		Desc.RasterizerState.AntialiasedLineEnable = false;
		Desc.RasterizerState.FrontCounterClockwise = false;
		Desc.RasterizerState.MultisampleEnable = false;

		Desc.RasterizerState.CullMode = DX12Factory::Translation(description.RasterizerState.CullMode);
		Desc.RasterizerState.FillMode = DX12Factory::Translation(description.RasterizerState.FillMode);;
		Desc.RasterizerState.FrontCounterClockwise = description.RasterizerState.FrontFace == BearRasterizerFrontFace::CounterClockwise;


		Desc.RasterizerState.SlopeScaledDepthBias = description.RasterizerState.DepthBiasEnable ? description.RasterizerState.DepthSlopeScaleBias:0;
		Desc.RasterizerState.DepthBias = description.RasterizerState.DepthBiasEnable ? (int)(description.RasterizerState.DepthBias * (float)(1 << 24)):0;

		Desc.RasterizerState.DepthBiasClamp = description.RasterizerState.DepthClampEnable ? description.RasterizerState.DepthClmapBias : 0.f;
	}
	Desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	{
		Desc.BlendState.IndependentBlendEnable = description.BlendState.IndependentBlendEnable;
		Desc.BlendState.AlphaToCoverageEnable = description.MultisampleState.AlphaToCoverageEnable;
		for (bsize i = 0; i < 8; i++)
		{
			Desc.BlendState.RenderTarget[i].BlendEnable = description.BlendState.RenderTarget[i].Enable;
			Desc.BlendState.RenderTarget[i].BlendOp = DX12Factory::Translation(description.BlendState.RenderTarget[i].Color);
			Desc.BlendState.RenderTarget[i].BlendOpAlpha = DX12Factory::Translation(description.BlendState.RenderTarget[i].Alpha);
			Desc.BlendState.RenderTarget[i].SrcBlend = DX12Factory::Translation(description.BlendState.RenderTarget[i].ColorSrc);
			Desc.BlendState.RenderTarget[i].DestBlend = DX12Factory::Translation(description.BlendState.RenderTarget[i].ColorDst);
			Desc.BlendState.RenderTarget[i].SrcBlendAlpha = DX12Factory::Translation(description.BlendState.RenderTarget[i].AlphaSrc);
			Desc.BlendState.RenderTarget[i].DestBlendAlpha = DX12Factory::Translation(description.BlendState.RenderTarget[i].AlphaDst);
			if (description.BlendState.RenderTarget[i].ColorWriteMask .test((int32)BearColorWriteMask::R))
				Desc.BlendState.RenderTarget[i].RenderTargetWriteMask |= D3D12_COLOR_WRITE_ENABLE_RED;
			if (description.BlendState.RenderTarget[i].ColorWriteMask.test((int32)BearColorWriteMask::G))
				Desc.BlendState.RenderTarget[i].RenderTargetWriteMask |= D3D12_COLOR_WRITE_ENABLE_GREEN;
			if (description.BlendState.RenderTarget[i].ColorWriteMask.test((int32)BearColorWriteMask::B))
				Desc.BlendState.RenderTarget[i].RenderTargetWriteMask |= D3D12_COLOR_WRITE_ENABLE_BLUE;
			if (description.BlendState.RenderTarget[i].ColorWriteMask.test((int32)BearColorWriteMask::A))
				Desc.BlendState.RenderTarget[i].RenderTargetWriteMask |= D3D12_COLOR_WRITE_ENABLE_ALPHA;
		}

	}
	{

		ZeroMemory(&Desc.DepthStencilState, sizeof(D3D12_DEPTH_STENCIL_DESC));
		Desc.DepthStencilState.DepthEnable = description.DepthStencilState.DepthEnable;
		Desc.DepthStencilState.DepthFunc = DX12Factory::Translation(description.DepthStencilState.DepthTest);
		Desc.DepthStencilState.DepthWriteMask = description.DepthStencilState.EnableDepthWrite ? D3D12_DEPTH_WRITE_MASK::D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK::D3D12_DEPTH_WRITE_MASK_ZERO;
		Desc.DepthStencilState.StencilEnable = description.DepthStencilState.StencillEnable;
		Desc.DepthStencilState.StencilReadMask = description.DepthStencilState.StencilReadMask;
		Desc.DepthStencilState.StencilWriteMask = description.DepthStencilState.StencilWriteMask;
		Desc.DepthStencilState.FrontFace.StencilDepthFailOp = DX12Factory::Translation(description.DepthStencilState.FrontFace.StencilDepthFailOp);
		Desc.DepthStencilState.FrontFace.StencilFailOp = DX12Factory::Translation(description.DepthStencilState.FrontFace.StencilFailOp);
		Desc.DepthStencilState.FrontFace.StencilPassOp = DX12Factory::Translation(description.DepthStencilState.FrontFace.StencilPassOp);
		Desc.DepthStencilState.FrontFace.StencilFunc = DX12Factory::Translation(description.DepthStencilState.FrontFace.StencilTest);
		if (description.DepthStencilState.BackStencillEnable)
		{
			Desc.DepthStencilState.BackFace.StencilDepthFailOp = DX12Factory::Translation(description.DepthStencilState.BackFace.StencilDepthFailOp);
			Desc.DepthStencilState.BackFace.StencilFailOp = DX12Factory::Translation(description.DepthStencilState.BackFace.StencilFailOp);
			Desc.DepthStencilState.BackFace.StencilPassOp = DX12Factory::Translation(description.DepthStencilState.BackFace.StencilPassOp);
			Desc.DepthStencilState.BackFace.StencilFunc = DX12Factory::Translation(description.DepthStencilState.BackFace.StencilTest);
		}
		else
		{
			Desc.DepthStencilState.BackFace = Desc.DepthStencilState.FrontFace;
		}

	}
	Desc.SampleMask = UINT_MAX;
	DX12Factory::Translation(description.TopologyType, TopologyType, Desc.PrimitiveTopologyType);
	

	if (description.RenderPass.empty())
	{
		Desc.NumRenderTargets = 1;
		Desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

	}
	else
	{
		const DX12RenderPass* RenderPass = static_cast<const DX12RenderPass*>(description.RenderPass.get());
		Desc.NumRenderTargets =  static_cast<UINT>(RenderPass->CountRenderTarget);
		for (bsize i = 0; i < Desc.NumRenderTargets; i++)
		{
			Desc.RTVFormats[i] = DX12Factory::Translation(RenderPass->Description.RenderTargets[i].Format);

			
		}
		if (RenderPass->Description.DepthStencil.Format != BearDepthStencilFormat::None)
		{
			Desc.DSVFormat = DX12Factory::Translation(RenderPass->Description.DepthStencil.Format);
		}
	}

	
	Desc.SampleDesc.Count = 1;


	RootSignature = description.RootSignature;
	BEAR_CHECK(RootSignature.empty() == false);
	RootSignaturePointer = static_cast<DX12RootSignature*>(RootSignature.get());
	Desc.pRootSignature = RootSignaturePointer->RootSignature.Get();


	R_CHK(Factory->Device->CreateGraphicsPipelineState(&Desc, IID_PPV_ARGS(&PipelineState)));
}

DX12PipelineGraphics::~DX12PipelineGraphics()
{
	PipelineGraphicsCounter--;
}

void* DX12PipelineGraphics::QueryInterface(int type)
{
	switch (type)
	{
	case DX12Q_Pipeline:
		return reinterpret_cast<void*>(static_cast<DX12Pipeline*>(this));
	default:
		return nullptr;
	}
}

BearPipelineType DX12PipelineGraphics::GetType()
{
	return BearPipelineType::Graphics;
}

void DX12PipelineGraphics::Set(ID3D12GraphicsCommandListX* command_list)
{
	command_list->SetPipelineState(PipelineState.Get());
	command_list->IASetPrimitiveTopology(TopologyType);
	RootSignaturePointer->Set(command_list);
}
