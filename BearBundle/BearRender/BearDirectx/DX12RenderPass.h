#pragma once 
class DX12RenderPass:public BearRHI::BearRHIRenderPass
{
public:
	DX12RenderPass(const BearRenderPassDescription& Description);
	virtual ~DX12RenderPass();
	BearRenderPassDescription Description;
	bsize CountRenderTarget;
};