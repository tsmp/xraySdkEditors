#include "DX12PCH.h"
bsize RayTracingBottomLevelCounter = 0;

#ifdef RTX
inline void AllocateUAVBuffer(UINT64 buffer_size, ID3D12Resource** pp_resource, D3D12_RESOURCE_STATES initial_resource_state = D3D12_RESOURCE_STATE_COMMON)
{
	auto UploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto BufferDesc = CD3DX12_RESOURCE_DESC::Buffer(buffer_size, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	R_CHK(Factory->Device->CreateCommittedResource(&UploadHeapProperties,D3D12_HEAP_FLAG_NONE,&BufferDesc,initial_resource_state,nullptr,IID_PPV_ARGS(pp_resource)));
}

DX12RayTracingBottomLevel::DX12RayTracingBottomLevel(const BearRayTracingBottomLevelDescription& description)
{
	RayTracingBottomLevelCounter++;

	BearVector<D3D12_RAYTRACING_GEOMETRY_DESC> GeometryDescs;
	GeometryDescs.reserve(description.GeometryDescriptions.size());
	for (const BearRayTracingBottomLevelDescription::GeometryDescription& i : description.GeometryDescriptions)
	{
		D3D12_RAYTRACING_GEOMETRY_DESC GeometryDesc = {};
		switch (i.Type)
		{
		case BearRaytracingGeometryType::Triangles:
			GeometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
			break;
		case BearRaytracingGeometryType::ProceduralPrimitiveAABBS:
			GeometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS;
			break;
		default:
			BEAR_CHECK(false);
		}
		{
			GeometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAGS::D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;
			if (i.Flags.test((uint32)BearRaytracingGeometryFlags::Opaque))
			{
				GeometryDesc.Flags |= D3D12_RAYTRACING_GEOMETRY_FLAGS::D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
			}
			if (i.Flags.test((uint32)BearRaytracingGeometryFlags::NoDuplicateAnyhitInvocation))
			{
				GeometryDesc.Flags |= D3D12_RAYTRACING_GEOMETRY_FLAGS::D3D12_RAYTRACING_GEOMETRY_FLAG_NO_DUPLICATE_ANYHIT_INVOCATION;
			}
		}
		if (GeometryDesc.Type == D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES)
		{

			BEAR_CHECK(!i.Triangles.VertexBuffer.empty());
			GeometryDesc.Triangles.Transform3x4 = 0;
			{
				auto VertexBuffer = static_cast<const DX12VertexBuffer*>(i.Triangles.VertexBuffer.get());
				BEAR_CHECK(VertexBuffer->VertexBufferView.SizeInBytes);
				BEAR_CHECK(VertexBuffer->VertexBufferView.StrideInBytes);
				GeometryDesc.Triangles.VertexBuffer.StartAddress = VertexBuffer->VertexBufferView.BufferLocation + (i.Triangles.VertexOffset * VertexBuffer->VertexBufferView.StrideInBytes);
				GeometryDesc.Triangles.VertexBuffer.StrideInBytes = VertexBuffer->VertexBufferView.StrideInBytes;
				BEAR_CHECK(i.Triangles.VertexCount >= VertexBuffer->VertexBufferView.SizeInBytes / VertexBuffer->VertexBufferView.StrideInBytes);
				GeometryDesc.Triangles.VertexCount = static_cast<UINT>(i.Triangles.VertexCount);
				GeometryDesc.Triangles.VertexFormat = DX12Factory::TranslationForRayTracing(i.Triangles.VertexFormat);
			}
			if (i.Triangles.IndexBuffer.empty())
			{
				GeometryDesc.Triangles.IndexFormat = DXGI_FORMAT_UNKNOWN;
			}
			else
			{
				auto IndexBuffer = static_cast<const DX12IndexBuffer*>(i.Triangles.IndexBuffer.get());
				BEAR_CHECK(IndexBuffer->IndexBufferView.SizeInBytes);
				GeometryDesc.Triangles.IndexBuffer = IndexBuffer->IndexBufferView.BufferLocation + (i.Triangles.IndexOffset * 4);
				GeometryDesc.Triangles.IndexFormat = IndexBuffer->IndexBufferView.Format;
				GeometryDesc.Triangles.IndexCount = static_cast<UINT>(i.Triangles.IndexCount);
			}
		}
		else
		{
			BEAR_CHECK(!i.AABB.Buffer.empty());
			auto Buffer = static_cast<const DX12StructuredBuffer*>(i.AABB.Buffer.get());
			GeometryDesc.AABBs.AABBs.StartAddress = Buffer->StructuredBuffer->GetGPUVirtualAddress() + (i.AABB.Stride * i.AABB.Offset);
			GeometryDesc.AABBs.AABBs.StrideInBytes = i.AABB.Stride;
			GeometryDesc.AABBs.AABBCount = i.AABB.Count;
		}
		GeometryDescs.push_back(GeometryDesc);
	}

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS  AccelerationStructureInputs = {};

	AccelerationStructureInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;

	{

		if (description.BuildFlags.test((uint32)BearAccelerationStructureBuildFlags::AllowCompaction))
			AccelerationStructureInputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_COMPACTION;
		if (description.BuildFlags.test((uint32)BearAccelerationStructureBuildFlags::AllowUpdate))
			AccelerationStructureInputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
		if (description.BuildFlags.test((uint32)BearAccelerationStructureBuildFlags::PreferFastBuild))
			AccelerationStructureInputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
		if (description.BuildFlags.test((uint32)BearAccelerationStructureBuildFlags::PreferFastTrace))
			AccelerationStructureInputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
		if (description.BuildFlags.test((uint32)BearAccelerationStructureBuildFlags::MinimizeMemory))
			AccelerationStructureInputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_MINIMIZE_MEMORY;
	}

	AccelerationStructureInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	AccelerationStructureInputs.pGeometryDescs = GeometryDescs.data();
	AccelerationStructureInputs.NumDescs = static_cast<UINT>(GeometryDescs.size());

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO PrebuildInfo = {};
	Factory->Device->GetRaytracingAccelerationStructurePrebuildInfo(&AccelerationStructureInputs, &PrebuildInfo);

	ComPtr<ID3D12Resource> ScratchResource;
	AllocateUAVBuffer(PrebuildInfo.ScratchDataSizeInBytes, &ScratchResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	AllocateUAVBuffer(PrebuildInfo.ResultDataMaxSizeInBytes, &BottomLevelAccelerationStructure, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);
	Factory->LockCommandList();

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC BottomLevelBuildDesc = {};
	{
		BottomLevelBuildDesc.Inputs = AccelerationStructureInputs;
		BottomLevelBuildDesc.ScratchAccelerationStructureData = ScratchResource->GetGPUVirtualAddress();
		BottomLevelBuildDesc.DestAccelerationStructureData = BottomLevelAccelerationStructure->GetGPUVirtualAddress();
	}
	Factory->CommandList->BuildRaytracingAccelerationStructure(&BottomLevelBuildDesc, 0, nullptr);
	auto ResourceBarrier1 = CD3DX12_RESOURCE_BARRIER::UAV(BottomLevelAccelerationStructure.Get());
	Factory->CommandList->ResourceBarrier(1, &ResourceBarrier1);

	Factory->UnlockCommandList();
}

DX12RayTracingBottomLevel::~DX12RayTracingBottomLevel()
{
	RayTracingBottomLevelCounter--;
}

#endif