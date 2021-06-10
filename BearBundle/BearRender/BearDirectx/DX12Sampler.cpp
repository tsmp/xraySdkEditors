#include "DX12PCH.h"
bsize SamplerCounter = 0;

DX12SamplerState::DX12SamplerState(const BearSamplerDescription& description)
{
	SamplerCounter++;
	ZeroMemory(&SamplerDesc, sizeof(D3D12_SAMPLER_DESC));
	SamplerDesc.AddressU = DX12Factory::Translation(description.AddressU);
	SamplerDesc.AddressV = DX12Factory::Translation(description.AddressV);
	SamplerDesc.AddressW = DX12Factory::Translation(description.AddressW);
	SamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	memcpy(SamplerDesc.BorderColor, description.BorderColor.R32G32B32A32, 4*sizeof(float));
	SamplerDesc.MaxAnisotropy = static_cast<UINT>(description.MaxAnisotropy < 0 ? 1 : description.MaxAnisotropy);


	SamplerDesc.MipLODBias = static_cast<float>(description.MipBias);
	switch (description.Filter)
	{
	case	BearSamplerFilter::MinMagMipPoint:
		SamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		break;
	case	BearSamplerFilter::MinMagLinearMipPoint:
		SamplerDesc.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		break;
	case	BearSamplerFilter::MinMagMipLinear:
		SamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		break;
	case	BearSamplerFilter::Anisotropic:
		SamplerDesc.Filter = D3D12_FILTER_ANISOTROPIC;
		break;
	case	BearSamplerFilter::ComparisonMinMagMipPoint:
		SamplerDesc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
		break;
	case	BearSamplerFilter::ComparisonMinMagLinearMipPoint:
		SamplerDesc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
		break;
	case	BearSamplerFilter::ComparisonMinMagMipLinear:
		SamplerDesc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
		break;
	case	BearSamplerFilter::ComparisonAnisotropic:
		SamplerDesc.Filter = D3D12_FILTER_COMPARISON_ANISOTROPIC;
		break;
	default:
		BEAR_CHECK(0);
	}
	switch (description.Filter)
	{
	case	BearSamplerFilter::MinMagMipPoint:
	case	BearSamplerFilter::MinMagLinearMipPoint:
	case	BearSamplerFilter::MinMagMipLinear:
		SamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		break;
	case	BearSamplerFilter::Anisotropic:
		if (SamplerDesc.MaxAnisotropy == 1)
		{
			SamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		}
		SamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		break;
	case	BearSamplerFilter::ComparisonMinMagMipPoint:
	case	BearSamplerFilter::ComparisonMinMagLinearMipPoint:
	case	BearSamplerFilter::ComparisonMinMagMipLinear:
		SamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS;
		break;
	case	BearSamplerFilter::ComparisonAnisotropic:
		if (SamplerDesc.MaxAnisotropy == 1)
		{
			SamplerDesc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
		}
		SamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS;
	default:
		BEAR_CHECK(0);
	}
	ShaderResource = Factory->ReserveSamplersHeapAllocator.allocate(1, Factory->Device.Get());
	CD3DX12_CPU_DESCRIPTOR_HANDLE ÑpuDescriptorHandle(ShaderResource.DescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	ÑpuDescriptorHandle.Offset(Factory->CbvSrvUavDescriptorSize, static_cast<UINT>(ShaderResource.Id));
	Factory->Device->CreateSampler(&SamplerDesc, ÑpuDescriptorHandle);
}

DX12SamplerState::~DX12SamplerState()
{
	Factory->ReserveSamplersHeapAllocator.free(ShaderResource);
	SamplerCounter--;
}
