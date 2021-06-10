#include "BearUI.hpp"
///////////////////////////////////////////////////////////////////
struct ImGuiVertex
{
    float x, y;
    float r, g, b, a;
    float u, v;
};
///////////////////////////////////////////////////////////////////
BearUIViewportBase::BearUIViewportBase()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    m_LastMouseCursor = -1;
    ImGuiInitialize();
    //m_TextureFont = BearRenderInterface::CreateTexture2D()
}

BearUIViewportBase::~BearUIViewportBase()
{
    ImGui::DestroyContext();
}

void BearUIViewportBase::Push(BearUIObject* obj, bool auto_delete)
{
    BearUIObject::Push(obj, auto_delete);
}

void BearUIViewportBase::Pop(BearUIObject* obj)
{
    BearUIObject::Pop(obj);
}

uint32 BearUIViewportBase::GetTextureID(BearFactoryPointer<BearRHI::BearRHIShaderResource> SRV)
{
    for (auto b = m_TextureMap.begin(), e = m_TextureMap.end(); b != e; b++)
    {
        if (b->second == SRV)
        {
            return b->first;
        }
    }
    m_TextureMap.insert(static_cast<uint32>(m_TextureMap.size()) + 1, SRV);
    return static_cast<uint32>(m_TextureMap.size());
}

void BearUIViewportBase::FreeTextureID(uint32 id)
{
    auto ItemSRV = m_TextureMap.find(id);
    BEAR_VERIFY(ItemSRV != m_TextureMap.end());

    m_TexturesFree.push_back(id);

    m_TextureMap.erase(ItemSRV);

    auto ItemHEAP = m_HeapMap.find(id);
    if (m_HeapMap.end() != ItemHEAP)
        m_HeapMap.erase(ItemHEAP);
    if (m_TextureMap.empty())
    {
        m_TexturesFree.clear();
    }
}

void BearUIViewportBase::Unload()
{
    BEAR_VERIFY(m_TextureMap.empty());
    BEAR_VERIFY(m_HeapMap.empty());
    m_Context.clear();
    m_VertexBuffer.clear();
    m_IndexBuffer.clear();
    m_TextureFont.clear();
    m_Pipeline.clear();
    m_UniformBuffer.clear();
    m_Sampler.clear();
    m_RootSignature.clear();
    m_FontDescriptorHeap.clear();
    m_HeapMap.clear();
    m_TextureMap.clear();
    m_TexturesFree.clear();
}


void BearUIViewportBase::Frame()
{
    if (m_Context.empty())return;
    ImGuiFrame();
    ImGui::NewFrame();
    BearUIObject::Draw();
    ImGui::EndFrame();
}
void BearUIViewportBase::Render()
{
    ImGui::Render();
    ImDrawData*DrawData = ImGui::GetDrawData();
    if (DrawData->TotalVtxCount == 0|| DrawData->TotalIdxCount == 0)return;
    if (DrawData->DisplaySize.x <= 0.0f || DrawData->DisplaySize.y <= 0.0f)return;
    if (static_cast<bsize>(DrawData->TotalVtxCount)> m_VertexBuffer->GetCount())
    {
        m_VertexBuffer->Create(sizeof(ImGuiVertex), DrawData->TotalVtxCount, true);
    }
    if (static_cast<bsize>(DrawData->TotalIdxCount) > m_IndexBuffer->GetCount())
    {
        m_IndexBuffer->Create(static_cast<bsize>(DrawData->TotalIdxCount), true);
    }
    {
        ImGuiVertex* VertexDest = (ImGuiVertex*) m_VertexBuffer->Lock();
        uint32* IndexDest = m_IndexBuffer->Lock();
        for (bsize n = 0; n < static_cast<bsize>(DrawData->CmdListsCount); n++)
        {
            const ImDrawList* CmdList = DrawData->CmdLists[n];
            const ImDrawVert* VertexSource = CmdList->VtxBuffer.Data;
            for (bsize i = 0; i < static_cast<bsize>(CmdList->VtxBuffer.Size); i++)
            {
                BearColor ColorSource(VertexSource->col);
                VertexDest->x =VertexSource->pos.x;
                VertexDest->y =VertexSource->pos.y;
                VertexDest->r = ColorSource.R32G32B32A32[0];
                VertexDest->g = ColorSource.R32G32B32A32[1];
                VertexDest->b = ColorSource.R32G32B32A32[2];
                VertexDest->a = ColorSource.R32G32B32A32[3];
                VertexDest->u  = VertexSource->uv.x;
                VertexDest->v = VertexSource->uv.y;
                VertexDest++;
                VertexSource++;
            }
            memcpy(IndexDest, CmdList->IdxBuffer.Data, CmdList->IdxBuffer.Size * sizeof(ImDrawIdx));
            IndexDest += CmdList->IdxBuffer.Size;
        }
        m_VertexBuffer->Unlock();
        m_IndexBuffer->Unlock();
    }
    {

        float* ScreenSize = (float*)m_UniformBuffer->Lock();
        ScreenSize[0] = static_cast<float>(DrawData->DisplaySize.x);
        ScreenSize[1] = static_cast<float>(DrawData->DisplaySize.y);
        ScreenSize[2] = 1 / ScreenSize[0];
        ScreenSize[3] = 1 / ScreenSize[1];
        m_UniformBuffer->Unlock();
    }
    {
        for (bsize n = 0; n < static_cast<bsize>(DrawData->CmdListsCount); n++)
        {
            const ImDrawList* CmdList = DrawData->CmdLists[n];
            for (bsize i = 0; i < static_cast<bsize>(CmdList->CmdBuffer.Size); i++)
            {
                const ImDrawCmd* DrawCmd = &CmdList->CmdBuffer[static_cast<int>(i)];
                auto SRV = m_TextureMap[DrawCmd->TextureId];
                auto Item = m_HeapMap.find(DrawCmd->TextureId);
                if (Item == m_HeapMap.end())
                {
                    BearDescriptorHeapDescription Description;
                    Description.RootSignature = m_RootSignature;
                    auto DescriptorHeap = BearRenderInterface::CreateDescriptorHeap(Description);
                    DescriptorHeap->SetSampler(0, m_Sampler);
                    DescriptorHeap->SetShaderResource(0, SRV);
                    DescriptorHeap->SetUniformBuffer(0, m_UniformBuffer);
                    m_HeapMap.insert(DrawCmd->TextureId, DescriptorHeap);
                }
            }
        }
    }
    {
        ImVec2 ClipOffset = DrawData->DisplayPos;

        m_Context->BeginEvent("ImGui");
        m_Context->SetViewport(0, 0, static_cast<float>(DrawData->DisplaySize.x), static_cast<float>(DrawData->DisplaySize.y));
        m_Context->SetPipeline(m_Pipeline);
        
        bsize IndexOffset = 0;
        bsize VertexOffset = 0;
        for (bsize n = 0; n < static_cast<bsize>(DrawData->CmdListsCount); n++)
        {
            const ImDrawList* CmdList = DrawData->CmdLists[n];
            for (bsize i = 0; i < static_cast<bsize>(CmdList->CmdBuffer.Size); i++)
            {
                const ImDrawCmd* DrawCmd = &CmdList->CmdBuffer[static_cast<int>(i)];
                if (DrawCmd->UserCallback != NULL)
                {
                    // User callback, registered via ImDrawList::AddCallback()
                    // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
                    if (DrawCmd->UserCallback == ImDrawCallback_ResetRenderState)
                    {
                        m_Context->BeginEvent("ImGui");
                        m_Context->SetViewport(0, 0, static_cast<float>(DrawData->DisplaySize.x), static_cast<float>(DrawData->DisplaySize.y));
                        m_Context->SetPipeline(m_Pipeline);
                    }
                    else
                    {
                        m_Context->EndEvent();
                        DrawCmd->UserCallback(CmdList, DrawCmd);
                    }
                }
                else
                {
                    BEAR_VERIFY(DrawCmd->TextureId);
                    if (DrawCmd->TextureId == 1)
                    {
                        m_Context->SetDescriptorHeap(m_FontDescriptorHeap);
                    }
                    else
                    {
                        m_Context->SetDescriptorHeap(m_HeapMap[DrawCmd->TextureId]);
                    }
                    m_Context->SetVertexBuffer(m_VertexBuffer);
                    m_Context->SetIndexBuffer(m_IndexBuffer);
                    m_Context->SetScissor(true, static_cast<float>(DrawCmd->ClipRect.x - ClipOffset.x), static_cast<float>(DrawCmd->ClipRect.y - ClipOffset.y), static_cast<float>(DrawCmd->ClipRect.z - ClipOffset.x), static_cast<float>(DrawCmd->ClipRect.w - ClipOffset.y));
                    m_Context->DrawIndex(DrawCmd->ElemCount, DrawCmd->IdxOffset + IndexOffset, DrawCmd->VtxOffset + VertexOffset);
                }
            }

            IndexOffset += CmdList->IdxBuffer.Size;
            VertexOffset += CmdList->VtxBuffer.Size;
        }
        m_Context->EndEvent();
    }
    BearUIObject::Ñleanup();
}
void BearUIViewportBase::Load(BearRenderTargetFormat output_format)
{
    BearFactoryPointer<BearRHI::BearRHIRenderPass> RenderPass;

    m_Context = BearRenderInterface::CreateContext();

    auto PixelShader = BearRenderInterface::CreatePixelShader();
    LoadShader(PixelShader, ShaderType::DefaultPixel);
    auto VertexShader = BearRenderInterface::CreateVertexShader();
    LoadShader(VertexShader, ShaderType::DefaultVertex);

    m_VertexBuffer = BearRenderInterface::CreateVertexBuffer();
    m_IndexBuffer = BearRenderInterface::CreateIndexBuffer();

    {
        BearRenderPassDescription Description;
        Description.RenderTargets[0].Format = output_format;
        RenderPass = BearRenderInterface::CreateRenderPass(Description);
    }
    {
        BearRootSignatureDescription Description;
        Description.SRVResources[0].Shader = BearShaderType::Pixel;
        Description.Samplers[0].Shader = BearShaderType::Pixel;
        Description.UniformBuffers[0].Shader = BearShaderType::Vertex;
        m_RootSignature = BearRenderInterface::CreateRootSignature(Description);
    }


    {
        BearPipelineGraphicsDescription Description;
        Description.InputLayout.Elements[0] = BearInputLayoutElement("POSITION", BearVertexFormat::R32G32_FLOAT, 0);
        Description.InputLayout.Elements[1] = BearInputLayoutElement("COLOR", BearVertexFormat::R32G32B32A32_FLOAT, 8);
        Description.InputLayout.Elements[2] = BearInputLayoutElement("UV", BearVertexFormat::R32G32_FLOAT, 24);
        Description.RasterizerState.CullMode = BearRasterizerCullMode::None;
        Description.RenderPass = RenderPass;
        Description.RootSignature = m_RootSignature;

        Description.Shaders.Pixel = PixelShader;
        Description.Shaders.Vertex = VertexShader;
        Description.BlendState.RenderTarget[0].Enable = true;
        Description.BlendState.RenderTarget[0].ColorSrc = BearBlendFactor::SrcAlpha;
        Description.BlendState.RenderTarget[0].ColorDst = BearBlendFactor::InvSrcAlpha;

        m_Pipeline = BearRenderInterface::CreatePipelineGraphics(Description);
    }
    {
        ImGuiIO& io = ImGui::GetIO();
        unsigned char* tex_pixels = NULL;
        int tex_w, tex_h;
        io.Fonts->GetTexDataAsRGBA32(&tex_pixels, &tex_w, &tex_h);
        m_TextureFont = BearRenderInterface::CreateTexture2D(static_cast<bsize>(tex_w), static_cast<bsize>(tex_h), 1, 1, BearTexturePixelFormat::R8G8B8A8, BearTextureUsage::Static, tex_pixels);
        io.Fonts->TexID = 1;
    }
    {
        m_UniformBuffer = BearRenderInterface::CreateUniformBuffer(sizeof(float) * 4, 1, true);
    }
    {
        BearSamplerDescription Description;
        Description.Filter = BearSamplerFilter::MinMagMipPoint;
        m_Sampler = BearRenderInterface::CreateSampler(Description);
    }
    {
        BearDescriptorHeapDescription Description;
        Description.RootSignature = m_RootSignature;
        m_FontDescriptorHeap = BearRenderInterface::CreateDescriptorHeap(Description);
        m_FontDescriptorHeap->SetSampler(0, m_Sampler);
        m_FontDescriptorHeap->SetShaderResource(0, m_TextureFont);
        m_FontDescriptorHeap->SetUniformBuffer(0, m_UniformBuffer);
    }
    BEAR_ASSERT(GetTextureID(m_TextureFont) == 1);
}

void BearUIViewportBase::LoadShader(BearFactoryPointer<BearRHI::BearRHIShader>& Shader, ShaderType type)
{
    BearFileStream File;
    BearString Text, Error;
    BearMap<BearStringConteniar, BearStringConteniar> Defines;
    const bchar* Name = 0;
    switch (type)
    {
    case BearUIViewportBase::ShaderType::DefaultPixel:
        Name = "imgui.ps.hlsl";
        break;
    case BearUIViewportBase::ShaderType::DefaultVertex:
        Name = "imgui.vs.hlsl";
        break;
    default:
        BEAR_ASSERT(0);
        break;
    }
    BEAR_ASSERT(File.Open(Name));
    File.ToString(Text, BearEncoding::ANSI);

    BEAR_ASSERT(Shader->LoadAsText(*Text, TEXT("main"), Defines, Error, &GIncluderDefault));
    BearString new_file;
    new_file.append(Name).append(TEXT(".bin"));
    BEAR_ASSERT(File.Open(*new_file, File.M_Write));
    File.WriteBuffer(Shader->GetPointer(), Shader->GetSize());
}

void BearUIViewportBase::OnKeyDown(BearInput::Key key)
{
    ImGuiIO& io = ImGui::GetIO();
    switch (key)
    {
    case BearInput::KeyMouseLeft:
        io.MouseDown[0] = true;
        return;
    case BearInput::KeyMouseRight:
        io.MouseDown[1] = true;
        return;
    case BearInput::KeyMouseMiddle:
        io.MouseDown[2] = true;
        return;
    case BearInput::KeyMouseScrollUp:
    case BearInput::KeyMouseScrollDown:
        return;
    default:
        io.KeysDown[key] = true;
        break;
    }
}

void BearUIViewportBase::OnKeyUp(BearInput::Key key)
{
    ImGuiIO& io = ImGui::GetIO();
    switch (key)
    {
    case BearInput::KeyMouseLeft:
        io.MouseDown[0] = false;
        return;
    case BearInput::KeyMouseRight:
        io.MouseDown[1] = false;
        return;
    case BearInput::KeyMouseMiddle:
        io.MouseDown[2] = false;
        return;
    case BearInput::KeyMouseScrollUp:
    case BearInput::KeyMouseScrollDown:
        return;
    default:
        io.KeysDown[key] = false;
        break;
    }
}

void BearUIViewportBase::OnChar(bchar16 text)
{
    ImGuiIO& io = ImGui::GetIO();
    io.AddInputCharacterUTF16(text);
}

void BearUIViewportBase::ImGuiInitialize()
{
    {
        ImGuiIO& io = ImGui::GetIO();
        io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
        io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)
        io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;    // We can create multi-viewports on the Platform side (optional)
        io.BackendFlags |= ImGuiBackendFlags_HasMouseHoveredViewport; // We can set io.MouseHoveredViewport correctly (optional, not easy)
        io.BackendPlatformName = "BearUI";

        ImGuiViewport* main_viewport = ImGui::GetMainViewport();
        main_viewport->PlatformHandle = main_viewport->PlatformHandleRaw = nullptr;

        // Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array that we will update during the application lifetime.
        io.KeyMap[ImGuiKey_Tab] = BearInput::KeyTab;
        io.KeyMap[ImGuiKey_LeftArrow] = BearInput::KeyLeft;
        io.KeyMap[ImGuiKey_RightArrow] = BearInput::KeyRight;
        io.KeyMap[ImGuiKey_UpArrow] = BearInput::KeyUp;
        io.KeyMap[ImGuiKey_DownArrow] = BearInput::KeyDown;
        io.KeyMap[ImGuiKey_PageUp] = BearInput::KeyPageUp;
        io.KeyMap[ImGuiKey_PageDown] = BearInput::KeyPageDown;
        io.KeyMap[ImGuiKey_Home] = BearInput::KeyHome;
        io.KeyMap[ImGuiKey_End] = BearInput::KeyEnd;
        io.KeyMap[ImGuiKey_Insert] = BearInput::KeyInsert;
        io.KeyMap[ImGuiKey_Delete] = BearInput::KeyDelete;
        io.KeyMap[ImGuiKey_Backspace] = BearInput::KeyBackSpace;
        io.KeyMap[ImGuiKey_Space] = BearInput::KeySpace;
        io.KeyMap[ImGuiKey_Enter] = BearInput::KeyReturn;
        io.KeyMap[ImGuiKey_Escape] = BearInput::KeyEscape;
        io.KeyMap[ImGuiKey_KeyPadEnter] = BearInput::KeyReturn;
        io.KeyMap[ImGuiKey_A] = 'A';
        io.KeyMap[ImGuiKey_C] = 'C';
        io.KeyMap[ImGuiKey_V] = 'V';
        io.KeyMap[ImGuiKey_X] = 'X';
        io.KeyMap[ImGuiKey_Y] = 'Y';
        io.KeyMap[ImGuiKey_Z] = 'Z';

    }
}

void BearUIViewportBase::ImGuiFrame()
{
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.Fonts->IsBuilt() && "Font atlas not built! It is generally built by the renderer back-end. Missing call to renderer _NewFrame() function? e.g. ImGui_ImplOpenGL3_NewFrame().");

    auto SizeViewport = GetSizeViewport();
    io.DisplaySize = ImVec2(SizeViewport.x, SizeViewport.y);

    io.DeltaTime = m_Timer.get_elapsed_time().asseconds();
    m_Timer.restart();

        // Read keyboard modifiers inputs
    io.KeyCtrl = BearInput::KeyState(BearInput::KeyLControl)|| BearInput::KeyState(BearInput::KeyRControl);
    io.KeyShift = BearInput::KeyState(BearInput::KeyLShift) || BearInput::KeyState(BearInput::KeyRShift);
    io.KeyAlt = BearInput::KeyState(BearInput::KeyLAlt) || BearInput::KeyState(BearInput::KeyRAlt);
    io.KeySuper = false;
    // io.KeysDown[], io.MousePos, io.MouseDown[], io.MouseWheel: filled by the WndProc handler below.

    // Update OS mouse position
    ImGuiUpdateMouse();

    // Update OS mouse cursor with the cursor requested by imgui
    ImGuiMouseCursor MouseCursor = io.MouseDrawCursor ? ImGuiMouseCursor_None : ImGui::GetMouseCursor();
    if (m_LastMouseCursor != MouseCursor)
    {
        BearInput::Cursor Cursor;
        m_LastMouseCursor = MouseCursor;
        switch (MouseCursor)
        {
        case ImGuiMouseCursor_Arrow:
            Cursor = BearInput::Cursor_Arrow;
            break;
        case ImGuiMouseCursor_TextInput:
            Cursor = BearInput::Cursor_TextInput;
            break;
        case ImGuiMouseCursor_ResizeAll:
            Cursor = BearInput::Cursor_ResizeAll;
            break;
        case ImGuiMouseCursor_ResizeNS:
            Cursor = BearInput::Cursor_ResizeNS;
            break;
        case ImGuiMouseCursor_ResizeEW:
            Cursor = BearInput::Cursor_ResizeEW;
            break;
        case ImGuiMouseCursor_ResizeNESW:
            Cursor = BearInput::Cursor_ResizeNESW;
            break;
        case ImGuiMouseCursor_ResizeNWSE:
            Cursor = BearInput::Cursor_ResizeNWSE;
            break;
        case ImGuiMouseCursor_Hand:
            Cursor = BearInput::Cursor_Hand;
            break;
        case ImGuiMouseCursor_NotAllowed:
            Cursor = BearInput::Cursor_NotAllowed;
            break;
        default:
            Cursor = BearInput::Cursor_None;
            break;
        }
        BearInput::SetCursor(Cursor);
    }
}

void BearUIViewportBase::ImGuiUpdateMouse()
{
    ImGuiIO& io = ImGui::GetIO();

    // Set OS mouse position if requested (rarely used, only when ImGuiConfigFlags_NavEnableSetMousePos is enabled by user)
    // (When multi-viewports are enabled, all imgui positions are same as OS positions)
    if (io.WantSetMousePos)
    {
        if ((io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) == 0)
        {
        }
      SetMousePosition(BearFVector2(io.MousePos.x, io.MousePos.y));
    }

    io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
    io.MouseHoveredViewport =0;

    auto MousePos = GetMousePosition();
    io.MousePos = ImVec2(MousePos.x, MousePos.y);
}
