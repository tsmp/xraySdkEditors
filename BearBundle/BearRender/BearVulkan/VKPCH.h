#pragma once
#include "../BearGraphics/BearRenderBase.h"
/////////////////////////////////////
///////////Configure/////////////////
/////////////////////////////////////
#ifdef VK_11
#if CURRENT_PROCCESOR == PROCCESOR_AMD64
#define RTX
#endif
#endif
/////////////////////////////////////
/////////////////////////////////////
/////////////////////////////////////
#ifdef DEVELOPER_VERSION
#ifdef RTX
#define NV_EXTENSIONS
#include "dxc/dxcapi.h"
#endif
#include "shaderc/shaderc.hpp"
#endif

#if CURRENT_PLATFORM == PLATFORM_WINDOWS
#ifndef VK_USE_PLATFORM_WIN32_KHR
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#endif

#define VK_NO_PROTOTYPES

#include "vulkan/vulkan.h"
extern void VkError(VkResult result);
#define V_CHK(a) { VkResult __Result_ = a ;if(__Result_!=VK_SUCCESS)VkError(__Result_); }

enum EVKQuery
{
	VKQ_None = 0,
	VKQ_ShaderResource,
	VKQ_UnorderedAccess,
	VKQ_Pipeline,
	VKQ_RayTracingPipeline
};

#include "VKImports.h"
#include "VKFactory.h"
#include "VKViewport.h"
#include "VKContext.h"
#include "VKShader.h"
#include "VKBufferTools.h"
#include "VKVertexBuffer.h"
#include "VKIndexBuffer.h"

#include "VKUniformBuffer.h"

#include "VKRootSignature.h"
#include "VKPipeline.h"
#include "VKPipelineGraphics.h"
#include "VKPipelineMesh.h"
#include "VKDescriptorHeap.h"

#include "VKSamplers.h"
#include "VKShaderResource.h"
#include "VKUnorderedAccess.h"
#include "VKTexture2D.h"
#include "VKStats.h"

#include "VKRenderPass.h" 
#include "VKFrameBuffer.h"
#include "VKTextureCube.h"
#include "VKStructuredBuffer.h"

#include "VKPipelineRayTracing.h"
#include "VKRayTracingBottomLevel.h"
#include "VKRayTracingTopLevel.h"
#include "VKRayTracingShaderTable.h"