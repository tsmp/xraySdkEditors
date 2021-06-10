#include "VKPCH.h"
size_t DescriptorHeapCounter = 0;
VKDescriptorHeap::VKDescriptorHeap(const BearDescriptorHeapDescription& description)
{
	DescriptorHeapCounter++;
	CountBuffers = 0;
	CountSamplers = 0;
	CountSRVs = 0;

	BEAR_ASSERT(!description.RootSignature.empty());

	const VKRootSignature* RootSig = static_cast<const VKRootSignature*>(description.RootSignature.get());
	{


		CountBuffers = static_cast<const VKRootSignature*>(description.RootSignature.get())->CountBuffers;
		CountSRVs = static_cast<const VKRootSignature*>(description.RootSignature.get())->CountSRVs;
		CountSamplers = static_cast<const VKRootSignature*>(description.RootSignature.get())->CountSamplers;
		CountUAVs = static_cast<const VKRootSignature*>(description.RootSignature.get())->CountUAVs;

		memcpy(SlotBuffers, static_cast<const VKRootSignature*>(description.RootSignature.get())->SlotBuffers, 16 * sizeof(size_t));
		memcpy(SlotSRVs, static_cast<const VKRootSignature*>(description.RootSignature.get())->SlotSRVs, 16 * sizeof(size_t));
		memcpy(SlotUAVs, static_cast<const VKRootSignature*>(description.RootSignature.get())->SlotUAVs, 16 * sizeof(size_t));
		memcpy(SlotSamplers, static_cast<const VKRootSignature*>(description.RootSignature.get())->SlotSamplers, 16 * sizeof(size_t));
	}

	{
		VkDescriptorPoolSize PoolSizes[64] = {};
		size_t Offset = 0;
		for (size_t i = 0; i < CountBuffers; i++)
		{
			PoolSizes[i + Offset].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			PoolSizes[i + Offset].descriptorCount = 1;
		}
		Offset += CountBuffers;

		for (size_t i = 0; i < CountSRVs; i++)
		{
			switch (RootSig->Description.SRVResources[RootSig->SlotSRVs[i]].DescriptorType)
			{
			case BearSRVDescriptorType::Buffer:
				PoolSizes[i + Offset].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				break;
			case BearSRVDescriptorType::Image:
				PoolSizes[i + Offset].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				break;
			case BearSRVDescriptorType::AccelerationStructure:
				PoolSizes[i + Offset].type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
				break;
			default:
				BEAR_CHECK(0);
				break;
			}
			PoolSizes[i + Offset].descriptorCount = 1;
		}
		Offset += CountSRVs;
		for (size_t i = 0; i < CountUAVs; i++)
		{
			switch (RootSig->Description.UAVResources[RootSig->SlotSRVs[i]].DescriptorType)
			{
			case BearUAVDescriptorType::Buffer:
				PoolSizes[i + Offset].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				break;
			case BearUAVDescriptorType::Image:
				PoolSizes[i + Offset].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
				break;
			default:
				BEAR_CHECK(0);
				break;
			}
			PoolSizes[i + Offset].descriptorCount = 1;
		}
		Offset += CountUAVs;
		for (size_t i = 0; i < CountSamplers; i++)
		{
			PoolSizes[i + Offset].type = VK_DESCRIPTOR_TYPE_SAMPLER;
			PoolSizes[i + Offset].descriptorCount = 1;
		}

		VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo = {};
		DescriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		DescriptorPoolCreateInfo.pNext = NULL;
		DescriptorPoolCreateInfo.maxSets = 1;
		DescriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(CountBuffers+ CountSRVs+ CountSamplers);;;
		DescriptorPoolCreateInfo.pPoolSizes = PoolSizes;

		V_CHK(vkCreateDescriptorPool(Factory->Device, &DescriptorPoolCreateInfo, NULL, &DescriptorPool));
	}

	{
		RootSignature = description.RootSignature;
		RootSignaturePointer = static_cast<VKRootSignature*>(RootSignature.get());
	}
	{
		
			VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo[1];
			DescriptorSetAllocateInfo[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			DescriptorSetAllocateInfo[0].pNext = NULL;
			DescriptorSetAllocateInfo[0].descriptorPool = DescriptorPool;
			DescriptorSetAllocateInfo[0].descriptorSetCount = 1;
			DescriptorSetAllocateInfo[0].pSetLayouts = &(RootSignaturePointer->DescriptorSetLayout);

			V_CHK(vkAllocateDescriptorSets(Factory->Device, DescriptorSetAllocateInfo, &DescriptorSet));
	}
	{
	

	//	BearDebug::DebugBreak();
		
	}
}

VKDescriptorHeap::~VKDescriptorHeap()
{
	DescriptorHeapCounter--;

	vkDestroyDescriptorPool(Factory->Device, DescriptorPool, 0);
}

void VKDescriptorHeap::SetUniformBuffer(size_t slot, BearFactoryPointer<BearRHI::BearRHIUniformBuffer> uniform_buffer, size_t offset)
{
	if (uniform_buffer.empty())return;
	BEAR_CHECK(slot < 16);
	/*slot = SlotBuffers[slot];
	BEAR_CHECK(slot < CountBuffers);*/
	if (UniformBuffers[slot] == uniform_buffer)
	{
		if (UniformBufferOffsets[slot] == offset)
		{
			return;
		}

	}
	UniformBuffers[slot] = uniform_buffer;
	UniformBufferOffsets[slot] = offset;
	VkWriteDescriptorSet WriteDescriptorSet = {};
	auto* UniformBuffer = static_cast<const VKUniformBuffer*>(uniform_buffer.get());
	WriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	WriteDescriptorSet.pNext = NULL;
	WriteDescriptorSet.dstSet = DescriptorSet;
	WriteDescriptorSet.descriptorCount = 1;
	WriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

	VkDescriptorBufferInfo BufferInfo;
	BufferInfo.buffer = UniformBuffer->Buffer;
	BufferInfo.range = UniformBuffer->Stride;
	BufferInfo.offset = UniformBuffer->Stride* offset;

	WriteDescriptorSet.pBufferInfo = &BufferInfo;
	WriteDescriptorSet.dstArrayElement = 0;
	WriteDescriptorSet.dstBinding = static_cast<uint32_t>( slot);;
	vkUpdateDescriptorSets(Factory->Device, static_cast<uint32_t>(1), &WriteDescriptorSet, 0, NULL);
}

void VKDescriptorHeap::SetShaderResource(size_t slot, BearFactoryPointer<BearRHI::BearRHIShaderResource> shader_resource,size_t offset)
{
	if (shader_resource.empty())return;
	BEAR_CHECK(slot < 16);
	/*slot = SlotSRVs[slot];
	BEAR_CHECK(slot < CountSRVs);*/
	if (ShaderResources[slot] == shader_resource)
	{
		if (ShaderResourcesOffsets[slot] == offset)return;
	}
	ShaderResourcesOffsets[slot] = offset;
	ShaderResources[slot] = shader_resource;
	VkWriteDescriptorSet WriteDescriptorSet = {};
	WriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	WriteDescriptorSet.pNext = NULL;
	WriteDescriptorSet.dstSet = DescriptorSet;
	WriteDescriptorSet.descriptorCount = 1;
	VKShaderResource* ShaderResource = reinterpret_cast<VKShaderResource*>(shader_resource.get()->QueryInterface(VKQ_ShaderResource));
	BEAR_ASSERT(ShaderResource);
	ShaderResource->SetAsSRV(&WriteDescriptorSet, offset);
	WriteDescriptorSet.dstArrayElement = 0;
	WriteDescriptorSet.dstBinding = static_cast<uint32_t>(16 + slot);
	vkUpdateDescriptorSets(Factory->Device, static_cast<uint32_t>(1), &WriteDescriptorSet, 0, NULL);
}

void VKDescriptorHeap::SetSampler(size_t slot, BearFactoryPointer<BearRHI::BearRHISampler> sampler)
{
	if (sampler.empty())return;
	BEAR_CHECK(slot < 16);
	//slot = SlotSamplers[slot];
//	BEAR_CHECK(slot < CountSamplers);
	if (Samplers[slot] == sampler)return;

	Samplers[slot] = sampler;
	VkWriteDescriptorSet WriteDescriptorSet = {};
	WriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	WriteDescriptorSet.pNext = NULL;
	WriteDescriptorSet.dstSet = DescriptorSet;
	WriteDescriptorSet.descriptorCount = 1;
	auto* Sampler = static_cast<const VKSamplerState*>(sampler.get());

	WriteDescriptorSet.dstArrayElement = 0;
	WriteDescriptorSet.dstBinding = static_cast<uint32_t>(48 + slot);
	WriteDescriptorSet.pImageInfo = &Sampler->ImageInfo;
	WriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
	vkUpdateDescriptorSets(Factory->Device, static_cast<uint32_t>(1), &WriteDescriptorSet, 0, NULL);
}

void VKDescriptorHeap::SetUnorderedAccess(bsize slot, BearFactoryPointer<BearRHI::BearRHIUnorderedAccess> unordered_acces, bsize offset)
{
	if (unordered_acces.empty())return;
	BEAR_CHECK(slot < 16);
	/*slot = SlotSRVs[slot];
	BEAR_CHECK(slot < CountSRVs);*/
	if (UnorderedAccess[slot] == unordered_acces)
	{
		if (UnorderedAccessOffsets[slot] == offset)return;
	}
	UnorderedAccessOffsets[slot] = offset;
	UnorderedAccess[slot] = unordered_acces;
	VkWriteDescriptorSet WriteDescriptorSet = {};
	WriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	WriteDescriptorSet.pNext = NULL;
	WriteDescriptorSet.dstSet = DescriptorSet;
	WriteDescriptorSet.descriptorCount = 1;
	VKUnorderedAccess* UnorderedAcces = reinterpret_cast<VKUnorderedAccess*>(unordered_acces.get()->QueryInterface(VKQ_UnorderedAccess));
	BEAR_ASSERT(UnorderedAcces);
	UnorderedAcces->SetAsUAV(&WriteDescriptorSet, offset);
	WriteDescriptorSet.dstArrayElement = 0;
	WriteDescriptorSet.dstBinding = static_cast<uint32_t>(32 + slot);
	vkUpdateDescriptorSets(Factory->Device, static_cast<uint32_t>(1), &WriteDescriptorSet, 0, NULL);
}

void VKDescriptorHeap::SetGraphics(VkCommandBuffer command_line)
{
	vkCmdBindDescriptorSets(command_line, VK_PIPELINE_BIND_POINT_GRAPHICS, RootSignaturePointer->PipelineLayout, 0, 1, &DescriptorSet, 0, NULL);
}

void VKDescriptorHeap::SetRayTracing(VkCommandBuffer command_line)
{
	vkCmdBindDescriptorSets(command_line, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, RootSignaturePointer->PipelineLayout, 0, 1, &DescriptorSet, 0, NULL);
}
