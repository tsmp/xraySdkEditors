#include "VKPCH.h"
size_t PipelineMeshCounter = 0;

VKPipelineMesh::VKPipelineMesh(const BearPipelineMeshDescription& description)
{
	PipelineMeshCounter++;
	Pipeline = 0;
}

VKPipelineMesh::~VKPipelineMesh()
{
	PipelineMeshCounter--;
}

void VKPipelineMesh::Set(VkCommandBuffer command_list)
{
}

void* VKPipelineMesh::QueryInterface(int type)
{
	switch (type)
	{
	case VKQ_Pipeline:
		return reinterpret_cast<void*>(static_cast<VKPipeline*>(this));
	default:
		return nullptr;
	}
}

BearPipelineType VKPipelineMesh::GetType()
{
	return BearPipelineType::Mesh;
}
