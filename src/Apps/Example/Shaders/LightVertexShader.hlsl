#include "LightRootSignature.hlsl"
#include "Pipeline.hlsli"

#pragma pack_matrix(column_major)

Matrices matrices : register(b1);

[RootSignature(LIGHTSIG)]
void main(
// === IN === //
	in float3 pos : Position,
// === OUT === //
	out float4 o_pos : SV_Position
)
{
	o_pos = mul(mul(matrices.viewProj, matrices.model), float4(pos, 1.0f));
}