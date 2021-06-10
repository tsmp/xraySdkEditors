#include "DX12PCH.h"
bsize RenderPassCounter = 0;
DX12RenderPass::DX12RenderPass(const BearRenderPassDescription& description)
{
	Description = description;
	for (CountRenderTarget=0; Description.RenderTargets[CountRenderTarget].Format != BearRenderTargetFormat::None&& CountRenderTarget<8; CountRenderTarget++) {}
	RenderPassCounter++;

}

DX12RenderPass::~DX12RenderPass()
{
	RenderPassCounter--;
}
