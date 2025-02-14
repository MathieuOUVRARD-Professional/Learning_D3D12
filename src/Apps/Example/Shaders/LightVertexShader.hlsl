#include "LightRootSignature.hlsl"
#include "Pipeline.hlsli"

CameraMatrices cameraMatrices : register(b1);

[RootSignature(LIGHTSIG)]
void main(
// === IN === //
	in float3 pos : Position,
// === OUT === //
	out float4 o_pos : SV_Position
)
{
	o_pos = mul(mul(cameraMatrices.projection, cameraMatrices.view), float4(pos, 1.0f));
}