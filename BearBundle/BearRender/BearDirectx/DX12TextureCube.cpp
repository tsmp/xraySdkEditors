#include "DX12PCH.h"
bsize TextureCubeCounter = 0;
DX12TextureCube::DX12TextureCube(bsize width, bsize height, bsize mips, bsize count, BearTexturePixelFormat pixel_format, BearTextureUsage type_usage, void* data)
{
	TextureCubeCounter++;

	BEAR_ASSERT(type_usage != BearTextureUsage::Storage);
	m_Format = pixel_format;
	m_TextureUsage = type_usage;
	m_LockBuffer = 0;

	bear_fill(TextureDesc);
	TextureDesc.MipLevels = static_cast<UINT16>(mips);
	TextureDesc.Format = Factory->Translation(pixel_format);
	TextureDesc.Width = static_cast<uint32>(width);
	TextureDesc.Height = static_cast<uint32>(height);
	TextureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	TextureDesc.DepthOrArraySize = static_cast<UINT16>(count)*6;
	TextureDesc.SampleDesc.Count = 1;
	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	auto Properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	R_CHK(Factory->Device->CreateCommittedResource(&Properties,D3D12_HEAP_FLAG_NONE,&TextureDesc,D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,nullptr,IID_PPV_ARGS(&TextureBuffer)));
	
	
	
	m_ShaderResource = Factory->ReserveResourceHeapAllocator.allocate(1, Factory->Device.Get());

	bear_fill(DX12ShaderResource::SRV);
	DX12ShaderResource::SRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	DX12ShaderResource::SRV.Format = TextureDesc.Format;
	if (TextureDesc.DepthOrArraySize > 6)
	{
		DX12ShaderResource::SRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
		DX12ShaderResource::SRV.TextureCubeArray.MipLevels = static_cast<UINT>(mips);
		DX12ShaderResource::SRV.TextureCubeArray.NumCubes = static_cast<UINT>(count)/6;
	}
	else
	{
		DX12ShaderResource::SRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		DX12ShaderResource::SRV.TextureCube.MipLevels = static_cast<UINT>(mips);
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE ÑpuDescriptorHandle(m_ShaderResource.DescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	ÑpuDescriptorHandle.Offset(Factory->CbvSrvUavDescriptorSize, static_cast<UINT>(m_ShaderResource.Id));
	Factory->Device->CreateShaderResourceView(TextureBuffer.Get(), &(DX12ShaderResource::SRV), ÑpuDescriptorHandle);
	

	if (BearTextureUsage::Static != m_TextureUsage)AllocBuffer();

	if (data)
	{
		auto ptr = reinterpret_cast<uint8*>(data);
		for (bsize x = 0; x < count; x++)
			for (bsize y = 0; y < mips; y++)
			{
				bsize  size =BearTextureUtils::GetSizeDepth(BearTextureUtils::GetMip(static_cast<bsize>(TextureDesc.Width), y),BearTextureUtils::GetMip(static_cast<bsize>(TextureDesc.Height), y), pixel_format);
				memcpy(Lock(y, x), ptr, size);
				Unlock();
				ptr += size;
			}


	}
}

bool DX12TextureCube::SetAsSRV(D3D12_CPU_DESCRIPTOR_HANDLE& heap)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE ÑpuDescriptorHandle(m_ShaderResource.DescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	ÑpuDescriptorHandle.Offset(Factory->CbvSrvUavDescriptorSize, static_cast<UINT>(m_ShaderResource.Id));
	Factory->Device->CopyDescriptorsSimple(1, heap, ÑpuDescriptorHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	return true;
}

void* DX12TextureCube::QueryInterface(int type)
{
	switch (type)
	{
	case DX12Q_ShaderResource:
		return reinterpret_cast<void*>(static_cast<DX12ShaderResource*>(this));
	default:
		return nullptr;
	}
}

DX12TextureCube::~DX12TextureCube()
{
	--TextureCubeCounter;
	Factory->ReserveResourceHeapAllocator.free(m_ShaderResource);
}

void* DX12TextureCube::Lock(bsize mip, bsize depth)
{
	if(TextureBuffer.Get() == 0)return 0;
	if (m_LockBuffer)Unlock();
	m_LockMip = mip;
	m_LockDepth = depth;

	switch (m_TextureUsage)
	{
	case BearTextureUsage::Static:
		AllocBuffer();
		break;
	case BearTextureUsage::Dynamic:
		break;
	case BearTextureUsage::Stating:
		Factory->LockCommandList();
		auto ResourceBarrier1 = CD3DX12_RESOURCE_BARRIER::Transition(TextureBuffer.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_SOURCE);
		Factory->CommandList->ResourceBarrier(1, &ResourceBarrier1);
		{
			D3D12_SUBRESOURCE_FOOTPRINT PitchedDesc = {  };
			PitchedDesc.Format = TextureDesc.Format;
			PitchedDesc.Width = static_cast<UINT>(BearTextureUtils::GetMip(static_cast<bsize>(TextureDesc.Width), m_LockMip));
			PitchedDesc.Height = static_cast<UINT>(BearTextureUtils::GetMip(static_cast<bsize>(TextureDesc.Height), m_LockMip));
			if (BearTextureUtils::isCompressor(m_Format))
			{
				PitchedDesc.Width = BearMath::max(UINT(4), PitchedDesc.Width);
				PitchedDesc.Height = BearMath::max(UINT(4), PitchedDesc.Height);
			}
			PitchedDesc.Depth = 1;
			PitchedDesc.RowPitch = static_cast<UINT> (BearTextureUtils::GetSizeWidth(PitchedDesc.Width, m_Format));

			D3D12_PLACED_SUBRESOURCE_FOOTPRINT PlacedTexture2D = { 0 };
			PlacedTexture2D.Offset = 0;
			PlacedTexture2D.Footprint = PitchedDesc;

			CD3DX12_TEXTURE_COPY_LOCATION Src(TextureBuffer.Get(), D3D12CalcSubresource(static_cast<UINT>(m_LockMip), static_cast<UINT>(m_LockDepth), 0, TextureDesc.MipLevels, TextureDesc.DepthOrArraySize));
			CD3DX12_TEXTURE_COPY_LOCATION Dst(m_Buffer.Get(), PlacedTexture2D);
			Factory->CommandList->CopyTextureRegion(&Dst, 0, 0, 0, &Src, 0);
		}
		auto ResourceBarrier2 = CD3DX12_RESOURCE_BARRIER::Transition(TextureBuffer.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		Factory->CommandList->ResourceBarrier(1, &ResourceBarrier2);
		Factory->UnlockCommandList();
		break;
	default:
		break;
	}

	
	CD3DX12_RANGE ReadRange(0, 0);
	R_CHK(m_Buffer->Map(0, &ReadRange, reinterpret_cast<void**>(&m_LockBuffer)));
	return m_LockBuffer;
}

void DX12TextureCube::Unlock()
{
	if (TextureBuffer.Get() == 0)
		return;
	if (m_LockBuffer == 0)
		return;
	m_Buffer->Unmap(0, nullptr);

	switch (m_TextureUsage)
	{
	case BearTextureUsage::Static:
	case BearTextureUsage::Dynamic:
		Factory->LockCommandList();
		auto ResourceBarrier1 = CD3DX12_RESOURCE_BARRIER::Transition(TextureBuffer.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
		Factory->CommandList->ResourceBarrier(1, &ResourceBarrier1);
		{
			D3D12_SUBRESOURCE_FOOTPRINT PitchedDesc = {  };
			PitchedDesc.Format = TextureDesc.Format;
			PitchedDesc.Width = static_cast<UINT>(BearTextureUtils::GetMip(static_cast<bsize>(TextureDesc.Width), m_LockMip));
			PitchedDesc.Height = static_cast<UINT>(BearTextureUtils::GetMip(static_cast<bsize>(TextureDesc.Height), m_LockMip));
			if (BearTextureUtils::isCompressor(m_Format))
			{
				PitchedDesc.Width = BearMath::max(UINT(4), PitchedDesc.Width);
				PitchedDesc.Height = BearMath::max(UINT(4), PitchedDesc.Height);
			}
			PitchedDesc.Depth = 1;
			PitchedDesc.RowPitch = static_cast<UINT> (BearTextureUtils::GetSizeWidth(PitchedDesc.Width, m_Format));
			bsize Delta = ((PitchedDesc.RowPitch + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1)) - PitchedDesc.RowPitch;
			PitchedDesc.RowPitch = (PitchedDesc.RowPitch + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1);

			if (Delta)
			{
				auto Dest = m_LockBuffer;
				auto Source = m_LockBuffer;
				Dest += PitchedDesc.RowPitch * (BearTextureUtils::GetCountBlock(PitchedDesc.Height, m_Format) - 1);
				Source += (PitchedDesc.RowPitch - Delta) * (BearTextureUtils::GetCountBlock(PitchedDesc.Height, m_Format) - 1);
				for (bsize i = BearTextureUtils::GetCountBlock(PitchedDesc.Height, m_Format); i > 0; i--)
				{
					bear_move(Dest, Source, PitchedDesc.RowPitch - Delta);
					Dest -= PitchedDesc.RowPitch;
					Source -= (PitchedDesc.RowPitch - Delta);
				}
			}

			D3D12_PLACED_SUBRESOURCE_FOOTPRINT PlacedTexture2D = { 0 };
			PlacedTexture2D.Offset = 0;
			PlacedTexture2D.Footprint = PitchedDesc;

			CD3DX12_TEXTURE_COPY_LOCATION Dst(TextureBuffer.Get(), D3D12CalcSubresource(static_cast<UINT>(m_LockMip), static_cast<UINT>(m_LockDepth), 0, TextureDesc.MipLevels, TextureDesc.DepthOrArraySize));
			CD3DX12_TEXTURE_COPY_LOCATION Src(m_Buffer.Get(), PlacedTexture2D);
			Factory->CommandList->CopyTextureRegion(&Dst, 0, 0, 0, &Src, 0);
		}
		auto var4 = CD3DX12_RESOURCE_BARRIER::Transition(TextureBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		Factory->CommandList->ResourceBarrier(1, &var4);
		Factory->UnlockCommandList();
		break;
	case BearTextureUsage::Stating:
		break;
	default:
		break;
	}

	if (m_TextureUsage == BearTextureUsage::Static)
	{
		FreeBuffer();
	}
	m_LockBuffer = 0;
}


void DX12TextureCube::AllocBuffer()
{
	bsize SizeWidth = 0;
	bsize SizeDepth = 0;
	{
		SizeWidth = (BearTextureUtils::GetSizeWidth(static_cast<bsize>(TextureDesc.Width), m_Format));
		SizeWidth = (SizeWidth + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1);
		SizeDepth = SizeWidth * BearTextureUtils::GetCountBlock(static_cast<bsize>(TextureDesc.Height), m_Format);
		SizeDepth = (SizeDepth + D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT - 1) & ~(D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT - 1);
	}
	auto Properties = CD3DX12_HEAP_PROPERTIES(BearTextureUsage::Stating == m_TextureUsage ? D3D12_HEAP_TYPE_READBACK : D3D12_HEAP_TYPE_UPLOAD);
	auto ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(SizeDepth);
	R_CHK(Factory->Device->CreateCommittedResource(	&Properties,D3D12_HEAP_FLAG_NONE,&ResourceDesc,D3D12_RESOURCE_STATE_GENERIC_READ,nullptr,IID_PPV_ARGS(&m_Buffer)));
}

void DX12TextureCube::FreeBuffer()
{
	m_Buffer.Reset();
}
