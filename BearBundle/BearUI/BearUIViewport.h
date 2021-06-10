#pragma once
class BEARUI_API BearUIViewport :public BearWindow,public BearUIViewportBase
{
	BEAR_CLASS_WITHOUT_COPY(BearUIViewport);
public:
	BearUIViewport(bsize width = 0x400, bsize height = 0x300, bool fullscreen = false, BearFlags<int32> flags = 0);
	virtual ~BearUIViewport();
	virtual void BeginFrame();
	virtual void EndFrame();
public:
	virtual void Load();
	virtual void Unload();
	virtual void Render();
protected:

	virtual void SetMousePosition(const BearFVector2& position);
	virtual BearFVector2 GetMousePosition()const;
	virtual BearFVector2 GetSizeViewport()const;
	virtual void OnEvent(BearEventWindows& e);
protected:
	BearFactoryPointer<BearRHI::BearRHIViewport> m_Viewport;
};