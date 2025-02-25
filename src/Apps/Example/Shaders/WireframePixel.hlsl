#include "RootSignature.hlsl"

cbuffer WireframeColor : register(b1)
{
	float4 color;
};

[RootSignature(ROOTSIG)]
void main(
// === OUT === //
	out float4 pixel : SV_Target
)
{	
	pixel = color;
}