#include "VKPCH.h"
size_t RootSignatureCounter = 0;
inline VkShaderStageFlags TransletionShaderVisible(BearShaderType type)
{
	switch (type)
	{
	case BearShaderType::Vertex:
		return VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
		break;
	case BearShaderType::Hull:
		return VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		break;
	case BearShaderType::Domain:
		return VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		break;
	case BearShaderType::Geometry:
		return VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT;
		break;
	case BearShaderType::Pixel:
		return VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;
		break;
	case BearShaderType::Compute:
		return VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT;
		break;
	case BearShaderType::RayTracing:
		return VkShaderStageFlagBits::VK_SHADER_STAGE_RAYGEN_BIT_KHR |
			VkShaderStageFlagBits::VK_SHADER_STAGE_MISS_BIT_KHR |
			VkShaderStageFlagBits::VK_SHADER_STAGE_CALLABLE_BIT_KHR |
			VkShaderStageFlagBits::VK_SHADER_STAGE_ANY_HIT_BIT_KHR |
			VkShaderStageFlagBits::VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR |
			VkShaderStageFlagBits::VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
		break;
	case BearShaderType::ALL:
		return VkShaderStageFlagBits::VK_SHADER_STAGE_ALL;
		break;
	default:
		BEAR_CHECK(0);
	}
	return VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
}
VKRootSignature::VKRootSignature(const BearRootSignatureDescription& description):Description(description)
{

	RootSignatureCounter++;
	CountBuffers = 0;
	CountSamplers = 0;
	CountSRVs = 0;
	CountUAVs = 0;
	{
		for (size_t i = 0; i < 16; i++)
		{
			SlotSamplers[i] = 16;
			SlotBuffers[i] = 16;
			SlotSRVs[i] = 16;
			SlotUAVs[i] = 16;
			if (description.UniformBuffers[i].Shader != BearShaderType::Null)CountBuffers++;
			if (description.SRVResources[CountSRVs].Shader != BearShaderType::Null) CountSRVs++;
			if (description.UAVResources[CountUAVs].Shader != BearShaderType::Null) CountUAVs++;
			if (description.Samplers[CountSamplers].Shader != BearShaderType::Null) CountSamplers++;
		}
	}
	
	{
		size_t Offset = 0;
		VkDescriptorSetLayoutBinding LayoutBinding[64] = {};
		{

			for (size_t i = 0; i < 16; i++)
			{
				if (description.UniformBuffers[i].Shader != BearShaderType::Null)
				{
					SlotBuffers[i] = Offset;
					LayoutBinding[Offset].binding = static_cast<uint32_t>(i);
					LayoutBinding[Offset].descriptorCount = 1;
					LayoutBinding[Offset].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					LayoutBinding[Offset].pImmutableSamplers = nullptr;
					LayoutBinding[Offset].stageFlags = TransletionShaderVisible(description.UniformBuffers[i].Shader);
					Offset++;
				}
			}
			for (size_t i = 0; i < 16; i++)
			{
				if (description.SRVResources[i].Shader != BearShaderType::Null)
				{
					SlotSRVs[i] = Offset - CountBuffers;
					LayoutBinding[Offset].binding = static_cast<uint32_t>(i + 16);
					LayoutBinding[Offset].descriptorCount = 1;
					switch (description.SRVResources[i].DescriptorType)
					{
					case BearSRVDescriptorType::Buffer:
						LayoutBinding[Offset].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
						break;
					case BearSRVDescriptorType::Image:
						LayoutBinding[Offset].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
						break;
					case BearSRVDescriptorType::AccelerationStructure:
						LayoutBinding[Offset].descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
						break;
					default:
						BEAR_CHECK(0);
						break;
					}
					LayoutBinding[Offset].pImmutableSamplers = nullptr;
					LayoutBinding[Offset].stageFlags = TransletionShaderVisible(description.SRVResources[i].Shader);
					Offset++;
				}
			
			}


			for (size_t i = 0; i < 16; i++)
			{
				if (description.UAVResources[i].Shader != BearShaderType::Null)
				{
					SlotUAVs[i] = Offset - (CountBuffers + CountSRVs);
					LayoutBinding[Offset].binding = static_cast<uint32_t>(i + 32);
					LayoutBinding[Offset].descriptorCount = 1;
					switch (description.UAVResources[i].DescriptorType)
					{
					case BearUAVDescriptorType::Buffer:
						LayoutBinding[Offset].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
						break;
					case BearUAVDescriptorType::Image:
						LayoutBinding[Offset].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
						break;
					default:
						BEAR_CHECK(0);
						break;
					}
					LayoutBinding[Offset].pImmutableSamplers = nullptr;
					LayoutBinding[Offset].stageFlags = TransletionShaderVisible(description.UAVResources[i].Shader);
					Offset++;
				}

			}

			for (size_t i = 0; i < 16; i++)
			{
				if (description.Samplers[i].Shader != BearShaderType::Null)
				{
					SlotSamplers[i] = Offset - (CountBuffers+CountSRVs+ CountUAVs);
					LayoutBinding[Offset].binding = static_cast<uint32_t>(i + 48);
					LayoutBinding[Offset].descriptorCount = 1;
					LayoutBinding[Offset].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
					LayoutBinding[Offset].pImmutableSamplers = nullptr;
					LayoutBinding[Offset].stageFlags = TransletionShaderVisible(description.Samplers[i].Shader);
					Offset++;
				}
			}
		}

		VkDescriptorSetLayoutCreateInfo LayoutCreateInfo = {};
		LayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		LayoutCreateInfo.bindingCount = static_cast<uint32_t>(Offset);;
		LayoutCreateInfo.pBindings = LayoutBinding;

		V_CHK(vkCreateDescriptorSetLayout(Factory->Device, &LayoutCreateInfo, nullptr, &DescriptorSetLayout));
	}
	{
		VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo = {};
		PipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		PipelineLayoutCreateInfo.pNext = NULL;
		PipelineLayoutCreateInfo.pushConstantRangeCount = 0;
		PipelineLayoutCreateInfo.pPushConstantRanges = NULL;
		PipelineLayoutCreateInfo.setLayoutCount = 1;
		PipelineLayoutCreateInfo.pSetLayouts = &DescriptorSetLayout;

		V_CHK(vkCreatePipelineLayout(Factory->Device, &PipelineLayoutCreateInfo, NULL, &PipelineLayout));
	}
		

}

VKRootSignature::~VKRootSignature()
{
	RootSignatureCounter--;
	vkDestroyPipelineLayout(Factory->Device, PipelineLayout, 0);
	vkDestroyDescriptorSetLayout(Factory->Device, DescriptorSetLayout, 0);
}

