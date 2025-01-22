#include "RootSignature.hlsl"

[RootSignature(ROOTSIG)]
float4 main(float4 pos : SV_Position) : SV_Target
{
	return float4(pos.x, pos.y, pos.x + pos.y, 1.0f);
}