#include "VKPCH.h"
size_t PipelineGraphicsCounter = 0;


VKPipelineGraphics::VKPipelineGraphics(const BearPipelineGraphicsDescription& description)
{
	PipelineGraphicsCounter++;
	VkDynamicState DynamicStateEnables[16] = {};
	VkPipelineDynamicStateCreateInfo DynamicState = {};
	{
		memset(DynamicStateEnables, 0, sizeof(DynamicStateEnables));
		DynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		DynamicState.pNext = NULL;
		DynamicState.pDynamicStates = DynamicStateEnables;
		DynamicState.dynamicStateCount = 0;
	}

	VkPipelineVertexInputStateCreateInfo VertexInputState = {};
	VkVertexInputBindingDescription VertexInputBinding[2] = {};
	VkVertexInputAttributeDescription   VertexInputAttribute[16] = {};
	{
		size_t  VertexInputCount = 0;
		{
			size_t Stride = 0;
			VertexInputBinding[0].binding = 0;
			VertexInputBinding[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;


			VertexInputBinding[1].binding = 1;
			VertexInputBinding[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

			for (size_t i = 0; i < 16 && !description.InputLayout.Elements[i].empty(); i++)
			{
				auto& cElement = description.InputLayout.Elements[i];
				VertexInputAttribute[i].offset = static_cast<UINT>(description.InputLayout.Elements[i].Offset);;
				VertexInputAttribute[i].location = static_cast<uint32>(i);
				VertexInputAttribute[i].format = VKFactory::Translation(cElement.Type);
				VertexInputAttribute[i].binding = description.InputLayout.Elements[i].IsInstance ? 1 : 0;
				Stride = BearMath::max(VKFactory::TranslationInSize(cElement.Type) + static_cast<size_t>(VertexInputAttribute[i].offset), Stride);
				VertexInputCount++;
			}
			VertexInputBinding[0].stride = static_cast<uint32>(Stride);
			VertexInputBinding[1].stride = static_cast<uint32>(Stride);;
		}

		VertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		VertexInputState.pNext = NULL;
		VertexInputState.flags = 0;
		VertexInputState.vertexBindingDescriptionCount = 1;
		VertexInputState.pVertexBindingDescriptions = VertexInputBinding;
		VertexInputState.vertexAttributeDescriptionCount = static_cast<uint32>(VertexInputCount); ;
		VertexInputState.pVertexAttributeDescriptions = VertexInputAttribute;
	}

	VkPipelineInputAssemblyStateCreateInfo InputAssemblyState = {};
	{
		InputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		InputAssemblyState.pNext = NULL;
		InputAssemblyState.flags = 0;
		InputAssemblyState.primitiveRestartEnable = VK_FALSE;
		InputAssemblyState.topology = VKFactory::Translation(description.TopologyType);

	}

	VkPipelineRasterizationStateCreateInfo RasterizationState = {};
	{
		RasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		RasterizationState.pNext = NULL;
		RasterizationState.flags = 0;
		RasterizationState.polygonMode = VKFactory::Translation(description.RasterizerState.FillMode);
		RasterizationState.cullMode = VKFactory::Translation(description.RasterizerState.CullMode);
		RasterizationState.frontFace = VKFactory::Translation(description.RasterizerState.FrontFace);

		RasterizationState.rasterizerDiscardEnable = VK_FALSE;// description.RasterizerState.RasterizerDiscard;

		RasterizationState.depthClampEnable = description.RasterizerState.DepthClampEnable;
		RasterizationState.depthBiasClamp = description.RasterizerState.DepthClmapBias;

		RasterizationState.depthBiasEnable = description.RasterizerState.DepthBiasEnable;
		RasterizationState.depthBiasConstantFactor = description.RasterizerState.DepthBias;
		RasterizationState.depthBiasSlopeFactor = description.RasterizerState.DepthSlopeScaleBias;


		RasterizationState.lineWidth = 1.0f;
	}

	VkPipelineColorBlendStateCreateInfo BlendState = {};
	VkPipelineColorBlendAttachmentState BlendAttachment[8] = {};
	{
		BlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		BlendState.pNext = NULL;
		BlendState.flags = 0;
		for (size_t i = 0; i < 8; i++)
		{
			size_t index = description.BlendState.IndependentBlendEnable? i:0;
			BlendAttachment[i].blendEnable = description.BlendState.RenderTarget[index].Enable;
			BlendAttachment[i].alphaBlendOp = VKFactory::Translation(description.BlendState.RenderTarget[index].Alpha);
			BlendAttachment[i].colorBlendOp = VKFactory::Translation(description.BlendState.RenderTarget[index].Color);
			BlendAttachment[i].srcColorBlendFactor = VKFactory::Translation(description.BlendState.RenderTarget[index].ColorSrc);
			BlendAttachment[i].dstColorBlendFactor = VKFactory::Translation(description.BlendState.RenderTarget[index].ColorDst);
			BlendAttachment[i].srcAlphaBlendFactor = VKFactory::Translation(description.BlendState.RenderTarget[index].AlphaSrc);
			BlendAttachment[i].dstAlphaBlendFactor = VKFactory::Translation(description.BlendState.RenderTarget[index].AlphaDst);
			BlendAttachment[i].colorWriteMask = 0;
			BlendAttachment[i].colorWriteMask = VKFactory::Translation(description.BlendState.RenderTarget[index].ColorWriteMask);
		}
	
		
		if (description.RenderPass.empty())
			BlendState.attachmentCount = 1;
		else
			BlendState.attachmentCount =static_cast<uint32_t>( static_cast<const VKRenderPass*>(description.RenderPass.get())->CountRenderTarget);
		BlendState.pAttachments = BlendAttachment;
		BlendState.logicOpEnable = VK_FALSE;
		BlendState.logicOp = VK_LOGIC_OP_NO_OP;
		BlendState.blendConstants[0] = 1.0f;
		BlendState.blendConstants[1] = 1.0f;
		BlendState.blendConstants[2] = 1.0f;
		BlendState.blendConstants[3] = 1.0f;
	}


	VkPipelineViewportStateCreateInfo ViewportState = {};
	{
		ViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		ViewportState.pNext = NULL;
		ViewportState.flags = 0;
		ViewportState.viewportCount = 1;
		DynamicStateEnables[DynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
		ViewportState.scissorCount = 1;
		DynamicStateEnables[DynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;
		ViewportState.pScissors = NULL;
		ViewportState.pViewports = NULL;
	}
	DynamicStateEnables[DynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_STENCIL_REFERENCE;
	VkPipelineDepthStencilStateCreateInfo DepthStencilState = {};
	{
		DepthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		DepthStencilState.pNext = NULL;
		DepthStencilState.flags = 0;
		DepthStencilState.depthTestEnable = description.DepthStencilState.DepthEnable;
		DepthStencilState.depthWriteEnable = description.DepthStencilState.EnableDepthWrite;
		DepthStencilState.depthCompareOp = VKFactory::Translation(description.DepthStencilState.DepthTest);
		DepthStencilState.depthBoundsTestEnable = VK_FALSE;
		DepthStencilState.minDepthBounds = 0;
		DepthStencilState.maxDepthBounds = 0;
		DepthStencilState.stencilTestEnable = description.DepthStencilState.StencillEnable;

		if (!description.DepthStencilState.BackStencillEnable)
		{
			DepthStencilState.front.failOp = VKFactory::Translation(description.DepthStencilState.FrontFace.StencilFailOp);
			DepthStencilState.front.passOp = VKFactory::Translation(description.DepthStencilState.FrontFace.StencilPassOp);
			DepthStencilState.front.compareOp = VKFactory::Translation(description.DepthStencilState.FrontFace.StencilTest);
			DepthStencilState.front.compareMask = description.DepthStencilState.StencilReadMask;
			DepthStencilState.front.reference = 0;
			DepthStencilState.front.depthFailOp = VKFactory::Translation(description.DepthStencilState.FrontFace.StencilDepthFailOp);;
			DepthStencilState.front.writeMask = description.DepthStencilState.StencilWriteMask;;
			DepthStencilState.back = DepthStencilState.front;
		}
		else
		{
			DepthStencilState.back.failOp = VKFactory::Translation(description.DepthStencilState.BackFace.StencilFailOp);
			DepthStencilState.back.passOp = VKFactory::Translation(description.DepthStencilState.BackFace.StencilPassOp);
			DepthStencilState.back.compareOp = VKFactory::Translation(description.DepthStencilState.BackFace.StencilTest);
			DepthStencilState.back.compareMask = description.DepthStencilState.StencilReadMask;
			DepthStencilState.back.reference = 0;
			DepthStencilState.back.depthFailOp = VKFactory::Translation(description.DepthStencilState.BackFace.StencilDepthFailOp);;
			DepthStencilState.back.writeMask = description.DepthStencilState.StencilWriteMask;;
		}
	}

	VkPipelineMultisampleStateCreateInfo MultisampleState = {};
	{
		MultisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		MultisampleState.pNext = NULL;
		MultisampleState.flags = 0;
		MultisampleState.pSampleMask = NULL;
		MultisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		MultisampleState.sampleShadingEnable = description.MultisampleState.MultisampleEnable;
		MultisampleState.alphaToCoverageEnable = description.MultisampleState.AlphaToCoverageEnable;
		MultisampleState.alphaToOneEnable = VK_FALSE;
		MultisampleState.minSampleShading = 0.0;
	}
	VkPipelineShaderStageCreateInfo ShaderStage[6];
	size_t CountShader = 0;
	{
	
		auto VertexShader = static_cast<const VKShader*>(description.Shaders.Vertex.get());
		if (VertexShader && VertexShader->IsType(BearShaderType::Vertex))
		{
			bear_copy(&ShaderStage[CountShader], &VertexShader->Shader, 1);
			CountShader++;
		}
		auto PixelShader = static_cast<const VKShader*>(description.Shaders.Pixel.get());
		if (PixelShader && PixelShader->IsType(BearShaderType::Pixel))
		{
			bear_copy(&ShaderStage[CountShader], &PixelShader->Shader, 1);
			CountShader++;
		}
		auto DomainShader = static_cast<const VKShader*>(description.Shaders.Domain.get());
		if (DomainShader && DomainShader->IsType(BearShaderType::Domain))
		{
			bear_copy(&ShaderStage[CountShader], &DomainShader->Shader, 1);
			CountShader++;
		}
		auto HullShader = static_cast<const VKShader*>(description.Shaders.Hull.get());
		if (HullShader && HullShader->IsType(BearShaderType::Hull))
		{
			bear_copy(&ShaderStage[CountShader], &HullShader->Shader, 1);
			CountShader++;
		}
		auto GeometryShader = static_cast<const VKShader*>(description.Shaders.Geometry.get());
		if (GeometryShader && GeometryShader->IsType(BearShaderType::Hull))
		{
			bear_copy(&ShaderStage[CountShader], &GeometryShader->Shader, 1);
			CountShader++;
		}
	}
	RootSignature = description.RootSignature;

	BEAR_CHECK(RootSignature.empty() == false);
	RootSignaturePointer = static_cast<VKRootSignature*>(RootSignature.get());
	VkGraphicsPipelineCreateInfo GraphicsPipelineCreateInfo = {};
	GraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	GraphicsPipelineCreateInfo.pNext = NULL;
	GraphicsPipelineCreateInfo.layout = RootSignaturePointer->PipelineLayout;// RootSignaturePointer->PipelineLayout;
	GraphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	GraphicsPipelineCreateInfo.basePipelineIndex = 0;
	GraphicsPipelineCreateInfo.flags = 0;
	GraphicsPipelineCreateInfo.pVertexInputState = &VertexInputState;
	GraphicsPipelineCreateInfo.pInputAssemblyState = &InputAssemblyState;
	GraphicsPipelineCreateInfo.pRasterizationState = &RasterizationState;
	GraphicsPipelineCreateInfo.pColorBlendState = &BlendState;
	GraphicsPipelineCreateInfo.pTessellationState = NULL;
	GraphicsPipelineCreateInfo.pMultisampleState = &MultisampleState;
	GraphicsPipelineCreateInfo.pDynamicState = &DynamicState;
	GraphicsPipelineCreateInfo.pViewportState = &ViewportState;
	GraphicsPipelineCreateInfo.pDepthStencilState = &DepthStencilState;
	GraphicsPipelineCreateInfo.pStages = ShaderStage;
	BEAR_CHECK(!description.RenderPass.empty());
	GraphicsPipelineCreateInfo.renderPass = static_cast<const VKRenderPass*>(description.RenderPass.get())->RenderPass;
	GraphicsPipelineCreateInfo.stageCount = static_cast<uint32>(CountShader); 
	GraphicsPipelineCreateInfo.subpass = 0;
	V_CHK(vkCreateGraphicsPipelines(Factory->Device,0, 1, &GraphicsPipelineCreateInfo, NULL, &Pipeline));
}

VKPipelineGraphics::~VKPipelineGraphics()
{
	vkDestroyPipeline(Factory->Device, Pipeline, nullptr);
	PipelineGraphicsCounter--;
}

void VKPipelineGraphics::Set(VkCommandBuffer commad_list)
{
	vkCmdBindPipeline(commad_list, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline);
}

void* VKPipelineGraphics::QueryInterface(int type)
{
	switch (type)
	{
	case VKQ_Pipeline:
		return reinterpret_cast<void*>(static_cast<VKPipeline*>(this));
	default:
		return nullptr;
	}
}

BearPipelineType VKPipelineGraphics::GetType()
{
	return BearPipelineType::Graphics;
}
