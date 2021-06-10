#pragma once
class DX12Context :public BearRHI::BearRHIContext
{
public:
	DX12Context();
	virtual ~DX12Context();

	virtual void BeginEvent(char const* name, BearColor color = BearColor::White);
	virtual void EndEvent();

	virtual void Reset();
	virtual void Wait();
	virtual void Flush(BearFactoryPointer<BearRHI::BearRHIViewport> viewport,bool wait);
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
	virtual void Draw(bsize count, bsize offset = 0);
	virtual void DrawIndex(bsize count, bsize  offset_index = 0, bsize  offset_vertex = 0);
	virtual void DispatchRays( bsize count_x, bsize count_y, bsize count_z, BearFactoryPointer<BearRHI::BearRHIRayTracingShaderTable> shader_table);
	virtual void DispatchMesh(bsize count_x, bsize count_y, bsize count_z);

	virtual void Lock(BearFactoryPointer<BearRHI::BearRHIViewport> viewport);
	virtual void Unlock(BearFactoryPointer<BearRHI::BearRHIViewport> viewport);
	virtual void Lock(BearFactoryPointer<BearRHI::BearRHIFrameBuffer> frame_buffer);
	virtual void Unlock(BearFactoryPointer<BearRHI::BearRHIFrameBuffer> drame_buffer);
	virtual void Lock(BearFactoryPointer<BearRHI::BearRHIUnorderedAccess> unordered_access);
	virtual void Unlock(BearFactoryPointer<BearRHI::BearRHIUnorderedAccess> unordered_access);

private:
	ComPtr<ID3D12GraphicsCommandListX> m_CommandList;
	ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
	ComPtr<ID3D12CommandQueue> m_CommandQueue;
	HANDLE m_FenceEvent;
	ComPtr<ID3D12Fence> m_Fence;
	uint64 m_FenceValue;
	D3D12_RECT m_ScissorRect;
	bool m_CurrentPipelineIsCompute;
};