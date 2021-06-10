#include "BearUI.hpp"

BearUIViewport::BearUIViewport(bsize width, bsize height, bool fullscreen, BearFlags<int32> flags):BearWindow(width,height,fullscreen,flags)
{
}

BearUIViewport::~BearUIViewport()
{
}

void BearUIViewport::BeginFrame()
{
	BearWindow::BeginFrame();
}

void BearUIViewport::EndFrame()
{
	BearWindow::EndFrame();
	Frame();
}

void BearUIViewport::Load()
{
	BearViewportDescription Description;
	Description.Clear = true;
	m_Viewport = BearRenderInterface::CreateViewport(GetWindowHandle(), GetSize().x, GetSize().y, IsFullScreen(), Description);
	BearUIViewportBase::Load(m_Viewport->GetFormat());
}

void BearUIViewport::Unload()
{
	m_Viewport.clear();
}

void BearUIViewport::Render()
{
	if (m_Context.empty())return;
	m_Context->Reset();
	m_Context->Lock(m_Viewport);

	m_Context->SetViewportAsFrameBuffer(m_Viewport);
	BearUIViewportBase::Render();

	m_Context->Unlock(m_Viewport);
	m_Context->Flush(m_Viewport, true);
}

void BearUIViewport::SetMousePosition(const BearFVector2& position)
{
	BearWindow::SetMousePosition(position);
}

BearFVector2 BearUIViewport::GetMousePosition() const
{
	return BearWindow::GetMousePosition();
}

BearFVector2 BearUIViewport::GetSizeViewport() const
{
	return GetSizeFloat();
}

void BearUIViewport::OnEvent(BearEventWindows& e)
{
	switch (e.Type)
	{
	case BearWindowEventType::KeyDown:
		OnKeyDown(e.Key);
		break;
	case BearWindowEventType::KeyUp:
		OnKeyUp(e.Key);
		break;
	case BearWindowEventType::Char:
		OnChar(e.Char);
		break;
	}
}
