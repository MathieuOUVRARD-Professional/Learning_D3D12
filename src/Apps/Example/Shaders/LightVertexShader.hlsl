#include "LightRootSignature.hlsl"
#include "Pipeline.hlsli"

cbuffer MatricesConstants : register(b0)
{
	Matrices matrices;
};

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