#pragma once
class VKSamplerState :public BearRHI::BearRHISampler
{
	//BEAR_CLASS_WITHOUT_COPY(VKSamplerState);
public:
	VKSamplerState(const BearSamplerDescription& description);
	virtual ~VKSamplerState(); 
	VkDescriptorImageInfo ImageInfo;
};