#pragma once

#include <sdkddkver.h>
#if defined( DX12)||defined( DX12_1)
#if (WDK_NTDDI_VERSION) > (NTDDI_WIN10_19H1)
#define DX12UTIMATE
#endif
#endif


///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
#include <d3d12.h>
#include <dxgi1_6.h>
#ifdef DX12UTIMATE
#include "d3dx12Utimate.h"
#else
#include "d3dx12.h"
#endif
#pragma warning(push)
#pragma warning(disable:4324)
#include <windows.h>
#ifdef DX11
#include <D3Dcompiler.h>
#else
#include "dxc/dxcapi.h"
#endif
#include <wrl.h>
#include <shellapi.h>
#pragma warning(pop)
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
#include <atlbase.h>
using Microsoft::WRL::ComPtr;
using ATL::CComPtr;
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
#include "../BearGraphics/BearRenderBase.h"
#define R_CHK(a) BEAR_VERIFY(SUCCEEDED(a))
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
enum EDX12Query
{
	DX12Q_None=0,
	DX12Q_ShaderResource,
	DX12Q_UnorderedAccess,
	DX12Q_Pipeline,
	DX12Q_RayTracingPipeline,
};
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
#if defined(DX11)
using ID3D12GraphicsCommandListX = ID3D12GraphicsCommandList;
using ID3D12DeviceX = ID3D12Device;
using IDXGIFactoryX = IDXGIFactory2;
constexpr D3D_SHADER_MODEL CurrentShadeModel = D3D_SHADER_MODEL::D3D_SHADER_MODEL_5_1;
const D3D_FEATURE_LEVEL CurrentFeatureLevel = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0;
#elif defined(DX12UTIMATE) //DX12 & DX12_1 with Dx12Utimate
using ID3D12GraphicsCommandListX = ID3D12GraphicsCommandList6;
using ID3D12DeviceX = ID3D12Device5;
using IDXGIFactoryX = IDXGIFactory4;
#ifdef DX12
constexpr D3D_SHADER_MODEL CurrentShadeModel = D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_0;
const D3D_FEATURE_LEVEL CurrentFeatureLevel = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_12_0;
#elif DX12_1
constexpr D3D_SHADER_MODEL CurrentShadeModel = D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_3;
const D3D_FEATURE_LEVEL CurrentFeatureLevel = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_12_1;
#endif
#elif defined( DX12)||defined(DX12_1)
using ID3D12GraphicsCommandListX = ID3D12GraphicsCommandList4;
using ID3D12DeviceX = ID3D12Device2;
using IDXGIFactoryX = IDXGIFactory4;
#ifdef DX12
constexpr D3D_SHADER_MODEL CurrentShadeModel = D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_0;
const D3D_FEATURE_LEVEL CurrentFeatureLevel = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_12_0;
#elif DX12_1
constexpr D3D_SHADER_MODEL CurrentShadeModel = D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_3;
const D3D_FEATURE_LEVEL CurrentFeatureLevel = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_12_1;
#endif
#else 
#error "Unsuport current version directx!!!";
#endif

#if defined( DX12)||defined(DX12_1)
#ifdef X64
#define RTX
#endif
#endif
#ifdef DX12UTIMATE
#define MESH_SHADING
#endif

#ifdef RTX
constexpr D3D_SHADER_MODEL CurrentRTXShadeModel = D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_3;
#endif

#ifdef MESH_SHADING
constexpr D3D_SHADER_MODEL CurrentMeshShadingShadeModel = D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_5;
#endif
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
#include "DX12AllocatorHeap.h"
#include "DX12Factory.h"
#include "DX12Viewport.h"
#include "DX12Context.h"
#include "DX12Shader.h"
#include "DX12IndexBuffer.h"
#include "DX12VertexBuffer.h"

#include "DX12UniformBuffer.h"
#include "DX12RootSignature.h"
#include "DX12Pipeline.h"
#include "DX12PipelineGraphics.h"
#include "DX12PipelineMesh.h"
#include "DX12PipelineRayTracing.h"
#include "DX12DescriptorHeap.h"

#include "DX12ShaderResource.h"
#include "DX12UnorderedAccess.h"
#include "DX12Texture2D.h"
#include "DX12Sampler.h"
#include "DX12Stats.h"

#include "DX12RenderPass.h"
#include "DX12FrameBuffer.h"

#include "DX12TextureCube.h"
#include "DX12StructuredBuffer.h"

#include "DX12RayTracingBottomLevel.h"
#include "DX12RayTracingTopLevel.h"
#include "DX12RayTracingShaderTable.h"