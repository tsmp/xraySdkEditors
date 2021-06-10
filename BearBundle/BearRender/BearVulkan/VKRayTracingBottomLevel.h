#pragma once
class VKRayTracingBottomLevel : public BearRHI::BearRHIRayTracingBottomLevel
{
public:
	VKRayTracingBottomLevel(const BearRayTracingBottomLevelDescription& description);
	virtual ~VKRayTracingBottomLevel();
	VkAccelerationStructureNV AccelerationStructure;
	VkDeviceMemory ResultBufferMemory;
	uint64_t AccelerationStructureHandle;
};

