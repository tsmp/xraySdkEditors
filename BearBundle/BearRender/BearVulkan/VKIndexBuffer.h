#pragma once
class VKIndexBuffer :public BearRHI::BearRHIIndexBuffer
{
	//BEAR_CLASS_WITHOUT_COPY(VKIndexBuffer);
public:
	VKIndexBuffer();
	virtual void Create(size_t count, bool dynamic,void*data);
	virtual ~VKIndexBuffer();
	virtual uint32* Lock();
	virtual void Unlock();
	virtual void Clear();
	virtual size_t GetCount();
	VkBuffer Buffer;
	size_t Size;
private:
	VkDeviceMemory m_Memory;
	bool m_dynamic;

};