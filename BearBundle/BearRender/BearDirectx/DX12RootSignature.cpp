#include "DX12PCH.h"
bsize RootSignatureCounter = 0;
inline D3D12_SHADER_VISIBILITY TransletionShaderVisible(BearShaderType type, D3D12_ROOT_SIGNATURE_FLAGS& flags)
{
	switch (type)
	{
#ifdef RTX
	case BearShaderType::RayTracing:
		BEAR_CHECK(Factory->bSupportRayTracing);
		return D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;
#endif
#ifdef MESH_SHADING
	case BearShaderType::Mesh:
		BEAR_CHECK(Factory->bSupportMeshShader);
		flags = flags & ~D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS;
		return D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_MESH;
	case BearShaderType::Amplification:
		BEAR_CHECK(Factory->bSupportMeshShader);
		flags = flags & ~D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS;
		return D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_AMPLIFICATION;
#endif
	case BearShaderType::Vertex:
		flags = flags & ~D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
		return D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_VERTEX;
	case BearShaderType::Hull:
		flags = flags & ~D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
		return D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_HULL;
	case BearShaderType::Domain:
		flags = flags & ~D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
		return D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_DOMAIN;
	case BearShaderType::Geometry:
		flags = flags & ~D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
		return D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_GEOMETRY;
	case BearShaderType::Pixel:
		flags = flags & ~D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
		return D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_PIXEL;
	case BearShaderType::Compute:
		return D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;
	case BearShaderType::ALL:
		flags = flags & ~D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
		flags = flags & ~D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
		flags = flags & ~D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
		flags = flags & ~D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
		flags = flags & ~D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
#ifdef MESH_SHADING
		if (Factory->bSupportMeshShader)
		{
			flags = flags & ~D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS;
			flags = flags & ~D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS;
		}
#endif
		return D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;
	default:
		BEAR_CHECK(0);
	}
	return D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;
}
DX12RootSignature::DX12RootSignature(const BearRootSignatureDescription& description)
{
	RootSignatureCounter++;
	CountBuffers = 0;
	CountSamplers = 0;
	CountSRVs = 0;
	CountUAVs = 0;
	{
		for (bsize i = 0; i < 16; i++)
		{
			SlotSamplers[i] = 16;
			SlotBuffers[i] = 16;
			SlotSRVs[i] = 16;
			if (description.UniformBuffers[i].Shader != BearShaderType::Null)CountBuffers++;
			if (description.SRVResources[i].Shader != BearShaderType::Null) CountSRVs++;
			if (description.Samplers[i].Shader != BearShaderType::Null) CountSamplers++;
			if (description.UAVResources[i].Shader != BearShaderType::Null) CountUAVs++;
		}
	}

	{
		D3D12_FEATURE_DATA_ROOT_SIGNATURE FeatureData = {};

		FeatureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

		if (FAILED(Factory->Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &FeatureData, sizeof(FeatureData))))
		{
			FeatureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}
		D3D12_ROOT_SIGNATURE_FLAGS RootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
#ifdef MESH_SHADING
			D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS|
			D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS|
#endif
			D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

		bsize Count = CountBuffers + CountSRVs + CountSamplers + CountUAVs;
		CD3DX12_DESCRIPTOR_RANGE1 Ranges[64];
		CD3DX12_ROOT_PARAMETER1 RootParameters[128];
		bsize offset = 0;
		for (bsize i = 0; i < 16; i++)
		{
			if (description.UniformBuffers[i].Shader != BearShaderType::Null)
			{
				SlotBuffers[i] = offset;
				Ranges[offset].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, static_cast<UINT>(i));
				RootParameters[offset].InitAsDescriptorTable(1, &Ranges[offset], TransletionShaderVisible(description.UniformBuffers[i].Shader, RootSignatureFlags));
				offset++;
			}
			
		}
		for (bsize i = 0; i < 16; i++)
		{
			if (description.SRVResources[i].Shader != BearShaderType::Null)
			{
				SlotSRVs[i] = offset- CountBuffers;
				switch (description.SRVResources[i].DescriptorType)
				{
				case BearSRVDescriptorType::AccelerationStructure:
				case BearSRVDescriptorType::Buffer:
					RootParameters[offset].InitAsShaderResourceView(static_cast<UINT>(i),0,D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, TransletionShaderVisible(description.SRVResources[i].Shader, RootSignatureFlags));
					break;
				case BearSRVDescriptorType::Image:
					Ranges[offset].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, static_cast<UINT>(i));
					RootParameters[offset].InitAsDescriptorTable(1, &Ranges[offset], TransletionShaderVisible(description.SRVResources[i].Shader, RootSignatureFlags));
					break;
				default:
					break;
				}
				offset++;
			}
		}
		for (bsize i = 0; i < 16; i++)
		{
			if (description.UAVResources[i].Shader != BearShaderType::Null)
			{
				SlotUAVs[i] = offset - (CountBuffers + CountSRVs);
				switch (description.UAVResources[i].DescriptorType)
				{
				case BearUAVDescriptorType::Buffer:
					RootParameters[offset].InitAsUnorderedAccessView(static_cast<UINT>(i), 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, TransletionShaderVisible(description.UAVResources[i].Shader, RootSignatureFlags));
					break;
				case BearUAVDescriptorType::Image:
					Ranges[offset].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, static_cast<UINT>(i));
					RootParameters[offset].InitAsDescriptorTable(1, &Ranges[offset], TransletionShaderVisible(description.UAVResources[i].Shader, RootSignatureFlags));
					break;
				default:
					break;
				}
				offset++;
			}
		}

		for (bsize i = 0; i < 16; i++)
		{
			if (description.Samplers[i].Shader != BearShaderType::Null)
			{
				SlotSamplers[i] = offset - (CountBuffers + CountSRVs+CountUAVs);
				Ranges[offset].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, static_cast<UINT>(i));
				RootParameters[offset].InitAsDescriptorTable(1, &Ranges[offset], TransletionShaderVisible(description.Samplers[i].Shader, RootSignatureFlags));
				offset++;
			}
		}

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC RootSignatureDesc;
		RootSignatureDesc.Init_1_1(static_cast<UINT>(Count), RootParameters, 0, 0, RootSignatureFlags);

		ComPtr<ID3DBlob> Signature;
		ComPtr<ID3DBlob> Error;
		R_CHK(D3DX12SerializeVersionedRootSignature(&RootSignatureDesc, FeatureData.HighestVersion, &Signature, &Error));
		R_CHK(Factory->Device->CreateRootSignature(0, Signature->GetBufferPointer(), Signature->GetBufferSize(), IID_PPV_ARGS(&RootSignature)));
	}

}

DX12RootSignature::~DX12RootSignature()
{
	RootSignatureCounter--;
}
void DX12RootSignature::Set(ID3D12GraphicsCommandListX* command_list)
{
	command_list->SetGraphicsRootSignature(RootSignature.Get());
}