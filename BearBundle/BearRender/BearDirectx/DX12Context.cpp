#include "DX12PCH.h"
#ifdef DEVELOPER_VERSION
#define USE_PIX
#include "pix3.h"
extern bool GDebugRender;
#endif

bsize ContextCounter = 0;
DX12Context::DX12Context()
{
    {
        D3D12_COMMAND_QUEUE_DESC QueueDesc = {};
        QueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

        R_CHK(Factory->Device->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&m_CommandQueue)));
    }
    R_CHK(Factory->Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocator)));
    R_CHK(Factory->Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_CommandList)));
    m_CommandList->Close();
    
    R_CHK(Factory->Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)));
    m_FenceValue = 1;
    m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (m_FenceEvent == nullptr)
    {
        R_CHK(HRESULT_FROM_WIN32(GetLastError()));
    }
    m_CurrentPipelineIsCompute = false;
    ContextCounter++;
}
DX12Context::~DX12Context()
{
    ContextCounter--;
    if (m_FenceValue)
        if (m_Fence->GetCompletedValue() < m_FenceValue - 1)
        {
            R_CHK(m_Fence->SetEventOnCompletion(m_FenceValue - 1, m_FenceEvent));
            WaitForSingleObject(m_FenceEvent, INFINITE);
        }
    CloseHandle(m_FenceEvent);
}

void DX12Context::BeginEvent(char const* name, BearColor color)
{
#ifdef DEVELOPER_VERSION
    if (GDebugRender)
    {
        PIXBeginEvent(m_CommandList.Get(), PIX_COLOR(color.R8U, color.G8U, color.B8U), name);
    }
#endif
}
void DX12Context::EndEvent()
{
#ifdef DEVELOPER_VERSION
    if (GDebugRender)
    {
        PIXEndEvent(m_CommandList.Get());
    }
#endif
}

void DX12Context::Reset()
{
    m_ScissorRect.left = 0;
    m_ScissorRect.right = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
    m_ScissorRect.top = 0;
    m_ScissorRect.bottom = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
    m_CommandAllocator->Reset();
    m_CommandList->Reset(m_CommandAllocator.Get(), nullptr);
    
}
void DX12Context::Wait()
{
    if (m_Fence->GetCompletedValue() < m_FenceValue - 1)
    {
        R_CHK(m_Fence->SetEventOnCompletion(m_FenceValue - 1, m_FenceEvent));
        WaitForSingleObject(m_FenceEvent, INFINITE);
    }
}
void DX12Context::Flush(BearFactoryPointer<BearRHI::BearRHIViewport> viewport, bool wait)
{
    m_CommandList->Close();
    ID3D12CommandList* ppm_commandLists[] = { m_CommandList.Get() };
    auto Viewport = static_cast<DX12Viewport*>(viewport.get());
    Viewport->CommandQueue->ExecuteCommandLists(_countof(ppm_commandLists), ppm_commandLists);
    Viewport->Swap();
    Viewport->CommandQueue->Signal(m_Fence.Get(), m_FenceValue++);
    if (wait)
        Wait();
}
void DX12Context::Flush(bool wait)
{
    m_CommandList->Close();
    ID3D12CommandList* PPMCommandLists[] = { m_CommandList.Get() };
    m_CommandQueue->ExecuteCommandLists(_countof(PPMCommandLists), PPMCommandLists);
    R_CHK(m_CommandQueue->Signal(m_Fence.Get(), m_FenceValue++));
    if (wait)
        Wait();
}

void DX12Context::ClearFrameBuffer()
{
    //m_CommandList->ClearState();
}

void DX12Context::SetPipeline(BearFactoryPointer<BearRHI::BearRHIPipeline> pipeline)
{
    DX12Pipeline* Pipeline = reinterpret_cast<DX12Pipeline*>(pipeline.get()->QueryInterface(DX12Q_Pipeline));
    BEAR_CHECK(Pipeline);
    Pipeline->Set(m_CommandList.Get());
    m_CurrentPipelineIsCompute = Pipeline->IsComputePipeline();
}
void DX12Context::SetViewportAsFrameBuffer(BearFactoryPointer<BearRHI::BearRHIViewport> viewport)
{
    BEAR_CHECK(!viewport.empty());

    auto Handle = static_cast<DX12Viewport*>(viewport.get())->GetHandle();
    if (static_cast<DX12Viewport*>(viewport.get())->Description.Clear)
    {
        D3D12_RECT Rect = { 0,0,static_cast<LONG>(static_cast<DX12Viewport*>(viewport.get())->Width),static_cast<LONG>(static_cast<DX12Viewport*>(viewport.get())->Height) };
        m_CommandList->ClearRenderTargetView(Handle, static_cast<DX12Viewport*>(viewport.get())->Description.ClearColor.R32G32B32A32, 1, &Rect);
    }
    m_CommandList->OMSetRenderTargets(1, &Handle, FALSE, nullptr);

}
void DX12Context::SetFrameBuffer(BearFactoryPointer<BearRHI::BearRHIFrameBuffer> frame_buffer)
{
    BEAR_CHECK(!frame_buffer.empty());
    auto FrameBuffer = static_cast<DX12FrameBuffer*>(frame_buffer.get());

    D3D12_RECT Rect = { 0,0,static_cast<LONG>(FrameBuffer->Width),static_cast<LONG>(FrameBuffer->Height) };
    for (bsize i = 0; i < FrameBuffer->Count; i++)
    {
        if (FrameBuffer->RenderPassRef->Description.RenderTargets[i].Clear)
            m_CommandList->ClearRenderTargetView(FrameBuffer->RenderTargetRefs[i], FrameBuffer->RenderPassRef->Description.RenderTargets[i].Color.R32G32B32A32, 1, &Rect);
    }
    if (!FrameBuffer->Description.DepthStencil.empty() && FrameBuffer->RenderPassRef->Description.DepthStencil.Clear)
    {
        D3D12_CLEAR_FLAGS flags = D3D12_CLEAR_FLAG_DEPTH;

        switch (FrameBuffer->RenderPassRef->Description.DepthStencil.Format)
        {
        case BearDepthStencilFormat::Depth24Stencil8:
        case BearDepthStencilFormat::Depth32FStencil8:
            flags |= D3D12_CLEAR_FLAG_STENCIL;
        default:
            break;
        }
        m_CommandList->ClearDepthStencilView(FrameBuffer->DepthStencilRef, flags, FrameBuffer->RenderPassRef->Description.DepthStencil.Depth, FrameBuffer->RenderPassRef->Description.DepthStencil.Stencil, 1, &Rect);
    }
    m_CommandList->OMSetRenderTargets(static_cast<UINT>(FrameBuffer->Count), FrameBuffer->RenderTargetRefs, FALSE, FrameBuffer->Description.DepthStencil.empty() ? nullptr : &FrameBuffer->DepthStencilRef);

}
void DX12Context::SetDescriptorHeap(BearFactoryPointer<BearRHI::BearRHIDescriptorHeap> descriptor_heap)
{
    if (m_CurrentPipelineIsCompute)
    {
        static_cast<DX12DescriptorHeap*>(descriptor_heap.get())->SetCompute(m_CommandList.Get());
    }
    else
    {
        static_cast<DX12DescriptorHeap*>(descriptor_heap.get())->SetGraphics(m_CommandList.Get());// DescriptorHeap
    }
}
void DX12Context::SetVertexBuffer(BearFactoryPointer<BearRHI::BearRHIVertexBuffer> buffer)
{
    m_CommandList->IASetVertexBuffers(0, 1, &static_cast<DX12VertexBuffer*>(buffer.get())->VertexBufferView);

}
void DX12Context::SetIndexBuffer(BearFactoryPointer<BearRHI::BearRHIIndexBuffer> buffer)
{
    m_CommandList->IASetIndexBuffer(&static_cast<DX12IndexBuffer*>(buffer.get())->IndexBufferView);

}
void DX12Context::SetViewport(float x, float y, float width, float height, float min_depth, float max_depth)
{
    D3D12_VIEWPORT rect;
    rect.TopLeftX = x;
    rect.TopLeftY = y;
    rect.Width = width;
    rect.Height = height;

    rect.MinDepth = min_depth;
    rect.MaxDepth = max_depth;
    m_CommandList->RSSetViewports(1, &rect);
}
void DX12Context::SetScissor(bool enable, float x, float y, float x1, float y1)
{
    if (enable)
    {
        m_ScissorRect.left = static_cast<LONG>(x);
        m_ScissorRect.right = static_cast<LONG>(x1);
        m_ScissorRect.top = static_cast<LONG>(y);
        m_ScissorRect.bottom = static_cast<LONG>(y1);
    }
    else
    {
        m_ScissorRect.left = 0;
        m_ScissorRect.right = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
        m_ScissorRect.top = 0;
        m_ScissorRect.bottom = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
    }

    m_CommandList->RSSetScissorRects(1, &m_ScissorRect);
}
void DX12Context::SetStencilRef(uint32 ref)
{
    m_CommandList->OMSetStencilRef(ref);
}
void DX12Context::Draw(bsize count, bsize offset)
{
    if (m_ScissorRect.left - m_ScissorRect.right == 0)return;
    if (m_ScissorRect.top - m_ScissorRect.bottom == 0)return;
    m_CommandList->DrawInstanced(static_cast<UINT>(count), 1, static_cast<UINT>(offset), 0);
}
void DX12Context::DrawIndex(bsize count, bsize offset_index, bsize  offset_vertex)
{
    if (m_ScissorRect.left - m_ScissorRect.right == 0)return;
    if (m_ScissorRect.top - m_ScissorRect.bottom == 0)return;
    m_CommandList->DrawIndexedInstanced(static_cast<UINT>(count), 1, static_cast<UINT>(offset_index), static_cast<UINT>(offset_vertex), 0);

}

void DX12Context::DispatchRays(bsize count_x, bsize count_y, bsize count_z, BearFactoryPointer<BearRHI::BearRHIRayTracingShaderTable> shader_table)
{
#ifdef RTX
    D3D12_DISPATCH_RAYS_DESC Desc = {};

    auto* ShaderTable = static_cast<DX12RayTracingShaderTable*>(shader_table.get());
    Desc.RayGenerationShaderRecord = ShaderTable->RayGenerationShaderRecord;
    Desc.MissShaderTable = ShaderTable->MissShaderTable;
    Desc.HitGroupTable = ShaderTable->HitGroupTable;
    Desc.CallableShaderTable = ShaderTable->CallableShaderTable;

    Desc.Width = static_cast<UINT>(count_x);
    Desc.Height = static_cast<UINT>(count_y);
    Desc.Depth = static_cast<UINT>(count_z);
    m_CommandList->DispatchRays(&Desc);
#endif
}

void DX12Context::DispatchMesh(bsize count_x, bsize count_y, bsize count_z)
{
#ifdef MESH_SHADING
    m_CommandList->DispatchMesh(static_cast<UINT>(count_x), static_cast<UINT>(count_y), static_cast<UINT>(count_z));
#endif
}

void DX12Context::Lock(BearFactoryPointer<BearRHI::BearRHIViewport> viewport)
{
    BEAR_CHECK(!viewport.empty());
    static_cast<DX12Viewport*>(viewport.get())->Lock(m_CommandList.Get());
}
void DX12Context::Unlock(BearFactoryPointer<BearRHI::BearRHIViewport> viewport)
{
    BEAR_CHECK(!viewport.empty());
    static_cast<DX12Viewport*>(viewport.get())->Unlock(m_CommandList.Get());
}
void DX12Context::Lock(BearFactoryPointer<BearRHI::BearRHIFrameBuffer> frame_buffer)
{
    BEAR_CHECK(!frame_buffer.empty());
    static_cast<DX12FrameBuffer*>(frame_buffer.get())->Lock(m_CommandList.Get());
}
void DX12Context::Unlock(BearFactoryPointer<BearRHI::BearRHIFrameBuffer> frame_buffer)
{
    BEAR_CHECK(!frame_buffer.empty());
    static_cast<DX12FrameBuffer*>(frame_buffer.get())->Unlock(m_CommandList.Get());
}

void DX12Context::Copy(BearFactoryPointer<BearRHI::BearRHIIndexBuffer> dest, BearFactoryPointer<BearRHI::BearRHIIndexBuffer> source)
{
    if (dest.empty() || source.empty())return;
    if (static_cast<DX12IndexBuffer*>(dest.get())->IndexBuffer.Get() == nullptr)return;
    if (static_cast<DX12IndexBuffer*>(source.get())->IndexBuffer.Get() == nullptr)return;
    auto var1 = CD3DX12_RESOURCE_BARRIER::Transition(static_cast<DX12IndexBuffer*>(dest.get())->IndexBuffer.Get(), D3D12_RESOURCE_STATE_INDEX_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
    m_CommandList->ResourceBarrier(1, &var1);
    m_CommandList->CopyBufferRegion(static_cast<DX12IndexBuffer*>(dest.get())->IndexBuffer.Get(), 0, static_cast<DX12IndexBuffer*>(source.get())->IndexBuffer.Get(), 0, static_cast<DX12IndexBuffer*>(dest.get())->IndexBufferView.SizeInBytes);
    auto var2 = CD3DX12_RESOURCE_BARRIER::Transition(static_cast<DX12IndexBuffer*>(dest.get())->IndexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
    m_CommandList->ResourceBarrier(1, &var2);
}
void DX12Context::Copy(BearFactoryPointer<BearRHI::BearRHITexture2D> dest, BearFactoryPointer<BearRHI::BearRHITexture2D> source)
{
    {
        if (dest.empty() || source.empty())return;
        if (static_cast<DX12Texture2D*>(dest.get())->TextureBuffer.Get() == nullptr)return;
        if (static_cast<DX12Texture2D*>(source.get())->TextureBuffer.Get() == nullptr)return;
        auto dst = static_cast<DX12Texture2D*>(dest.get());
        auto src = static_cast<DX12Texture2D*>(source.get());
        if (src->TextureDesc.Width!= dst->TextureDesc.Width)return;
        if (src->TextureDesc.Height != dst->TextureDesc.Height)return;;
        if (src->TextureDesc.DepthOrArraySize != dst->TextureDesc.DepthOrArraySize)return;;
        if (dst->TextureDesc.MipLevels!= 1)
            if (src->TextureDesc.MipLevels != dst->TextureDesc.MipLevels)return;
        auto var1 = CD3DX12_RESOURCE_BARRIER::Transition(dst->TextureBuffer.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
        auto var2 = CD3DX12_RESOURCE_BARRIER::Transition(src->TextureBuffer.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_SOURCE);
        auto var3 = CD3DX12_RESOURCE_BARRIER::Transition(dst->TextureBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        auto var4 = CD3DX12_RESOURCE_BARRIER::Transition(src->TextureBuffer.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        m_CommandList->ResourceBarrier(1, &var1);
        m_CommandList->ResourceBarrier(1, &var2);
        CD3DX12_TEXTURE_COPY_LOCATION DSTBuffer[512];
        CD3DX12_TEXTURE_COPY_LOCATION SRCBuffer[512];
        for (bsize x = 0; x < dst->TextureDesc.DepthOrArraySize; x++)
            for (bsize y = 0; y < dst->TextureDesc.MipLevels; y++)
            {
                CD3DX12_TEXTURE_COPY_LOCATION DST(dst->TextureBuffer.Get(), D3D12CalcSubresource(static_cast<UINT>(y), static_cast<UINT>(x), 0, dst->TextureDesc.MipLevels, dst->TextureDesc.DepthOrArraySize));
                CD3DX12_TEXTURE_COPY_LOCATION SRC(src->TextureBuffer.Get(), D3D12CalcSubresource(static_cast<UINT>(y), static_cast<UINT>(x), 0, src->TextureDesc.MipLevels, src->TextureDesc.DepthOrArraySize));
                DSTBuffer[x * dst->TextureDesc.MipLevels + y] = DST;
                SRCBuffer[x * dst->TextureDesc.MipLevels + y] = SRC;
                m_CommandList->CopyTextureRegion(&DSTBuffer[x * dst->TextureDesc.MipLevels + y], 0, 0, 0, &SRCBuffer[x * dst->TextureDesc.MipLevels + y], 0);
            }
        m_CommandList->ResourceBarrier(1, &var3);
        m_CommandList->ResourceBarrier(1, &var4);
    }
}
void DX12Context::Copy(BearFactoryPointer<BearRHI::BearRHIVertexBuffer> dest, BearFactoryPointer<BearRHI::BearRHIVertexBuffer> source)
{
    if (dest.empty() || source.empty())return;
    if (static_cast<DX12VertexBuffer*>(dest.get())->VertexBuffer.Get() == nullptr)return;
    if (static_cast<DX12VertexBuffer*>(source.get())->VertexBuffer.Get() == nullptr)return;
    auto var1 = CD3DX12_RESOURCE_BARRIER::Transition(static_cast<DX12VertexBuffer*>(dest.get())->VertexBuffer.Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
    m_CommandList->ResourceBarrier(1, &var1);
    m_CommandList->CopyBufferRegion(static_cast<DX12VertexBuffer*>(dest.get())->VertexBuffer.Get(), 0, static_cast<DX12VertexBuffer*>(source.get())->VertexBuffer.Get(), 0, static_cast<DX12VertexBuffer*>(dest.get())->VertexBufferView.SizeInBytes);
    auto var2 = CD3DX12_RESOURCE_BARRIER::Transition(static_cast<DX12VertexBuffer*>(dest.get())->VertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    m_CommandList->ResourceBarrier(1, &var2);

}
void DX12Context::Copy(BearFactoryPointer<BearRHI::BearRHIUniformBuffer> dest, BearFactoryPointer<BearRHI::BearRHIUniformBuffer> source)
{
    if (dest.empty() || source.empty())return;
    if (static_cast<DX12UniformBuffer*>(dest.get())->UniformBuffer.Get() == nullptr)return;
    if (static_cast<DX12UniformBuffer*>(source.get())->UniformBuffer.Get() == nullptr)return;
    auto var1 = CD3DX12_RESOURCE_BARRIER::Transition(static_cast<DX12UniformBuffer*>(dest.get())->UniformBuffer.Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
    m_CommandList->ResourceBarrier(1, &var1);
    m_CommandList->CopyBufferRegion(static_cast<DX12UniformBuffer*>(dest.get())->UniformBuffer.Get(), 0, static_cast<DX12UniformBuffer*>(source.get())->UniformBuffer.Get(), 0, static_cast<DX12UniformBuffer*>(dest.get())->GetCount()* static_cast<DX12UniformBuffer*>(dest.get())->GetStride());
    auto var2 = CD3DX12_RESOURCE_BARRIER::Transition(static_cast<DX12UniformBuffer*>(dest.get())->UniformBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    m_CommandList->ResourceBarrier(1, &var2);

}
void DX12Context::Lock(BearFactoryPointer<BearRHI::BearRHIUnorderedAccess> unordered_access)
{
    BEAR_CHECK(!unordered_access.empty());
    DX12UnorderedAccess* UnorderedAccess = reinterpret_cast<DX12UnorderedAccess*>(unordered_access.get()->QueryInterface(DX12Q_UnorderedAccess));
    UnorderedAccess->LockUAV(m_CommandList.Get());
}
void DX12Context::Unlock(BearFactoryPointer<BearRHI::BearRHIUnorderedAccess> unordered_access)
{
    BEAR_CHECK(!unordered_access.empty());
    DX12UnorderedAccess* UnorderedAccess = reinterpret_cast<DX12UnorderedAccess*>(unordered_access.get()->QueryInterface(DX12Q_UnorderedAccess));
    UnorderedAccess->UnlockUAV(m_CommandList.Get());
}