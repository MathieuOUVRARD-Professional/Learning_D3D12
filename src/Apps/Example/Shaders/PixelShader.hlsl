#include "RootSignature.hlsl"
#include "Pipeline.hlsli"

cbuffer LightConstants : register(b1)
{
	Light light;
};

cbuffer PukeColor : register(b2)
{
	float3 puke;
};

cbuffer CameraPosition : register(b3)
{
	float3 cameraPosition;
};

Texture2D<float4> texture : register(t0);
Texture2D<float4> specTexture : register(t1);
sampler textureSampler : register(s0);

[RootSignature(ROOTSIG)]
void main(
// === IN === //
	in float2 i_uv : Texcoord,
	in float3 i_normal : Normal,
	in float4 i_currentPos : PositionT,

// === OUT === //
	out float4 pixel : SV_Target
)
{
	float ambient = 0.1f;
	float3 normal = normalize(i_normal);
	float3 lightDirection = normalize(light.position.xyz - i_currentPos.xyz);
	float diffuse = max(dot(normal, lightDirection), 0.0f);

	float specularLight = 1.0f;
	float3 viewDirection = normalize(cameraPosition - i_currentPos.xyz);
	float3 reflectionDirection = reflect(-lightDirection, normal);
	float specAmmount = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 50);
	float specular = specAmmount * specularLight;

	float4 texel = texture.Sample(textureSampler, i_uv);
    float4 texelSpec = specTexture.Sample(textureSampler, i_uv);
    pixel = float4(texel.rgb * light.color.xyz * (diffuse + ambient) + texelSpec.r * specular * light.color.xyz , 1.0f);
}