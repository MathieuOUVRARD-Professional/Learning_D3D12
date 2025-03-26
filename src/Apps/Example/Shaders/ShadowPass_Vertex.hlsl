#include "ShadowPass_RootSignature.hlsl"
#include "Pipeline.hlsli"

cbuffer MatricesConstants : register(b0)
{
	Matrices matrices;
};

[RootSignature(SHADOWPASS_SIG)]
void main(
// === IN === //
	in float3 i_pos : Position,

// === OUT === //
	out float4 o_pos : SV_Position
)
{	
    o_pos = mul(matrices.viewProj, mul(matrices.model, float4(i_pos, 1.0f)));
    
    //o_pos.z = ((LinearizeDepth(o_pos.z / o_pos.w, 0.01f, 20.0f) * o_pos.w) / 20.0f);
}