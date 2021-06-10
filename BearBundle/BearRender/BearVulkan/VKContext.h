#pragma once
class VKContext :public BearRHI::BearRHIContext
{
public:
	VKContext();
	virtual ~VKContext();

	virtual void BeginEvent(char const* name, BearColor color = BearColor::White);
	virtual void EndEvent();

	virtual void Reset();
	virtual void Wait();
	virtual void Flush(BearFactoryPointer<BearRHI::BearRHIViewport> viewport, bool wait);
	virtual void Flush(bool wait);
	virtual void ClearFrameBuffer();

	virtual void Copy(BearFactoryPointer<BearRHI::BearRHIIndexBuffer> dest, BearFactoryPointer<BearRHI::BearRHIIndexBuffer> source);
	virtual void Copy(BearFactoryPointer<BearRHI::BearRHIVertexBuffer> dest, BearFactoryPointer<BearRHI::BearRHIVertexBuffer> source);
	virtual void Copy(BearFactoryPointer<BearRHI::BearRHIUniformBuffer> dest, BearFactoryPointer<BearRHI::BearRHIUniformBuffer> source);
	virtual void Copy(BearFactoryPointer<BearRHI::BearRHITexture2D> dest, BearFactoryPointer<BearRHI::BearRHITexture2D> source);

	virtual void SetViewportAsFrameBuffer(BearFactoryPointer<BearRHI::BearRHIViewport> viewport);
	virtual void SetFrameBuffer(BearFactoryPointer<BearRHI::BearRHIFrameBuffer> frame_buffer);
	virtual void SetPipeline(BearFactoryPointer<BearRHI::BearRHIPipeline> pipeline);
	virtual void SetDescriptorHeap(BearFactoryPointer<BearRHI::BearRHIDescriptorHeap> descriptor_heap);
	virtual void SetVertexBuffer(BearFactoryPointer<BearRHI::BearRHIVertexBuffer> buffer);
	virtual void SetIndexBuffer(BearFactoryPointer<BearRHI::BearRHIIndexBuffer> buffer);
	virtual void SetViewport(float x, float y, float width, float height, float min_depth = 0.f, float max_depth = 1.f);
	virtual void SetScissor(bool enable, float x, float y, float x1, float y1);
	virtual void SetStencilRef(uint32 ref);

	virtual void Draw(size_t count, size_t offset = 0);
	virtual void DrawIndex(size_t count, size_t  offset_index = 0, size_t  offset_vertex = 0);
	virtual void DispatchMesh(size_t count_x, size_t count_y, size_t count_z);
	virtual void DispatchRays(bsize count_x, bsize count_y, bsize count_z, BearFactoryPointer<BearRHI::BearRHIRayTracingShaderTable> shader_table);

	virtual void Lock(BearFactoryPointer<BearRHI::BearRHIViewport> viewport);
	virtual void Unlock(BearFactoryPointer<BearRHI::BearRHIViewport> viewport);
	virtual void Lock(BearFactoryPointer<BearRHI::BearRHIFrameBuffer> frame_buffer);
	virtual void Unlock(BearFactoryPointer<BearRHI::BearRHIFrameBuffer> frame_buffer);
	virtual void Lock(BearFactoryPointer<BearRHI::BearRHIUnorderedAccess> unordered_access);
	virtual void Unlock(BearFactoryPointer<BearRHI::BearRHIUnorderedAccess> unordered_access);
private:
	bool m_UseRenderPass;
	VkViewport m_Viewport;
	VkCommandPool m_CommandPool;
	VkCommandBuffer m_CommandBuffer;
	VkSemaphore m_SemaphoreWait;
	VkFence m_Fence;
	BearPipelineType m_PipelineType;
};