#pragma once
class VKPipelineRayTracing :public BearRHI::BearRHIPipelineRayTracing,public VKPipeline
{
	//BEAR_CLASS_WITHOUT_COPY(VKPipelineGraphics);
public:
	VKPipelineRayTracing(const BearPipelineRayTracingDescription& description);
	virtual ~VKPipelineRayTracing();
	virtual void Set(VkCommandBuffer command_buffer);
	virtual void* QueryInterface(int type);
	virtual BearPipelineType GetType();
	void* GetShaderIdentifier(BearStringConteniarUnicode name);
#ifdef RTX
	BearMap<BearStringConteniarUnicode, bsize> GroupMap;
	VkPipeline Pipeline;
	BearFactoryPointer<BearRHI::BearRHIRootSignature> RootSignature;
	VKRootSignature* RootSignaturePointer;
	BearRef<uint8> ShaderIdentifiers;
#endif
};