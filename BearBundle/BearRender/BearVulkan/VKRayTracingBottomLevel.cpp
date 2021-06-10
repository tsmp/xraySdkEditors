#include "VKPCH.h"
size_t RayTracingBottomLevelCounter = 0;
#ifdef RTX
#ifdef MemoryBarrier
#undef MemoryBarrier
#endif
VKRayTracingBottomLevel::VKRayTracingBottomLevel(const BearRayTracingBottomLevelDescription& description):AccelerationStructure(VK_NULL_HANDLE), ResultBufferMemory(VK_NULL_HANDLE)
{
	RayTracingBottomLevelCounter++;
	BEAR_CHECK(Factory->bSupportRayTracing)
	BearVector<VkGeometryNV>GeometryDescs;
	for (const  BearRayTracingBottomLevelDescription::GeometryDescription& i : description.GeometryDescriptions)
	{
		VkGeometryNV Gometry = {};
		Gometry.sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
		Gometry.pNext = VK_NULL_HANDLE;
		Gometry.geometryType = VKFactory::Translation(i.Type);
		Gometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
		Gometry.geometry.triangles.pNext = VK_NULL_HANDLE;
		Gometry.geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
		Gometry.geometry.aabbs.pNext = VK_NULL_HANDLE;
		{
			Gometry.flags = 0;
			if (i.Flags.test((uint32)BearRaytracingGeometryFlags::Opaque))
			{
				Gometry.flags |= VK_GEOMETRY_OPAQUE_BIT_NV;
			}
			if (i.Flags.test((uint32)BearRaytracingGeometryFlags::NoDuplicateAnyhitInvocation))
			{
				Gometry.flags |= VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_NV;
			}
		}
		if (Gometry.geometryType == VK_GEOMETRY_TYPE_TRIANGLES_NV)
		{

			BEAR_CHECK(!i.Triangles.VertexBuffer.empty());
			Gometry.geometry.triangles.transformData = VK_NULL_HANDLE;
			{
				auto VertexBuffer = static_cast<const VKVertexBuffer*>(i.Triangles.VertexBuffer.get());
				BEAR_CHECK(VertexBuffer->Size);
				BEAR_CHECK(VertexBuffer->VertexDescription.stride);
				Gometry.geometry.triangles.vertexData = VertexBuffer->Buffer;
				Gometry.geometry.triangles.vertexStride = VertexBuffer->VertexDescription.stride;
				Gometry.geometry.triangles.vertexOffset = static_cast<VkDeviceSize>(i.Triangles.VertexOffset);
				Gometry.geometry.triangles.vertexCount = static_cast<uint32_t>(i.Triangles.VertexCount);
				Gometry.geometry.triangles.vertexFormat = VKFactory::TranslationForRayTracing(i.Triangles.VertexFormat);
				BEAR_CHECK(i.Triangles.VertexCount >= VertexBuffer->Size / VertexBuffer->VertexDescription.stride);
				
				
			}
			if (i.Triangles.IndexBuffer.empty())
			{
				Gometry.geometry.triangles.indexType = VK_INDEX_TYPE_NONE_NV;
			}
			else
			{
				Gometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
				auto IndexBuffer = static_cast<const VKIndexBuffer*>(i.Triangles.IndexBuffer.get());
				BEAR_CHECK(IndexBuffer->Size);
				Gometry.geometry.triangles.indexData = IndexBuffer->Buffer;
				Gometry.geometry.triangles.indexOffset = static_cast<VkDeviceSize>(i.Triangles.IndexOffset);
				Gometry.geometry.triangles.indexCount = static_cast<uint32_t>(i.Triangles.IndexCount);
			}
		}
		else
		{
			BEAR_CHECK(!i.AABB.Buffer.empty());
			auto Buffer = static_cast<const VKStructuredBuffer*>(i.AABB.Buffer.get());
			Gometry.geometry.aabbs.aabbData = Buffer->Buffer;
			Gometry.geometry.aabbs.numAABBs = static_cast<uint32_t>(i.AABB.Count);
			Gometry.geometry.aabbs.offset = static_cast<VkDeviceSize>(i.AABB.Offset);
			Gometry.geometry.aabbs.stride = static_cast<uint32_t>(i.AABB.Stride);
		}
		GeometryDescs.emplace_back(Gometry);
	}
	VkAccelerationStructureInfoNV AccelerationStructureInfo = {};

	{
	
		AccelerationStructureInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
		AccelerationStructureInfo.pNext = VK_NULL_HANDLE;
		AccelerationStructureInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
		AccelerationStructureInfo.flags = 0;
		AccelerationStructureInfo.instanceCount = VK_NULL_HANDLE;
		AccelerationStructureInfo.geometryCount = static_cast<uint32_t>(GeometryDescs.size());
		AccelerationStructureInfo.pGeometries = GeometryDescs.data();

		VkAccelerationStructureCreateInfoNV AccelerationStructureCreateInfo = {};
		AccelerationStructureCreateInfo.sType =	VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
		AccelerationStructureCreateInfo.pNext = VK_NULL_HANDLE;
		AccelerationStructureCreateInfo.info = AccelerationStructureInfo;
		AccelerationStructureCreateInfo.compactedSize = 0;

		V_CHK(vkCreateAccelerationStructureNV(Factory->Device, &AccelerationStructureCreateInfo, nullptr,&AccelerationStructure));
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
		vkGetAccelerationStructureMemoryRequirementsNV(Factory->Device, &MemoryRequirementsInfo,&MemoryRequirements);

		ResultSizeInBytes = MemoryRequirements.memoryRequirements.size;
		ResultMemoryTypeBits = MemoryRequirements.memoryRequirements.memoryTypeBits;
		// Store the memory requirements for use during build/update
		MemoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV;
		vkGetAccelerationStructureMemoryRequirementsNV(Factory->Device, &MemoryRequirementsInfo,&MemoryRequirements);

		ScratchSizeInBytes = MemoryRequirements.memoryRequirements.size;
		/*MemoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_UPDATE_SCRATCH_NV;
		vkGetAccelerationStructureMemoryRequirementsNV(Factory->Device, &MemoryRequirementsInfo,&MemoryRequirements);*/

	}
	VkBuffer ScratchBuffer;
	VkDeviceMemory ScratchBufferMemory;
	CreateBuffer(Factory->PhysicalDevice, Factory->Device, ScratchSizeInBytes, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, ScratchBuffer, ScratchBufferMemory);
	
	

	{
		VkMemoryAllocateInfo AllocInfo = {};
		AllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		AllocInfo.allocationSize = ResultSizeInBytes;
		AllocInfo.memoryTypeIndex = FindMemoryType(Factory->PhysicalDevice, static_cast<uint32_t>(ResultMemoryTypeBits), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		V_CHK(vkAllocateMemory(Factory->Device, &AllocInfo, nullptr, &ResultBufferMemory));
	}

	VkBindAccelerationStructureMemoryInfoNV AccelerationStructureMemoryInfo;
	AccelerationStructureMemoryInfo.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV;
	AccelerationStructureMemoryInfo.pNext = nullptr;
	AccelerationStructureMemoryInfo.accelerationStructure = AccelerationStructure;
	AccelerationStructureMemoryInfo.memory = ResultBufferMemory;
	AccelerationStructureMemoryInfo.memoryOffset = 0;
	AccelerationStructureMemoryInfo.deviceIndexCount = 0;
	AccelerationStructureMemoryInfo.pDeviceIndices = nullptr;

	V_CHK(vkBindAccelerationStructureMemoryNV(Factory->Device, 1, &AccelerationStructureMemoryInfo));

	Factory->LockCommandBuffer();

	vkCmdBuildAccelerationStructureNV(Factory->CommandBuffer, &AccelerationStructureInfo, VK_NULL_HANDLE, 0, false, AccelerationStructure, VK_NULL_HANDLE, ScratchBuffer, 0);
	VkMemoryBarrier MemoryBarrier;
	MemoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	MemoryBarrier.pNext = nullptr;
	MemoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
	MemoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;

	vkCmdPipelineBarrier(Factory->CommandBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV,VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 0, 1, &MemoryBarrier,0, nullptr, 0, nullptr);
	Factory->UnlockCommandBuffer();


	V_CHK(vkGetAccelerationStructureHandleNV(Factory->Device,AccelerationStructure, sizeof(uint64_t),&AccelerationStructureHandle));
	vkDestroyBuffer(Factory->Device, ScratchBuffer, VK_NULL_HANDLE);
	vkFreeMemory(Factory->Device, ScratchBufferMemory, 0);
}

VKRayTracingBottomLevel::~VKRayTracingBottomLevel()
{
	vkDestroyAccelerationStructureNV(Factory->Device, AccelerationStructure, VK_NULL_HANDLE);
	vkFreeMemory(Factory->Device, ResultBufferMemory, VK_NULL_HANDLE);
	RayTracingBottomLevelCounter--;
}
#endif