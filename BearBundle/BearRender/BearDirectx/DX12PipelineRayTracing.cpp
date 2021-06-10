#include "DX12PCH.h"
bsize PipelineRayTracingCounter = 0;
#ifdef RTX
DX12PipelineRayTracing::DX12PipelineRayTracing(const BearPipelineRayTracingDescription & description)
{
	PipelineRayTracingCounter++;
	RootSignature = description.GlobalRootSignature;
	BEAR_CHECK(RootSignature.empty() == false);
	RootSignaturePointer = static_cast<DX12RootSignature*>(RootSignature.get());
	/////////////////////////////////////////////////////////////////////////////////////////////
	BearVector<BearStringConteniarUnicode> LocalRootSignature_Exports;
	/////////////////////////////////////////////////////////////////////////////////////////////
	CD3DX12_STATE_OBJECT_DESC RayTracingPipeline{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };
	for (const BearPipelineRayTracingDescription::ShaderDescription& i : description.Shaders)
	{
		auto Library = RayTracingPipeline.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
		for (const BearPipelineRayTracingDescription::ShaderDescription::ExportDescription& a : i.Exports)
		{
			if (a.NameFunction.size())
			{
				Library->DefineExport(*a.NameExport, *a.NameFunction);
			}
			else
			{
				Library->DefineExport(*a.NameExport);
			}
			LocalRootSignature_Exports.push_back(*a.NameExport);
		}
		auto ShaderLibrary = const_cast<DX12Shader*>(static_cast<const DX12Shader*>(i.Shader.get()));
		BEAR_CHECK(ShaderLibrary && ShaderLibrary->IsType(BearShaderType::RayTracing));
		D3D12_SHADER_BYTECODE DXI = CD3DX12_SHADER_BYTECODE((void*)ShaderLibrary->GetPointer(), ShaderLibrary->GetSize());
		Library->SetDXILLibrary(&DXI);
	}
	for (const BearPipelineRayTracingDescription::HitGroupDescription& i : description.HitGroups)
	{
		auto HitGroup = RayTracingPipeline.CreateSubobject< CD3DX12_HIT_GROUP_SUBOBJECT>();
		HitGroup->SetHitGroupExport(*i.NameExport);

		switch (i.Type)
		{
		case BearHitGroupType::Triangles:
			HitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);
			break;
		case BearHitGroupType::ProceduralPrimitive:
			HitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE);
			break;
		default:
			BEAR_CHECK(false);
			HitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);
			break;
		}

		if (i.AnyHitShaderImport.size())
			HitGroup->SetAnyHitShaderImport(*i.AnyHitShaderImport);
		if (i.ClosestHitShaderImport.size())
			HitGroup->SetClosestHitShaderImport(*i.ClosestHitShaderImport);
		if (i.IntersectionShaderImport.size())
			HitGroup->SetIntersectionShaderImport(*i.IntersectionShaderImport);
	}
	{
		auto ShaderConfig = RayTracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
		ShaderConfig->Config(description.ShaderConfig.MaxPayloadSizeInBytes,description.ShaderConfig.MaxAttributeSizeInBytes);
	}
	{
		auto LocalRootSignatureSub = RayTracingPipeline.CreateSubobject< CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
		LocalRootSignatureSub->SetRootSignature(Factory->LocalRootSignatureDefault.Get());

		auto RootSignatureAssociation = RayTracingPipeline.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
		for (const BearStringConteniarUnicode& a : LocalRootSignature_Exports)
		{
			RootSignatureAssociation->AddExport(*a);
		}
		RootSignatureAssociation->SetSubobjectToAssociate(*LocalRootSignatureSub);
	}
	{
		auto GlobalRootSignature = RayTracingPipeline.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
		GlobalRootSignature->SetRootSignature(RootSignaturePointer->RootSignature.Get());
	}
	{
		auto Config = RayTracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
		Config->Config(description.PipelineConfig.MaxTraceRecursionDepth);
	}
	R_CHK(Factory->Device->CreateStateObject(RayTracingPipeline, IID_PPV_ARGS(&PipelineState)));
}

DX12PipelineRayTracing::~DX12PipelineRayTracing()
{
	PipelineRayTracingCounter--;
}

void* DX12PipelineRayTracing::QueryInterface(int åype)
{
	switch (åype)
	{
	case DX12Q_Pipeline:
		return reinterpret_cast<void*>(static_cast<DX12Pipeline*>(this));
	case DX12Q_RayTracingPipeline:
		return reinterpret_cast<void*>(this);
	default:
		return nullptr;
	}
}

BearPipelineType DX12PipelineRayTracing::GetType()
{
	return BearPipelineType::RayTracing;
}

void DX12PipelineRayTracing::Set(ID3D12GraphicsCommandListX*command_list)
{
	command_list->SetComputeRootSignature(RootSignaturePointer->RootSignature.Get());
	command_list->SetPipelineState1(PipelineState.Get());
}
#endif
