#pragma once
class BEARUI_API BearUIViewportBase:protected BearUIObject
{
	BEAR_CLASS_WITHOUT_COPY(BearUIViewportBase);
public:
	BearUIViewportBase();
	virtual ~BearUIViewportBase();
	virtual void Load() = 0;
	virtual void Unload();
	virtual void Push(BearUIObject* obj, bool auto_delete = true);
	virtual void Pop(BearUIObject* obj);
	uint32 GetTextureID(BearFactoryPointer<BearRHI::BearRHIShaderResource> SRV);
	void FreeTextureID(uint32 id);
protected:
	void Frame();
	void Render();
	void Load(BearRenderTargetFormat output_format);
	enum class ShaderType
	{
		DefaultPixel,
		DefaultVertex,
	};
	virtual void LoadShader(BearFactoryPointer<BearRHI::BearRHIShader>& Shader, ShaderType type);
protected:
	virtual void SetMousePosition(const BearFVector2& position) = 0;
	virtual BearFVector2 GetMousePosition()const = 0;
	virtual BearFVector2 GetSizeViewport()const = 0;
protected:
	void OnKeyDown(BearInput::Key key);
	void OnKeyUp(BearInput::Key key);
	void OnChar(bchar16 text);
private:
	void ImGuiInitialize();
	void ImGuiFrame();
	void ImGuiUpdateMouse();

	BearTimer m_Timer;
	ImGuiMouseCursor m_LastMouseCursor;

	BearFactoryPointer<BearRHI::BearRHIVertexBuffer> m_VertexBuffer;
	BearFactoryPointer<BearRHI::BearRHIIndexBuffer> m_IndexBuffer;
	BearFactoryPointer<BearRHI::BearRHITexture2D> m_TextureFont;
	BearFactoryPointer<BearRHI::BearRHIPipelineGraphics> m_Pipeline;
	BearFactoryPointer<BearRHI::BearRHIUniformBuffer> m_UniformBuffer;
	BearFactoryPointer<BearRHI::BearRHISampler> m_Sampler;
	BearFactoryPointer<BearRHI::BearRHIRootSignature> m_RootSignature;
	BearFactoryPointer<BearRHI::BearRHIDescriptorHeap> m_FontDescriptorHeap;
	BearMap<uint32, BearFactoryPointer<BearRHI::BearRHIDescriptorHeap>> m_HeapMap;
	BearMap<uint32, BearFactoryPointer<BearRHI::BearRHIShaderResource>> m_TextureMap;
	BearVector<uint32> m_TexturesFree;
protected:
	BearFactoryPointer<BearRHI::BearRHIContext> m_Context;
};