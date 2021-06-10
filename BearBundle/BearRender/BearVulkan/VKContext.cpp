#include "VKPCH.h"
size_t ContextCounter = 0;

#ifdef DEVELOPER_VERSION
extern bool GDebugRender;
#endif

VKContext::VKContext() :m_PipelineType(BearPipelineType::Graphics), m_UseRenderPass(false)
{
	ContextCounter++;

	VkCommandPoolCreateInfo CommandPoolCreateInfo = {};
	CommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	CommandPoolCreateInfo.pNext = NULL;
	CommandPoolCreateInfo.queueFamilyIndex = Factory->QueueFamilyIndex;
	CommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	m_CommandPool = 0;
	V_CHK(vkCreateCommandPool(Factory->Device, &CommandPoolCreateInfo, NULL, &m_CommandPool));


	VkCommandBufferAllocateInfo CommandBufferAllocateInfo = {};
	CommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	CommandBufferAllocateInfo.pNext = NULL;
	CommandBufferAllocateInfo.commandPool = m_CommandPool;
	CommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	CommandBufferAllocateInfo.commandBufferCount = 1;

	V_CHK(vkAllocateCommandBuffers(Factory->Device, &CommandBufferAllocateInfo, &m_CommandBuffer));


	VkFenceCreateInfo FenceCreateInfo = {};
	FenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	FenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	V_CHK(vkCreateFence(Factory->Device, &FenceCreateInfo, nullptr, &m_Fence));
	V_CHK(vkWaitForFences(Factory->Device, 1, &m_Fence, true, UINT64_MAX));
	V_CHK(vkResetFences(Factory->Device, 1, &m_Fence));

	VkSemaphoreCreateInfo SemaphoreCreateInfo = {};
	SemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	SemaphoreCreateInfo.pNext = NULL;
	SemaphoreCreateInfo.flags = 0;

	V_CHK(vkCreateSemaphore(Factory->Device, &SemaphoreCreateInfo, NULL, &m_SemaphoreWait));


}
VKContext::~VKContext()
{
	vkDestroySemaphore(Factory->Device, m_SemaphoreWait, 0);
	vkDestroyFence(Factory->Device,m_Fence,0);
	vkFreeCommandBuffers(Factory->Device, m_CommandPool, 1, &m_CommandBuffer);
	vkDestroyCommandPool(Factory->Device, m_CommandPool, NULL);
	ContextCounter--;
}

void VKContext::Reset()
{
	m_UseRenderPass = false;
	VkCommandBufferInheritanceInfo CommandBufferInheritanceInfo = {};
	{
		CommandBufferInheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		CommandBufferInheritanceInfo.pNext = nullptr;
		CommandBufferInheritanceInfo.renderPass = 0;
		CommandBufferInheritanceInfo.subpass = 0;
		CommandBufferInheritanceInfo.framebuffer = 0;
		CommandBufferInheritanceInfo.occlusionQueryEnable = VK_FALSE;
		CommandBufferInheritanceInfo.queryFlags = 0;
		CommandBufferInheritanceInfo.pipelineStatistics = 0;
	}
	VkCommandBufferBeginInfo CommandBufferBeginInfo = {};
	{
		CommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		CommandBufferBeginInfo.pNext = nullptr;
		CommandBufferBeginInfo.flags = 0;
		CommandBufferBeginInfo.pInheritanceInfo = &CommandBufferInheritanceInfo;
	}
	V_CHK(vkBeginCommandBuffer(m_CommandBuffer, &CommandBufferBeginInfo));
}
void VKContext::Wait()
{
	V_CHK(vkWaitForFences(Factory->Device, 1, &m_Fence, true, UINT64_MAX));
	V_CHK(vkResetFences(Factory->Device, 1, &m_Fence));
}
void VKContext::Flush(BearFactoryPointer<BearRHI::BearRHIViewport> viewport, bool wait)
{
	static VkPipelineStageFlags Stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	{
		if (m_UseRenderPass)
			vkCmdEndRenderPass(m_CommandBuffer);
		m_UseRenderPass = false;
	}

	V_CHK(vkEndCommandBuffer(m_CommandBuffer));
	auto Viewport = static_cast<VKViewport*>(viewport.get());

	VkSubmitInfo SubmitInfo = {};
	SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	SubmitInfo.waitSemaphoreCount = 0;
	SubmitInfo.pWaitSemaphores = 0;
	SubmitInfo.signalSemaphoreCount = 0;
	SubmitInfo.pSignalSemaphores = &Viewport->Semaphore;
	SubmitInfo.pWaitDstStageMask = &Stage;
	SubmitInfo.pCommandBuffers = &m_CommandBuffer;
	SubmitInfo.commandBufferCount = 1;
	V_CHK(vkQueueSubmit(Factory->Queue, 1, &SubmitInfo, m_Fence));

	Viewport->Swap();
	if (wait)Wait();
}
void VKContext::Flush(bool wait)
{
	static VkPipelineStageFlags Stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	{
		if (m_UseRenderPass)
			vkCmdEndRenderPass(m_CommandBuffer);
		m_UseRenderPass = false;
	}

	V_CHK(vkEndCommandBuffer(m_CommandBuffer));

	VkSubmitInfo SubmitInfo = {};
	SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	SubmitInfo.waitSemaphoreCount = 0;
	SubmitInfo.pWaitSemaphores = 0;
	SubmitInfo.signalSemaphoreCount = 0;
	SubmitInfo.pSignalSemaphores = 0;
	SubmitInfo.pWaitDstStageMask = &Stage;
	SubmitInfo.pCommandBuffers = &m_CommandBuffer;
	SubmitInfo.commandBufferCount = 1;
	V_CHK(vkQueueSubmit(Factory->Queue, 1, &SubmitInfo, m_Fence));

	if (wait)Wait();
}

void VKContext::ClearFrameBuffer()
{
	if(m_UseRenderPass)
		vkCmdEndRenderPass(m_CommandBuffer);
	m_UseRenderPass = false;
}

void VKContext::BeginEvent(char const* name, BearColor color)
{
#ifdef DEVELOPER_VERSION
	if (GDebugRender)
	{
		VkDebugUtilsLabelEXT DebugUtilsLabelEXT = {};
		DebugUtilsLabelEXT.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
		DebugUtilsLabelEXT.pLabelName = name;
		memcpy(DebugUtilsLabelEXT.color, color.R32G32B32A32, sizeof(float) * 4);
		vkCmdBeginDebugUtilsLabelEXT(m_CommandBuffer, &DebugUtilsLabelEXT);
	}
#endif
}
void VKContext::EndEvent()
{
#ifdef DEVELOPER_VERSION
	if (GDebugRender)
	{
		vkCmdEndDebugUtilsLabelEXT(m_CommandBuffer);
	}
#endif
}



void VKContext::SetViewportAsFrameBuffer(BearFactoryPointer<BearRHI::BearRHIViewport> viewport)
{
	if (m_UseRenderPass)vkCmdEndRenderPass(m_CommandBuffer);
	VkRenderPassBeginInfo RenderPassBeginInfo = static_cast<VKViewport*>(viewport.get())->GetRenderPass();
	vkCmdBeginRenderPass(m_CommandBuffer, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	m_UseRenderPass = true;
}
void VKContext::SetFrameBuffer(BearFactoryPointer<BearRHI::BearRHIFrameBuffer> frame_buffer)
{
	if (m_UseRenderPass)vkCmdEndRenderPass(m_CommandBuffer);
	VkRenderPassBeginInfo RenderPassBeginInfo = static_cast<VKFrameBuffer*>(frame_buffer.get())->GetRenderPass();
	vkCmdBeginRenderPass(m_CommandBuffer, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	m_UseRenderPass = true;
}
void VKContext::SetPipeline(BearFactoryPointer<BearRHI::BearRHIPipeline> pipeline)
{
	VKPipeline* Pipeline = reinterpret_cast<VKPipeline*>(pipeline.get()->QueryInterface(VKQ_Pipeline));
	BEAR_CHECK(Pipeline);
	Pipeline->Set(m_CommandBuffer);
	m_PipelineType = Pipeline->GetType();
}
void VKContext::SetDescriptorHeap(BearFactoryPointer<BearRHI::BearRHIDescriptorHeap> descriptor_heap)
{
	switch (m_PipelineType)
	{
	case BearPipelineType::RayTracing:
		static_cast<VKDescriptorHeap*>(descriptor_heap.get())->SetRayTracing(m_CommandBuffer);
		break;
	default:
		static_cast<VKDescriptorHeap*>(descriptor_heap.get())->SetGraphics(m_CommandBuffer);
		break;
	}

}
void VKContext::SetVertexBuffer(BearFactoryPointer<BearRHI::BearRHIVertexBuffer> buffer)
{
	static VkDeviceSize Offset = 0;
	vkCmdBindVertexBuffers(m_CommandBuffer, 0, 1, &static_cast<VKVertexBuffer*>(buffer.get())->Buffer, &Offset);
}
void VKContext::SetIndexBuffer(BearFactoryPointer<BearRHI::BearRHIIndexBuffer> buffer)
{
	vkCmdBindIndexBuffer(m_CommandBuffer, static_cast<VKIndexBuffer*>(buffer.get())->Buffer, 0, VkIndexType::VK_INDEX_TYPE_UINT32);
}
void VKContext::SetStencilRef(uint32 ref)
{
	vkCmdSetStencilReference(m_CommandBuffer, VK_STENCIL_FRONT_AND_BACK, ref);
}
void VKContext::SetViewport(float x, float y, float width, float height, float min_depth, float max_depth)
{
	m_Viewport.x = x;
	m_Viewport.y = height - y;
	m_Viewport.width = width;
	m_Viewport.height = -height;
	m_Viewport.maxDepth = max_depth;
	m_Viewport.minDepth = min_depth;
	vkCmdSetViewport(m_CommandBuffer, 0, 1, &m_Viewport);
}
void VKContext::SetScissor(bool enable, float x, float y, float x1, float y1)
{
	VkRect2D Scissor = {};

	if (enable)
	{
		Scissor.offset.x = static_cast<int>(x);
		Scissor.offset.y = static_cast<int>(y);
		Scissor.extent.width = static_cast<uint32>(x1-x);
		Scissor.extent.height = static_cast<uint32>(y1-y);
	}
	else
	{
		Scissor.offset.x = static_cast<int>(m_Viewport.x);
		Scissor.offset.y = static_cast<int>(m_Viewport.height) + static_cast<int>(m_Viewport.y);
		Scissor.extent.width = static_cast<uint32>(m_Viewport.width);
		Scissor.extent.height = static_cast<uint32>(abs(m_Viewport.height));
	}

	vkCmdSetScissor(m_CommandBuffer, 0, 1, &Scissor);
}
void VKContext::Draw(size_t count, size_t offset)
{
	vkCmdDraw(m_CommandBuffer, static_cast<uint32>(count), 1, static_cast<uint32>(offset), 0);
}
void VKContext::DrawIndex(size_t count, size_t  offset_index, size_t offset_vertex)
{
	vkCmdDrawIndexed(m_CommandBuffer, static_cast<uint32>(count), 1, static_cast<uint32>(offset_index), static_cast<uint32>(offset_vertex), 0);
}

void VKContext::DispatchMesh(size_t count_x, size_t count_y, size_t count_z)
{
}

void VKContext::DispatchRays(bsize count_x, bsize count_y, bsize count_z, BearFactoryPointer<BearRHI::BearRHIRayTracingShaderTable> shader_table)
{
#ifdef RTX
	auto* ShaderTable = static_cast<const VKRayTracingShaderTable*>(shader_table.get());
	vkCmdTraceRaysNV(m_CommandBuffer, ShaderTable->RayGenerateRecord.Buffer, 0, ShaderTable->MissRecord.Buffer, 0, ShaderTable->MissRecord.Stride, ShaderTable->HitGroups.Buffer, 0, ShaderTable->HitGroups.Stride, ShaderTable->CallableRecord.Buffer, 0, ShaderTable->CallableRecord.Stride, static_cast<uint32_t>(count_x), static_cast<uint32_t>(count_y), static_cast<uint32_t>(count_z));
#endif
}


void VKContext::Copy(BearFactoryPointer<BearRHI::BearRHIIndexBuffer> dest, BearFactoryPointer<BearRHI::BearRHIIndexBuffer> source)
{
	if (dest.empty() || source.empty())return;
	if (static_cast<VKIndexBuffer*>(dest.get())->Buffer == 0)return;
	if (static_cast<VKIndexBuffer*>(source.get())->Buffer == 0)return;
	VkCommandBufferBeginInfo CommandBufferBeginInfo = {};
	{
		CommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		CommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	}
	V_CHK(vkBeginCommandBuffer(m_CommandBuffer, &CommandBufferBeginInfo));
	CopyBuffer(m_CommandBuffer, static_cast<VKIndexBuffer*>(source.get())->Buffer, static_cast<VKIndexBuffer*>(dest.get())->Buffer, static_cast<VKIndexBuffer*>(dest.get())->Size);
}
void VKContext::Copy(BearFactoryPointer<BearRHI::BearRHIVertexBuffer> dest, BearFactoryPointer<BearRHI::BearRHIVertexBuffer> source)
{
	if (dest.empty() || source.empty())return;
	if (static_cast<VKVertexBuffer*>(dest.get())->Buffer == 0)return;

	if (static_cast<VKVertexBuffer*>(source.get())->Buffer == 0)return;
	VkCommandBufferBeginInfo CommandBufferBeginInfo = {};
	CommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	CommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	V_CHK(vkBeginCommandBuffer(m_CommandBuffer, &CommandBufferBeginInfo));
	CopyBuffer(m_CommandBuffer, static_cast<VKVertexBuffer*>(source.get())->Buffer, static_cast<VKVertexBuffer*>(dest.get())->Buffer, static_cast<VKVertexBuffer*>(dest.get())->Size);
}

void VKContext::Copy(BearFactoryPointer<BearRHI::BearRHITexture2D> dest, BearFactoryPointer<BearRHI::BearRHITexture2D> source)
{
	if (dest.empty() || source.empty())return;
	if (static_cast<VKTexture2D*>(dest.get())->Image == 0)return;

	if (static_cast<VKTexture2D*>(source.get())->Image == 0)return;
	auto Dest = static_cast<VKTexture2D*>(dest.get());
	auto Source = static_cast<VKTexture2D*>(source.get());


	if (Source->ImageCreateInfo.extent.width != Dest->ImageCreateInfo.extent.width)return;
	if (Source->ImageCreateInfo.extent.height != Dest->ImageCreateInfo.extent.height)return;;
	if (Source->ImageCreateInfo.arrayLayers != Dest->ImageCreateInfo.arrayLayers)return;;
	if (Dest->ImageCreateInfo.mipLevels != 1)
		if (Source->ImageCreateInfo.mipLevels != Dest->ImageCreateInfo.mipLevels)return;

	VkCommandBufferBeginInfo CommandBufferBeginInfo = {};
	CommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	CommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	V_CHK(vkBeginCommandBuffer(m_CommandBuffer, &CommandBufferBeginInfo));
	{

		TransitionImageLayout(m_CommandBuffer, Dest->Image, Dest->ImageCreateInfo.format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, Dest->ImageCreateInfo.mipLevels, Dest->ImageCreateInfo.arrayLayers, 0);
		TransitionImageLayout(m_CommandBuffer, Source->Image, Source->ImageCreateInfo.format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, Source->ImageCreateInfo.mipLevels, Source->ImageCreateInfo.arrayLayers, 0);

		for (uint32_t i = 0; i < Dest->ImageCreateInfo.mipLevels; i++)
		{
			VkImageCopy ImageCopy = {};
			ImageCopy.extent.width = Dest->ImageCreateInfo.extent.width;
			ImageCopy.extent.height = Dest->ImageCreateInfo.extent.height;
			ImageCopy.extent.depth = Dest->ImageCreateInfo.arrayLayers;
			ImageCopy.dstSubresource.mipLevel = i;
			ImageCopy.srcSubresource.mipLevel = i;
			vkCmdCopyImage(m_CommandBuffer, Source->Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, Dest->Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &ImageCopy);
		}
		TransitionImageLayout(m_CommandBuffer, Dest->Image, Dest->ImageCreateInfo.format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, Dest->ImageCreateInfo.initialLayout, Dest->ImageCreateInfo.mipLevels, Dest->ImageCreateInfo.arrayLayers, 0);
		TransitionImageLayout(m_CommandBuffer, Source->Image, Source->ImageCreateInfo.format, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, Source->ImageCreateInfo.initialLayout, Source->ImageCreateInfo.mipLevels, Source->ImageCreateInfo.arrayLayers, 0);
	}

}
void VKContext::Copy(BearFactoryPointer<BearRHI::BearRHIUniformBuffer> dest, BearFactoryPointer<BearRHI::BearRHIUniformBuffer> source)
{
	if (dest.empty() || source.empty())return;
	if (static_cast<VKUniformBuffer*>(dest.get())->Buffer == 0)return;
	if (static_cast<VKUniformBuffer*>(source.get())->Buffer == 0)return;

	VkCommandBufferBeginInfo CommandBufferBeginInfo = {};
	CommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	CommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	V_CHK(vkBeginCommandBuffer(m_CommandBuffer, &CommandBufferBeginInfo));

	CopyBuffer(m_CommandBuffer, static_cast<VKUniformBuffer*>(source.get())->Buffer, static_cast<VKUniformBuffer*>(dest.get())->Buffer, static_cast<VKUniformBuffer*>(dest.get())->Count * static_cast<VKUniformBuffer*>(dest.get())->Stride);

}

void VKContext::Lock(BearFactoryPointer<BearRHI::BearRHIViewport> viewport)
{
}

void VKContext::Unlock(BearFactoryPointer<BearRHI::BearRHIViewport> viewport)
{
}

void VKContext::Lock(BearFactoryPointer<BearRHI::BearRHIFrameBuffer> frame_buffer)
{
	BEAR_CHECK(!frame_buffer.empty());
	static_cast<VKFrameBuffer*>(frame_buffer.get())->Lock(m_CommandBuffer);
}

void VKContext::Unlock(BearFactoryPointer<BearRHI::BearRHIFrameBuffer> frame_buffer)
{
	BEAR_CHECK(!frame_buffer.empty());
	static_cast<VKFrameBuffer*>(frame_buffer.get())->Unlock(m_CommandBuffer);
}
void VKContext::Lock(BearFactoryPointer<BearRHI::BearRHIUnorderedAccess> unordered_access)
{
	BEAR_CHECK(!unordered_access.empty());
	VKUnorderedAccess* UAV = reinterpret_cast<VKUnorderedAccess*>(unordered_access.get()->QueryInterface(VKQ_UnorderedAccess));
	UAV->LockUAV(m_CommandBuffer);
}
void VKContext::Unlock(BearFactoryPointer<BearRHI::BearRHIUnorderedAccess> unordered_access)
{
	BEAR_CHECK(!unordered_access.empty());
	VKUnorderedAccess* UAV = reinterpret_cast<VKUnorderedAccess*>(unordered_access.get()->QueryInterface(VKQ_UnorderedAccess));
	UAV->UnlockUAV(m_CommandBuffer);
}