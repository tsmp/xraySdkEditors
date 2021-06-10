#pragma once
class VKRayTracingTopLevel : public VKShaderResource, public BearRHI::BearRHIRayTracingTopLevel
{
public:
	VKRayTracingTopLevel(const BearRayTracingTopLevelDescription& description);
	virtual ~VKRayTracingTopLevel();
	virtual void* QueryInterface(int Type);
	VkAccelerationStructureNV AccelerationStructure;
	VkDeviceMemory ResultBufferMemory;
	virtual void SetAsSRV(VkWriteDescriptorSet* heap, size_t offset);
	VkWriteDescriptorSetAccelerationStructureNV DescriptorAccelerationStructureInfo;
};

