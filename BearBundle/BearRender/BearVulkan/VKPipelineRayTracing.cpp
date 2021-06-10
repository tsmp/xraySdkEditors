#include "VKPCH.h"
size_t PipelineRayTracingCounter = 0;
#ifdef RTX
VKPipelineRayTracing::VKPipelineRayTracing(const BearPipelineRayTracingDescription& description)
{
	PipelineRayTracingCounter++;
	Pipeline = 0;

	BearVector<VkPipelineShaderStageCreateInfo> ShaderList;
	BearMap<BearStringConteniarUnicode,bsize> ShaderMap;


	BearVector<VkRayTracingShaderGroupCreateInfoNV> GroupList;

	for (const BearPipelineRayTracingDescription::ShaderDescription& i : description.Shaders)
	{
		auto ShaderLibrary = const_cast<VKShader*>(static_cast<const VKShader*>(i.Shader.get()));
		BEAR_CHECK(ShaderLibrary);
		BEAR_CHECK(ShaderLibrary->IsType(BearShaderType::RayTracing));
		VkPipelineShaderStageCreateInfo Shader = {};
		for (const BearPipelineRayTracingDescription::ShaderDescription::ExportDescription& a : i.Exports)
		{
			switch (a.Type)
			{
			case BearRayTracingShaderType::RayGeneration:
			{
				VkRayTracingShaderGroupCreateInfoNV GroupInfo = {};
				GroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
				GroupInfo.pNext = nullptr;
				GroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
				GroupInfo.generalShader =static_cast<uint32_t>( ShaderList.size());
				GroupInfo.closestHitShader = VK_SHADER_UNUSED_NV;
				GroupInfo.anyHitShader = VK_SHADER_UNUSED_NV;
				GroupInfo.intersectionShader = VK_SHADER_UNUSED_NV;
				GroupMap[a.NameExport] = GroupList.size();
				GroupList.emplace_back(GroupInfo);
				Shader = ShaderLibrary->Shader;
				Shader.pName = BearStringConteniarAnsi::Reserve(*BearEncoding::FastToAnsi( a.NameFunction.size() ? *a.NameFunction : *a.NameExport));
				Shader.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
				ShaderList.push_back(Shader);
			}
				break;
			case BearRayTracingShaderType::Miss:
			{
				VkRayTracingShaderGroupCreateInfoNV GroupInfo = {};
				GroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
				GroupInfo.pNext = nullptr;
				GroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
				GroupInfo.generalShader = static_cast<uint32_t>(ShaderList.size());
				GroupInfo.closestHitShader = VK_SHADER_UNUSED_NV;
				GroupInfo.anyHitShader = VK_SHADER_UNUSED_NV;
				GroupInfo.intersectionShader = VK_SHADER_UNUSED_NV;
				GroupMap[a.NameExport] = GroupList.size();
				GroupList.emplace_back(GroupInfo);
				Shader = ShaderLibrary->Shader;
				Shader.pName = BearStringConteniarAnsi::Reserve(*BearEncoding::FastToAnsi(a.NameFunction.size() ? *a.NameFunction : *a.NameExport));
				Shader.stage = VK_SHADER_STAGE_MISS_BIT_KHR;
				ShaderList.push_back(Shader);
			}
				break;
			case BearRayTracingShaderType::Callable:
			{
				VkRayTracingShaderGroupCreateInfoNV GroupInfo = {};
				GroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
				GroupInfo.pNext = nullptr;
				GroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
				GroupInfo.generalShader = static_cast<uint32_t>(ShaderList.size());
				GroupInfo.closestHitShader = VK_SHADER_UNUSED_NV;
				GroupInfo.anyHitShader = VK_SHADER_UNUSED_NV;
				GroupInfo.intersectionShader = VK_SHADER_UNUSED_NV;
				GroupMap[a.NameExport] = GroupList.size();
				GroupList.emplace_back(GroupInfo);
				Shader = ShaderLibrary->Shader;
				Shader.pName = BearStringConteniarAnsi::Reserve(*BearEncoding::FastToAnsi(a.NameFunction.size() ? *a.NameFunction : *a.NameExport));
				Shader.stage = VK_SHADER_STAGE_CALLABLE_BIT_KHR;
				ShaderList.push_back(Shader);
			}
				break;
			case BearRayTracingShaderType::Intersection:
			{
				Shader = ShaderLibrary->Shader;
				Shader.pName = BearStringConteniarAnsi::Reserve(*BearEncoding::FastToAnsi(a.NameFunction.size() ? *a.NameFunction : *a.NameExport));
				Shader.stage = VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
				ShaderMap[a.NameExport] = ShaderList.size();
				ShaderList.push_back(Shader);
			}
				break;
			case BearRayTracingShaderType::ClosestHit:
			{
				Shader = ShaderLibrary->Shader;
				Shader.pName = BearStringConteniarAnsi::Reserve(*BearEncoding::FastToAnsi(a.NameFunction.size() ? *a.NameFunction : *a.NameExport));
				Shader.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
				ShaderMap[a.NameExport] = ShaderList.size();
				ShaderList.push_back(Shader);
			}
				break;
			case BearRayTracingShaderType::AnyHit:
			{
				Shader = ShaderLibrary->Shader;
				Shader.pName = BearStringConteniarAnsi::Reserve(*BearEncoding::FastToAnsi(a.NameFunction.size() ? *a.NameFunction : *a.NameExport));
				Shader.stage = VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
				ShaderMap[a.NameExport] = ShaderList.size();
				ShaderList.push_back(Shader);
			}
				break;
			default:
				BEAR_CHECK(false);
				break;
			}

		}
	

	}
	for (const BearPipelineRayTracingDescription::HitGroupDescription& i : description.HitGroups)
	{
		VkRayTracingShaderGroupCreateInfoNV GroupInfo;
		GroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
		GroupInfo.pNext = nullptr;

		GroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
		switch (i.Type)
		{
		case BearHitGroupType::Triangles:
			GroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
			break;
		case BearHitGroupType::ProceduralPrimitive:
			GroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_NV;
			break;
		default:
			BEAR_CHECK(false);
			break;
		}
		GroupInfo.generalShader = VK_SHADER_UNUSED_NV;
		if(i.ClosestHitShaderImport.size())
		{
			auto Item = ShaderMap.find(i.ClosestHitShaderImport);
			BEAR_CHECK(Item != ShaderMap.end());
			GroupInfo.closestHitShader = static_cast<uint32_t>(Item->second);
		}
		else
		{
			GroupInfo.closestHitShader = VK_SHADER_UNUSED_NV;
		}
		if (i.AnyHitShaderImport.size())
		{
			auto Item = ShaderMap.find(i.AnyHitShaderImport);
			BEAR_CHECK(Item != ShaderMap.end());
			GroupInfo.anyHitShader = static_cast<uint32_t>(Item->second);
		}
		else
		{
			GroupInfo.anyHitShader = VK_SHADER_UNUSED_NV;
		}
		if (i.IntersectionShaderImport.size())
		{
			auto Item = ShaderMap.find(i.IntersectionShaderImport);
			BEAR_CHECK(Item != ShaderMap.end());
			GroupInfo.intersectionShader = static_cast<uint32_t>(Item->second);
		}
		else
		{
			GroupInfo.intersectionShader = VK_SHADER_UNUSED_NV;
		}
		GroupMap[i.NameExport] = GroupList.size();
		GroupList.emplace_back(GroupInfo);
	}

	RootSignature = description.GlobalRootSignature;
	BEAR_CHECK(RootSignature.empty() == false);
	RootSignaturePointer = static_cast<VKRootSignature*>(RootSignature.get());

	VkRayTracingPipelineCreateInfoNV RayPipelineInfo;
	RayPipelineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV;
	RayPipelineInfo.pNext = nullptr;
	RayPipelineInfo.flags = 0;
	RayPipelineInfo.stageCount = static_cast<uint32_t>(ShaderList.size());
	RayPipelineInfo.pStages = ShaderList.data();
	RayPipelineInfo.groupCount = static_cast<uint32_t>(GroupList.size());
	RayPipelineInfo.pGroups = GroupList.data();
	RayPipelineInfo.maxRecursionDepth = static_cast<uint32_t>(description.PipelineConfig.MaxTraceRecursionDepth);
	RayPipelineInfo.layout = RootSignaturePointer->PipelineLayout;
	RayPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	RayPipelineInfo.basePipelineIndex = 0;
	V_CHK(vkCreateRayTracingPipelinesNV(Factory->Device, VK_NULL_HANDLE, 1, &RayPipelineInfo, nullptr, &Pipeline));
	ShaderIdentifiers = bear_alloc<uint8>(GroupList.size() * Factory->PhysicalDeviceRayTracingProperties.shaderGroupHandleSize);
	V_CHK(vkGetRayTracingShaderGroupHandlesNV(Factory->Device, Pipeline, 0, static_cast<uint32_t>(GroupList.size()), GroupList.size()* Factory->PhysicalDeviceRayTracingProperties.shaderGroupHandleSize, *ShaderIdentifiers));


}

VKPipelineRayTracing::~VKPipelineRayTracing()
{
	if(Pipeline) vkDestroyPipeline(Factory->Device, Pipeline, nullptr);
	PipelineRayTracingCounter--;
}

void VKPipelineRayTracing::Set(VkCommandBuffer command_buffer)
{
	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, Pipeline);
}

void* VKPipelineRayTracing::QueryInterface(int type)
{
	switch (type)
	{
	case VKQ_Pipeline:
		return reinterpret_cast<void*>(static_cast<VKPipeline*>(this));
	case VKQ_RayTracingPipeline:
		return reinterpret_cast<void*>(this);
	default:
		return nullptr;
	}
}

BearPipelineType VKPipelineRayTracing::GetType()
{
	return BearPipelineType::RayTracing;
}

void* VKPipelineRayTracing::GetShaderIdentifier(BearStringConteniarUnicode name)
{
	auto Item = GroupMap.find(name);
	BEAR_CHECK(Item != GroupMap.end());
				
	return *(ShaderIdentifiers)+(Item->second * Factory->PhysicalDeviceRayTracingProperties.shaderGroupHandleSize);
}
#endif