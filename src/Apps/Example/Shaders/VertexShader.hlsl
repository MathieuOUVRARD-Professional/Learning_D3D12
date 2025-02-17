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
	out float2 o_uv : Texcoord,
	out float3 o_normal : Normal,
	out float4 o_currentPos : PositionT,
	out float4 o_pos : SV_Position
)
{	
	o_currentPos = mul(matrices.model, float4(i_pos, 1.0f));

	o_pos = mul(matrices.viewProj, o_currentPos);

	o_uv = i_uv;

	o_normal = mul(matrices.model, float4(i_normal, 1.0f));
}