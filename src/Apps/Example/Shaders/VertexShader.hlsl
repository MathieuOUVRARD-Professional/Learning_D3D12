#include "RootSignature.hlsl"
#include "Pipeline.hlsli"

Matrices matrices : register(b1);

[RootSignature(ROOTSIG)]
void main(
// === IN === //
	in float3 pos : Position,
	in float2 uv : Texcoord,
// === OUT === //
	out float2 o_uv : Texcoord,
	out float4 o_pos : SV_Position
)
{
	o_pos = mul(mul(matrices.viewProj, matrices.model), float4(pos, 1.0f));
	o_uv = uv;
}