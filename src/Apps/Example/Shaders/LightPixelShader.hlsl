#include "LightRootSignature.hlsl"

cbuffer LightColor : register(b1)
{
	float3 lightColor;
};

[RootSignature(LIGHTSIG)]
void main(
// === OUT === //
	out float4 pixel : SV_Target

)
{	
	pixel = float4(lightColor, 1.0f);
}