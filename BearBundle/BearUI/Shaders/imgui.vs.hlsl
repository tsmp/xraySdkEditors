#include "common.h.hlsl"
BINDING_UNIFORM_0 cbuffer ScreenSizeBuffer : register(b0)
{
	float4        ScreenSize;
}

PS_IMGUI_IN main(VS_IMGUI_IN input)
{
    PS_IMGUI_IN result;
  
    {
    //    input.position.xy += 0.5f;
		result.position.x = input.position.x * ScreenSize.z * 2 - 1;
		result.position.y = (input.position.y * ScreenSize.w * 2 - 1)*-1;
		result.position.zw =float2(0,1);
    }
    result.uv = input.uv;
    result.color = input.color;
    return result;
}