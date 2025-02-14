#include "LightRootSignature.hlsl"

float3 lightColor : register(b0);

[RootSignature(LIGHTSIG)]
void main(
// === OUT === //
	out float4 pixel : SV_Target

)
{	
	pixel = float4(lightColor, 1.0f);
}