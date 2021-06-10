#include "DX12PCH.h"
bsize Texture2DCounter = 0;
DX12Texture2D::DX12Texture2D(bsize width, bsize height, bsize mips, bsize count, BearTexturePixelFormat pixel_format, BearTextureUsage type_usage, void* data )
{
	Texture2DCounter++;
	bAllowUAV = type_usage == BearTextureUsage::Storage;
	m_TextureType = BearTextureType::Default;
	m_Format = pixel_format;
	m_TextureUsage = type_usage;
	m_LockBuffer = 0;

	bear_fill(TextureDesc);
	TextureDesc.MipLevels = static_cast<UINT16>(mips);
	TextureDesc.Format = Factory->Translation(pixel_format);
	TextureDesc.Width = static_cast<uint32>(width);
	TextureDesc.Height = static_cast<uint32>(height);
	TextureDesc.Flags = type_usage == BearTextureUsage::Storage? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS:D3D12_RESOURCE_FLAG_NONE;
	TextureDesc.DepthOrArraySize = static_cast<UINT16>(count);
	TextureDesc.SampleDesc.Count = 1;
	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	auto Properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	m_ShaderResource = Factory->ReserveResourceHeapAllocator.allocate(bAllowUAV ? (TextureDesc.MipLevels+1) : 1, Factory->Device.Get());

	m_CurrentStates = bAllowUAV ? (D3D12_RESOURCE_STATE_UNORDERED_ACCESS) : (D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE| D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	R_CHK(Factory->Device->CreateCommittedResource(&Properties,D3D12_HEAP_FLAG_NONE,&TextureDesc, m_CurrentStates,nullptr,	IID_PPV_ARGS(&TextureBuffer)));
	
	{
		bear_fill(DX12ShaderResource::SRV);
		DX12ShaderResource::SRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		DX12ShaderResource::SRV.Format = TextureDesc.Format;
		if (TextureDesc.DepthOrArraySize > 1)
		{
			DX12ShaderResource::SRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
			DX12ShaderResource::SRV.Texture2DArray.MipLevels = static_cast<UINT>(mips);
			DX12ShaderResource::SRV.Texture2DArray.ArraySize = static_cast<UINT>(count);
		}
		else
		{
			DX12ShaderResource::SRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			DX12ShaderResource::SRV.Texture2D.MipLevels = static_cast<UINT>(mips);
		}
		CD3DX12_CPU_DESCRIPTOR_HANDLE ÑpuDescriptorHandle(m_ShaderResource.DescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		ÑpuDescriptorHandle.Offset(Factory->CbvSrvUavDescriptorSize, static_cast<UINT>(m_ShaderResource.Id));
		Factory->Device->CreateShaderResourceView(TextureBuffer.Get(), &(DX12ShaderResource::SRV), ÑpuDescriptorHandle);

	}
	if(bAllowUAV)
	{
		bear_fill(DX12UnorderedAccess::UAV);
		DX12UnorderedAccess::UAV.Format = TextureDesc.Format;
		if (TextureDesc.DepthOrArraySize > 1)
		{
			DX12UnorderedAccess::UAV.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
			DX12UnorderedAccess::UAV.Texture2DArray.ArraySize = static_cast<UINT>(count);
		}
		else
		{
			DX12UnorderedAccess::UAV.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		}

		CD3DX12_CPU_DESCRIPTOR_HANDLE ÑpuDescriptorHandle(m_ShaderResource.DescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		ÑpuDescriptorHandle.Offset(Factory->CbvSrvUavDescriptorSize, static_cast<UINT>(m_ShaderResource.Id + 1));
		for (bsize i = 0; i < mips; i++)
		{
			DX12UnorderedAccess::UAV.Texture2D.MipSlice =static_cast<UINT>( i);
			DX12UnorderedAccess::UAV.Texture2DArray.MipSlice = static_cast<UINT>(i);
			Factory->Device->CreateUnorderedAccessView(TextureBuffer.Get(), nullptr, &(DX12UnorderedAccess::UAV), ÑpuDescriptorHandle);
			ÑpuDescriptorHandle.Offset(Factory->CbvSrvUavDescriptorSize, static_cast<UINT>(1));
		}
		return;
	}
	

	if (BearTextureUsage::Static != m_TextureUsage)
		AllocBuffer();

	if (data)
	{
		auto ptr = reinterpret_cast<uint8*>(data);
		for (bsize x = 0; x < count; x++)
			for (bsize y = 0; y < mips; y++)
			{
				bsize  size =BearTextureUtils::GetSizeDepth(BearTextureUtils::GetMip(static_cast<bsize>(TextureDesc.Width), y),BearTextureUtils::GetMip(static_cast<bsize>(TextureDesc.Height), y), m_Format);
				memcpy(Lock(y, x), ptr, size);
				Unlock();
				ptr += size;
			}

	}
}

DX12Texture2D::DX12Texture2D(bsize width, bsize height, BearRenderTargetFormat pixel_format)
{
	Texture2DCounter++;

	m_TextureType = BearTextureType::RenderTarget;
	RTVFormat = pixel_format;
	m_TextureUsage = BearTextureUsage::Static;
	m_LockBuffer = 0;

	bear_fill(TextureDesc);
	TextureDesc.MipLevels = 1;
	TextureDesc.Format = DX12Factory::Translation(RTVFormat);
	TextureDesc.Width = static_cast<uint32>(width);
	TextureDesc.Height = static_cast<uint32>(height);
	TextureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	TextureDesc.DepthOrArraySize = 1;
	TextureDesc.SampleDesc.Count = 1;
	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	
	auto Properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	m_CurrentStates = (D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	R_CHK(Factory->Device->CreateCommittedResource(&Properties,D3D12_HEAP_FLAG_NONE,&TextureDesc,m_CurrentStates,nullptr,IID_PPV_ARGS(&TextureBuffer)));

	m_ShaderResource = Factory->ReserveResourceHeapAllocator.allocate(1, Factory->Device.Get());

	bear_fill(DX12ShaderResource::SRV);
	DX12ShaderResource::SRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	DX12ShaderResource::SRV.Format = TextureDesc.Format;
	DX12ShaderResource::SRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	DX12ShaderResource::SRV.Texture2D.MipLevels = static_cast<UINT>(1);
	CD3DX12_CPU_DESCRIPTOR_HANDLE ÑpuDescriptorHandle(m_ShaderResource.DescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	ÑpuDescriptorHandle.Offset(Factory->CbvSrvUavDescriptorSize, static_cast<UINT>(m_ShaderResource.Id));
	Factory->Device->CreateShaderResourceView(TextureBuffer.Get(), &(DX12ShaderResource::SRV), ÑpuDescriptorHandle);

}

DX12Texture2D::DX12Texture2D(bsize width, bsize height, BearDepthStencilFormat Format)
{
	Texture2DCounter++;

	m_TextureType = BearTextureType::DepthStencil;
	DSVFormat = Format;
	m_TextureUsage = BearTextureUsage::Static;
	m_LockBuffer = 0;

	bear_fill(TextureDesc);
	TextureDesc.MipLevels = 1;
	TextureDesc.Format = DX12Factory::Translation(DSVFormat);
	TextureDesc.Width = static_cast<uint32>(width);
	TextureDesc.Height = static_cast<uint32>(height);
	TextureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	TextureDesc.DepthOrArraySize = 1;
	TextureDesc.SampleDesc.Count = 1;
	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	CD3DX12_HEAP_PROPERTIES var1(D3D12_HEAP_TYPE_DEFAULT);

	auto Properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	R_CHK(Factory->Device->CreateCommittedResource(&Properties,D3D12_HEAP_FLAG_NONE,&TextureDesc,D3D12_RESOURCE_STATE_DEPTH_WRITE,nullptr,IID_PPV_ARGS(&TextureBuffer)));
	bear_fill(DX12ShaderResource::SRV);
}

bool DX12Texture2D::SetAsSRV(D3D12_CPU_DESCRIPTOR_HANDLE& heap)
{
	BEAR_CHECK(m_TextureType != BearTextureType::DepthStencil);
	CD3DX12_CPU_DESCRIPTOR_HANDLE ÑpuDescriptorHandle(m_ShaderResource.DescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	ÑpuDescriptorHandle.Offset(Factory->CbvSrvUavDescriptorSize, static_cast<UINT>(m_ShaderResource.Id));
	Factory->Device->CopyDescriptorsSimple(1, heap, ÑpuDescriptorHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	return true;
}

bool DX12Texture2D::SetAsUAV(D3D12_CPU_DESCRIPTOR_HANDLE& heap, bsize offset)
{
	if (!bAllowUAV)return false;
	BEAR_CHECK(m_TextureType != BearTextureType::DepthStencil);
	BEAR_CHECK(TextureDesc.MipLevels > offset);
	CD3DX12_CPU_DESCRIPTOR_HANDLE ÑpuDescriptorHandle(m_ShaderResource.DescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	ÑpuDescriptorHandle.Offset(Factory->CbvSrvUavDescriptorSize, static_cast<UINT>(m_ShaderResource.Id+1+ offset));
	Factory->Device->CopyDescriptorsSimple(1, heap, ÑpuDescriptorHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	return true;
}

void* DX12Texture2D::QueryInterface(int type)
{
	switch (type)
	{
	case DX12Q_ShaderResource:
		return reinterpret_cast<void*>(static_cast<DX12ShaderResource*>(this));
	case DX12Q_UnorderedAccess:
		if (!bAllowUAV)return nullptr;
		return reinterpret_cast<void*>(static_cast<DX12UnorderedAccess*>(this));
	default:
		return nullptr;
	}
}

DX12Texture2D::~DX12Texture2D()
{
	if(m_TextureType != BearTextureType::DepthStencil)
		Factory->ReserveResourceHeapAllocator.free(m_ShaderResource);
	--Texture2DCounter;
}

void* DX12Texture2D::Lock(bsize mip, bsize depth)
{
	BEAR_CHECK(m_TextureType == BearTextureType::Default);
	BEAR_CHECK(!bAllowUAV);

	if(TextureBuffer.Get() == 0)return 0;
	if (m_TextureType != BearTextureType::Default)return 0 ;
	if (bAllowUAV)return 0;
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
		auto ResourceBarrier1 = CD3DX12_RESOURCE_BARRIER::Transition(TextureBuffer.Get(), m_CurrentStates, D3D12_RESOURCE_STATE_COPY_SOURCE);
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
		auto ResourceBarrier2 = CD3DX12_RESOURCE_BARRIER::Transition(TextureBuffer.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, m_CurrentStates);
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

void DX12Texture2D::Unlock()
{
	if (TextureBuffer.Get() == 0)
		return;
	if (m_LockBuffer == 0)return;
	m_Buffer->Unmap(0, nullptr);

	switch (m_TextureUsage)
	{
	case BearTextureUsage::Static:
	case BearTextureUsage::Dynamic:
		Factory->LockCommandList();
		auto ResourceBarrier1 = CD3DX12_RESOURCE_BARRIER::Transition(TextureBuffer.Get(), m_CurrentStates, D3D12_RESOURCE_STATE_COPY_DEST);
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
		auto ResourceBarrier2 = CD3DX12_RESOURCE_BARRIER::Transition(TextureBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, m_CurrentStates);
		Factory->CommandList->ResourceBarrier(1, &ResourceBarrier2);
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

BearTextureType DX12Texture2D::GetType()
{
	return m_TextureType;
}

void DX12Texture2D::AllocBuffer()
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

void DX12Texture2D::FreeBuffer()
{
	m_Buffer.Reset();
}

void DX12Texture2D::LockUAV(ID3D12GraphicsCommandListX* command_list)
{
	BEAR_CHECK(bAllowUAV);
	auto ResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(TextureBuffer.Get(), m_CurrentStates, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE| D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	command_list->ResourceBarrier(1, &ResourceBarrier);
}

void DX12Texture2D::UnlockUAV(ID3D12GraphicsCommandListX* command_list)
{
	BEAR_CHECK(bAllowUAV);
	auto ResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(TextureBuffer.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, m_CurrentStates);
	command_list->ResourceBarrier(1, &ResourceBarrier);
}
