#include "VKPCH.h"
size_t RayTracingTopLevelCounter = 0;
#ifdef RTX
struct VkGeometryInstance 
{
    float transform[3][4];
    uint32_t instanceId : 24;
    uint32_t mask : 8;
    uint32_t instanceOffset : 24;
    uint32_t flags : 8;
    uint64_t accelerationStructureHandle;
};

#ifdef MemoryBarrier
#undef MemoryBarrier
#endif

VKRayTracingTopLevel::VKRayTracingTopLevel(const BearRayTracingTopLevelDescription& description)
{
    RayTracingTopLevelCounter++;
	DescriptorAccelerationStructureInfo.pNext = VK_NULL_HANDLE;
	DescriptorAccelerationStructureInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV;
	BearVector<VkGeometryInstance> InstanceDescs;
	InstanceDescs.reserve(description.InstanceDescriptions.size());
	for (const BearRayTracingTopLevelDescription::InstanceDescription& i : description.InstanceDescriptions)
	{
		VkGeometryInstance InstanceDesc = {};
		for (bsize x = 0; x < 3; x++)
		{
			for (bsize y = 0; y < 4; y++)
			{
				InstanceDesc.transform[x][y] = i.Transform3x4.Get(x, y);
			}
		}
		InstanceDesc.instanceId = i.InstanceID;
		InstanceDesc.mask = i.InstanceMask;
		InstanceDesc.instanceOffset = i.InstanceContributionToHitGroupIndex;
		InstanceDesc.flags = *i.Flags;
		auto BottomLevel = static_cast<const VKRayTracingBottomLevel*>(i.BottomLevel.get());
		InstanceDesc.accelerationStructureHandle = BottomLevel->AccelerationStructureHandle;
		InstanceDescs.push_back(InstanceDesc);
	}
	VkAccelerationStructureInfoNV AccelerationStructureInfo = {};

	{

		AccelerationStructureInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
		AccelerationStructureInfo.pNext = VK_NULL_HANDLE;
		AccelerationStructureInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
		AccelerationStructureInfo.flags = 0;
		AccelerationStructureInfo.instanceCount = static_cast<uint32_t>(InstanceDescs.size());
		AccelerationStructureInfo.geometryCount = 0;;

		VkAccelerationStructureCreateInfoNV AccelerationStructureCreateInfo = {};
		AccelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
		AccelerationStructureCreateInfo.pNext = VK_NULL_HANDLE;
		AccelerationStructureCreateInfo.info = AccelerationStructureInfo;
		AccelerationStructureCreateInfo.compactedSize = 0;

		V_CHK(vkCreateAccelerationStructureNV(Factory->Device, &AccelerationStructureCreateInfo, nullptr, &AccelerationStructure));
	}
	bsize ResultSizeInBytes = 0;
	bsize ResultMemoryTypeBits = 0;
	bsize ScratchSizeInBytes = 0;
	{
		VkAccelerationStructureMemoryRequirementsInfoNV MemoryRequirementsInfo;
		MemoryRequirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
		MemoryRequirementsInfo.pNext = VK_NULL_HANDLE;
		MemoryRequirementsInfo.accelerationStructure = AccelerationStructure;
		MemoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV;

		VkMemoryRequirements2 MemoryRequirements = {};
		MemoryRequirements.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
		vkGetAccelerationStructureMemoryRequirementsNV(Factory->Device, &MemoryRequirementsInfo, &MemoryRequirements);

		ResultSizeInBytes = MemoryRequirements.memoryRequirements.size;
		ResultMemoryTypeBits = MemoryRequirements.memoryRequirements.memoryTypeBits;
		// Store the memory requirements for use during build/update
		MemoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV;
		vkGetAccelerationStructureMemoryRequirementsNV(Factory->Device, &MemoryRequirementsInfo, &MemoryRequirements);

		/*MemoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_UPDATE_SCRATCH_NV;
		vkGetAccelerationStructureMemoryRequirementsNV(Factory->Device, &MemoryRequirementsInfo,&MemoryRequirements);*/

		ScratchSizeInBytes = MemoryRequirements.memoryRequirements.size;
	}
	VkBuffer ScratchBuffer;
	VkDeviceMemory ScratchBufferMemory;
	CreateBuffer(Factory->PhysicalDevice, Factory->Device,ScratchSizeInBytes, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, ScratchBuffer, ScratchBufferMemory);



	{
		VkMemoryAllocateInfo AllocInfo = {};
		AllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		AllocInfo.allocationSize = ResultSizeInBytes;
		AllocInfo.memoryTypeIndex = FindMemoryType(Factory->PhysicalDevice, static_cast<uint32_t>(ResultMemoryTypeBits), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		V_CHK(vkAllocateMemory(Factory->Device, &AllocInfo, nullptr, &ResultBufferMemory));
	}

	VkBindAccelerationStructureMemoryInfoNV AccelerationStructureMemoryInfo = {};
	AccelerationStructureMemoryInfo.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV;
	AccelerationStructureMemoryInfo.pNext = nullptr;
	AccelerationStructureMemoryInfo.accelerationStructure = AccelerationStructure;
	AccelerationStructureMemoryInfo.memory = ResultBufferMemory;
	AccelerationStructureMemoryInfo.memoryOffset = 0;
	AccelerationStructureMemoryInfo.deviceIndexCount = 0;
	AccelerationStructureMemoryInfo.pDeviceIndices = nullptr;

	V_CHK(vkBindAccelerationStructureMemoryNV(Factory->Device, 1, &AccelerationStructureMemoryInfo));


	VkBuffer InstancesBuffer;
	VkDeviceMemory InstancesBufferMemory;
	
	CreateBuffer(Factory->PhysicalDevice, Factory->Device, sizeof(VkGeometryInstance)*InstanceDescs.size(), VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, InstancesBuffer, InstancesBufferMemory);
	{
		void* data;
		V_CHK(vkMapMemory(Factory->Device, InstancesBufferMemory, 0, sizeof(VkGeometryInstance) * InstanceDescs.size(), 0, &data));
		bear_copy(data, InstanceDescs.data(), sizeof(VkGeometryInstance) * InstanceDescs.size());
		vkUnmapMemory(Factory->Device, InstancesBufferMemory);
	}
	Factory->LockCommandBuffer();

	vkCmdBuildAccelerationStructureNV(Factory->CommandBuffer, &AccelerationStructureInfo, InstancesBuffer, 0, false, AccelerationStructure, VK_NULL_HANDLE, ScratchBuffer, 0);
	VkMemoryBarrier MemoryBarrier;
	MemoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	MemoryBarrier.pNext = nullptr;
	MemoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
	MemoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;

	vkCmdPipelineBarrier(Factory->CommandBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 0, 1, &MemoryBarrier, 0, nullptr, 0, nullptr);
	Factory->UnlockCommandBuffer();
	
	vkDestroyBuffer(Factory->Device, ScratchBuffer, VK_NULL_HANDLE);
	vkFreeMemory(Factory->Device, ScratchBufferMemory, 0);
	vkDestroyBuffer(Factory->Device, InstancesBuffer, VK_NULL_HANDLE);
	vkFreeMemory(Factory->Device, InstancesBufferMemory, 0);
}

VKRayTracingTopLevel::~VKRayTracingTopLevel()
{
	vkDestroyAccelerationStructureNV(Factory->Device, AccelerationStructure, VK_NULL_HANDLE);
	vkFreeMemory(Factory->Device, ResultBufferMemory, VK_NULL_HANDLE);
    RayTracingTopLevelCounter--;
}

void* VKRayTracingTopLevel::QueryInterface(int Type)
{
	switch (Type)
	{
	case VKQ_ShaderResource:
		return reinterpret_cast<void*>(static_cast<VKShaderResource*>(this));
	default:
		return nullptr;
	}
}

void VKRayTracingTopLevel::SetAsSRV(VkWriteDescriptorSet* heap, size_t offset)
{
	BEAR_CHECK(AccelerationStructure);
	DescriptorAccelerationStructureInfo.accelerationStructureCount = 1;
	DescriptorAccelerationStructureInfo.pAccelerationStructures = &AccelerationStructure;
	heap->descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
	heap->pNext = &DescriptorAccelerationStructureInfo;
}
#endif