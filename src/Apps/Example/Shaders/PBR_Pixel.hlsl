#include "PBR_RootSignature.hlsl"
#include "Pipeline.hlsli"

Texture2D bindlessTextures[] : register(t0, space0);
sampler textureSampler : register(s0);

cbuffer CameraPosition : register(b1)
{
	float3 cameraPosition;
};

cbuffer MaterialData : register(b1)
{
	MaterialData materialData;
};

[RootSignature(PBR_SIG)]
void main(
// === IN === //
	in float2 i_uv : Texcoord,
	in float3 i_normal : Normal,
	in float4 i_currentPos : PositionT,
	in float3 i_tangent : Tangent0,
	in float3 i_bitangent : Tangent1,

// === OUT === //
	out float4 pixel : SV_Target
)
{
	float ambient = 0.1f;
	float3 normal = normalize(i_normal);

	float specularLight = 1.0f;
	float3 viewDirection = normalize(cameraPosition - i_currentPos.xyz);
	
	
    pixel = float4(0.0f, 0.0f, 0.0f, 0.0f);
}