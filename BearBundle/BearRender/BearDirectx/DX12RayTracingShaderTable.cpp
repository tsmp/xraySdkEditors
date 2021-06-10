#include "DX12PCH.h"
bsize RayTracingShaderTableCounter = 0;

inline bsize GetAlignment(bsize size, const bsize alignment)
{
	return (size + (alignment - 1)) & (~(alignment - 1));
}
#ifdef RTX
DX12RayTracingShaderTable::DX12RayTracingShaderTable(const BearRayTracingShaderTableDescription& description)
{
	RayTracingShaderTableCounter++;
	DX12PipelineRayTracing* Pipeline = reinterpret_cast<DX12PipelineRayTracing*>(const_cast<BearRHI::BearRHIPipelineRayTracing*>(description.Pipeline.get())->QueryInterface(DX12Q_RayTracingPipeline));
	BEAR_CHECK(Pipeline);

	ComPtr<ID3D12StateObjectProperties> StateObjectProperties;
	R_CHK(Pipeline->PipelineState.As(&StateObjectProperties));

	Size = 0;
	if (description.CallableShader.size())
	{
		Size += D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
	}
	if (description.RayGenerateShader.size())
	{
		Size += D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
	}
	if (description.MissShader.size())
	{
		Size += D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
	}
	{
		Size += D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT* description.HitGroups.size();
		Size = GetAlignment(Size, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
	}


	{
		bear_fill(RayGenerationShaderRecord);
		bear_fill(MissShaderTable);
		bear_fill(HitGroupTable);
		bear_fill(CallableShaderTable);
	}


	auto Properties  = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD );
	auto ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(Size);
	R_CHK(Factory->Device->CreateCommittedResource(&Properties,D3D12_HEAP_FLAG_NONE,&ResourceDesc,D3D12_RESOURCE_STATE_GENERIC_READ,nullptr,IID_PPV_ARGS(&Buffer)));
	{
		uint8* PtrStart = nullptr;
		uint8* Ptr = nullptr;
		R_CHK(Buffer->Map(0, nullptr,reinterpret_cast<void**>(&Ptr)));
		PtrStart = Ptr;
	

		if (description.RayGenerateShader.size())
		{
			bear_copy(Ptr, StateObjectProperties->GetShaderIdentifier(*description.RayGenerateShader), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
			RayGenerationShaderRecord.StartAddress = (Ptr - PtrStart)+Buffer->GetGPUVirtualAddress();
			RayGenerationShaderRecord.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
			Ptr += GetAlignment(RayGenerationShaderRecord.SizeInBytes, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
		}

		if (description.MissShader.size())
		{
			bear_copy(Ptr, StateObjectProperties->GetShaderIdentifier(*description.MissShader), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
			MissShaderTable.StartAddress  = (Ptr - PtrStart) + Buffer->GetGPUVirtualAddress();
			MissShaderTable.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
			MissShaderTable.StrideInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
			Ptr += GetAlignment(MissShaderTable.SizeInBytes, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
		}
		if (description.HitGroups.size())
		{
			HitGroupTable.StartAddress = (Ptr - PtrStart) + Buffer->GetGPUVirtualAddress();
			HitGroupTable.StrideInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
			for (bsize i = 0; i < description.HitGroups.size(); i++)
			{
				bear_copy(Ptr, StateObjectProperties->GetShaderIdentifier(*description.HitGroups[i]), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
				Ptr += GetAlignment(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
			}
			HitGroupTable.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES * description.HitGroups.size();
			Ptr += GetAlignment(HitGroupTable.SizeInBytes, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
		}
		if (description.CallableShader.size())
		{
			bear_copy(Ptr, StateObjectProperties->GetShaderIdentifier(*description.CallableShader), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
			CallableShaderTable.StartAddress = (Ptr - PtrStart) + Buffer->GetGPUVirtualAddress();
			CallableShaderTable.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
			CallableShaderTable.StrideInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
			Ptr += GetAlignment(HitGroupTable.SizeInBytes, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
		}
		Buffer->Unmap(0, nullptr);
	}

}

DX12RayTracingShaderTable::~DX12RayTracingShaderTable()
{
	RayTracingShaderTableCounter--;
}
#endif