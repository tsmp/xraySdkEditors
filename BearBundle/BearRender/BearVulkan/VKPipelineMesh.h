#pragma once
class VKPipelineMesh :public BearRHI::BearRHIPipelineMesh,public VKPipeline
{
	//BEAR_CLASS_WITHOUT_COPY(VKPipelineGraphics);
public:
	VKPipelineMesh(const BearPipelineMeshDescription&description);
	virtual ~VKPipelineMesh();
	virtual void Set(VkCommandBuffer command_list);
	virtual void* QueryInterface(int type);
	virtual BearPipelineType GetType();
	VkPipeline Pipeline;
	BearFactoryPointer<BearRHI::BearRHIRootSignature> RootSignature;
	VKRootSignature* RootSignaturePointer;
};