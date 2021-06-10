#include "VKPCH.h"
bsize RayTracingShaderTableCounter = 0;
inline bsize GetAlignment(bsize size, const bsize alignment)
{
    return (size + (alignment - 1)) & (~(alignment - 1));
}
#ifdef RTX
VKRayTracingShaderTable::VKRayTracingShaderTable(const BearRayTracingShaderTableDescription& description)
{
	RayTracingShaderTableCounter++;
	VKPipelineRayTracing* Pipeline = reinterpret_cast<VKPipelineRayTracing*>(const_cast<BearRHI::BearRHIPipelineRayTracing*>(description.Pipeline.get())->QueryInterface(VKQ_RayTracingPipeline));
	BEAR_CHECK(Pipeline);

	
	if (description.RayGenerateShader.size())
	{
		VkBuffer Buffer;
		VkDeviceMemory Memory;

		void* Identifier = Pipeline->GetShaderIdentifier(description.RayGenerateShader);
		uint8* Pointer =  nullptr;
		bsize Size = GetAlignment(Factory->PhysicalDeviceRayTracingProperties.shaderGroupHandleSize, Factory->PhysicalDeviceRayTracingProperties.shaderGroupBaseAlignment);

		CreateBuffer(Factory->PhysicalDevice, Factory->Device, Size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, Buffer, Memory);

		V_CHK(vkMapMemory(Factory->Device, Memory, 0, Size, 0, (void**)&Pointer));
		bear_copy(Pointer, Identifier, Factory->PhysicalDeviceRayTracingProperties.shaderGroupHandleSize);
		vkUnmapMemory(Factory->Device, Memory);

		RayGenerateRecord.Buffer = Buffer;
		RayGenerateRecord.Memory = Memory;
		RayGenerateRecord.Stride = Size;
	}
	if (description.MissShader.size())
	{
		VkBuffer Buffer;
		VkDeviceMemory Memory;
		void* Identifier = Pipeline->GetShaderIdentifier(description.MissShader);
		uint8* Pointer = nullptr;
		bsize Size = GetAlignment(Factory->PhysicalDeviceRayTracingProperties.shaderGroupHandleSize, Factory->PhysicalDeviceRayTracingProperties.shaderGroupBaseAlignment);

		CreateBuffer(Factory->PhysicalDevice, Factory->Device, Size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, Buffer, Memory);

		V_CHK(vkMapMemory(Factory->Device, Memory, 0, Size, 0, (void**)&Pointer));
		bear_copy(Pointer, Identifier, Factory->PhysicalDeviceRayTracingProperties.shaderGroupHandleSize);
		vkUnmapMemory(Factory->Device, Memory);

		MissRecord.Buffer = Buffer;
		MissRecord.Memory = Memory;
		MissRecord.Stride = Size;
	}
	if (description.CallableShader.size())
	{

		VkBuffer Buffer;
		VkDeviceMemory Memory;
		void* Identifier = Pipeline->GetShaderIdentifier(description.CallableShader);
		uint8* Pointer = nullptr;
		bsize Size = GetAlignment(Factory->PhysicalDeviceRayTracingProperties.shaderGroupHandleSize, Factory->PhysicalDeviceRayTracingProperties.shaderGroupBaseAlignment);

		CreateBuffer(Factory->PhysicalDevice, Factory->Device, Size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, Buffer, Memory);

		V_CHK(vkMapMemory(Factory->Device, Memory, 0, Size, 0, (void**)&Pointer));
		bear_copy(Pointer, Identifier, Factory->PhysicalDeviceRayTracingProperties.shaderGroupHandleSize);
		vkUnmapMemory(Factory->Device, Memory);

		CallableRecord.Buffer = Buffer;
		CallableRecord.Memory = Memory;
		CallableRecord.Stride = Size;
	}
	{

		VkBuffer Buffer;
		VkDeviceMemory Memory;
		uint8* Pointer = nullptr;
		bsize Size = GetAlignment(Factory->PhysicalDeviceRayTracingProperties.shaderGroupHandleSize, Factory->PhysicalDeviceRayTracingProperties.shaderGroupBaseAlignment)* description.HitGroups.size();

		CreateBuffer(Factory->PhysicalDevice, Factory->Device, Size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, Buffer, Memory);

	
		HitGroups.Buffer = Buffer;
		HitGroups.Memory = Memory;
		HitGroups.Stride = GetAlignment(Factory->PhysicalDeviceRayTracingProperties.shaderGroupHandleSize, Factory->PhysicalDeviceRayTracingProperties.shaderGroupBaseAlignment);

		V_CHK(vkMapMemory(Factory->Device, Memory, 0, Size, 0, (void**)&Pointer));
		for (bsize i = 0; i < description.HitGroups.size(); i++)
		{
			void* Identifier = Pipeline->GetShaderIdentifier(description.HitGroups[i]);
			bear_copy(Pointer, Identifier, Factory->PhysicalDeviceRayTracingProperties.shaderGroupHandleSize);
			Pointer += CallableRecord.Stride;
		}
		vkUnmapMemory(Factory->Device, Memory);
	}

}

VKRayTracingShaderTable::~VKRayTracingShaderTable()
{
	RayTracingShaderTableCounter--;
	if (RayGenerateRecord.Buffer)
	{
		vkDestroyBuffer(Factory->Device, RayGenerateRecord.Buffer, 0);
		vkFreeMemory(Factory->Device, RayGenerateRecord.Memory, 0);
	}
	if (MissRecord.Buffer)
	{
		vkDestroyBuffer(Factory->Device, MissRecord.Buffer, 0);
		vkFreeMemory(Factory->Device, MissRecord.Memory, 0);
	}
	if (CallableRecord.Buffer)
	{
		vkDestroyBuffer(Factory->Device, CallableRecord.Buffer, 0);
		vkFreeMemory(Factory->Device, CallableRecord.Memory, 0);
	}
	if (HitGroups.Buffer)
	{
		vkDestroyBuffer(Factory->Device, HitGroups.Buffer, 0);
		vkFreeMemory(Factory->Device, HitGroups.Memory, 0);
	}

}
#endif