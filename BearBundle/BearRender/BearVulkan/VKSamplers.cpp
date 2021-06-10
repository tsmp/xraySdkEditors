#include "VKPCH.h"
size_t SamplerCounter = 0;
VKSamplerState::VKSamplerState(const BearSamplerDescription& description)
{
    VkSamplerCreateInfo SamplerCreateInfo = {};
    SamplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

	SamplerCreateInfo.addressModeU = VKFactory::Translation(description.AddressU);
	SamplerCreateInfo.addressModeV = VKFactory::Translation(description.AddressV);
	SamplerCreateInfo.addressModeW = VKFactory::Translation(description.AddressW);

	switch (description.Filter)
	{
	case	BearSamplerFilter::MinMagMipPoint:
		SamplerCreateInfo.magFilter = VK_FILTER_NEAREST;
		SamplerCreateInfo.minFilter = VK_FILTER_NEAREST;
		SamplerCreateInfo.anisotropyEnable = VK_FALSE;
		SamplerCreateInfo.compareEnable = VK_FALSE;
		SamplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		SamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		break;
	case	BearSamplerFilter::MinMagLinearMipPoint:
		SamplerCreateInfo.magFilter = VK_FILTER_LINEAR;
		SamplerCreateInfo.minFilter = VK_FILTER_LINEAR;
		SamplerCreateInfo.anisotropyEnable = VK_FALSE;
		SamplerCreateInfo.compareEnable = VK_FALSE;
		SamplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		SamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		break;
	case	BearSamplerFilter::MinMagMipLinear:
		SamplerCreateInfo.magFilter = VK_FILTER_LINEAR;
		SamplerCreateInfo.minFilter = VK_FILTER_LINEAR;
		SamplerCreateInfo.anisotropyEnable = VK_FALSE;
		SamplerCreateInfo.compareEnable = VK_FALSE;
		SamplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		SamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		break;
	case	BearSamplerFilter::Anisotropic:
		SamplerCreateInfo.magFilter = VK_FILTER_LINEAR;
		SamplerCreateInfo.minFilter = VK_FILTER_LINEAR;
		SamplerCreateInfo.anisotropyEnable = VK_TRUE;
		SamplerCreateInfo.compareEnable = VK_FALSE;
		SamplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		SamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		break;
	case	BearSamplerFilter::ComparisonMinMagMipPoint:
		SamplerCreateInfo.magFilter = VK_FILTER_NEAREST;
		SamplerCreateInfo.minFilter = VK_FILTER_NEAREST;
		SamplerCreateInfo.anisotropyEnable = VK_FALSE;
		SamplerCreateInfo.compareEnable = VK_TRUE;
		SamplerCreateInfo.compareOp = VK_COMPARE_OP_LESS;
		SamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		break;
	case	BearSamplerFilter::ComparisonMinMagLinearMipPoint:
		SamplerCreateInfo.magFilter = VK_FILTER_LINEAR;
		SamplerCreateInfo.minFilter = VK_FILTER_LINEAR;
		SamplerCreateInfo.anisotropyEnable = VK_FALSE;
		SamplerCreateInfo.compareEnable = VK_TRUE;
		SamplerCreateInfo.compareOp = VK_COMPARE_OP_LESS;
		SamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		break;
	case	BearSamplerFilter::ComparisonMinMagMipLinear:
		SamplerCreateInfo.magFilter = VK_FILTER_LINEAR;
		SamplerCreateInfo.minFilter = VK_FILTER_LINEAR;
		SamplerCreateInfo.anisotropyEnable = VK_FALSE;
		SamplerCreateInfo.compareEnable = VK_TRUE;
		SamplerCreateInfo.compareOp = VK_COMPARE_OP_LESS;
		SamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		break;
	case	BearSamplerFilter::ComparisonAnisotropic:
		SamplerCreateInfo.magFilter = VK_FILTER_LINEAR;
		SamplerCreateInfo.minFilter = VK_FILTER_LINEAR;
		SamplerCreateInfo.anisotropyEnable = VK_TRUE;
		SamplerCreateInfo.compareEnable = VK_TRUE;
		SamplerCreateInfo.compareOp = VK_COMPARE_OP_LESS;
		SamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		break;
	default:
		BEAR_CHECK(0);
	}
	SamplerCreateInfo.mipLodBias = static_cast<float>(description.MipBias);
	SamplerCreateInfo.maxLod = 3.402823466e+38f;
    SamplerCreateInfo.addressModeU = VKFactory::Translation(description.AddressU);
    SamplerCreateInfo.addressModeV = VKFactory::Translation(description.AddressV);
    SamplerCreateInfo.addressModeW = VKFactory::Translation(description.AddressW);

    SamplerCreateInfo.maxAnisotropy =static_cast<float>( description.MaxAnisotropy);
    SamplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    SamplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

    SamplerCounter++;
    V_CHK(vkCreateSampler(Factory->Device, &SamplerCreateInfo, nullptr, &ImageInfo.sampler));
}

VKSamplerState::~VKSamplerState()
{
    SamplerCounter--;
	vkDestroySampler(Factory->Device, ImageInfo.sampler, nullptr);
}
