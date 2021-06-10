#pragma once
class VKPipelineGraphics :public BearRHI::BearRHIPipelineGraphics,public VKPipeline
{
	//BEAR_CLASS_WITHOUT_COPY(VKPipelineGraphics);
public:
	VKPipelineGraphics(const BearPipelineGraphicsDescription&description);
	virtual ~VKPipelineGraphics();
	virtual void Set(VkCommandBuffer command_list);
	virtual void* QueryInterface(int type);
	virtual BearPipelineType GetType();
	VkPipeline Pipeline;
	BearFactoryPointer<BearRHI::BearRHIRootSignature> RootSignature;
	VKRootSignature* RootSignaturePointer;
};