#include "PBR_RootSignature.hlsl"
#include "Pipeline.hlsli"

cbuffer MatricesConstants : register(b0)
{
	Matrices matrices;
};

[RootSignature(PBR_SIG)]
void main(
// === IN === //
	in float3 i_pos : Position,
	in float2 i_uv : Texcoord,
	in float3 i_normal : Normal,
	in float3 i_tangent : Tangent0,
	in float3 i_bitangent : Tangent1,

// === OUT === //
	out float2 o_uv : Texcoord,
	out float3 o_normal : Normal,
	out float3 o_tangent : Tangent0,
	out float3 o_bitangent : Tangent1,
	out float4 o_currentPos : PositionT,
	out float4 o_pos : SV_Position
)
{	
	o_uv = i_uv;

	o_normal = (float3)mul(matrices.model, float4(i_normal, 1.0f));

	o_tangent = i_tangent;

	o_bitangent = i_bitangent;

	o_currentPos = mul(matrices.model, float4(i_pos, 1.0f));

	o_pos = mul(matrices.viewProj, o_currentPos);

}