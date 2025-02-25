#include "RootSignature.hlsl"
#include "Pipeline.hlsli"

cbuffer MatricesConstants : register(b0)
{
	Matrices matrices;
};

[RootSignature(ROOTSIG)]
void main(
// === IN === //
	in float3 i_pos : Position,
	in float2 i_uv : Texcoord,
	in float3 i_normal : Normal,
// === OUT === //
	out float4 o_pos : SV_Position
)
{	
	o_pos = mul(matrices.model, float4(i_pos, 1.0f));

	o_pos = mul(matrices.viewProj, o_pos);
}