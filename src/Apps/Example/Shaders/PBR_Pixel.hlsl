#include "PBR_RootSignature.hlsl"
#include "Pipeline.hlsli"

// PBR Lighting Constants
static const float PI = 3.14159265359;

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

float3 ApplyNormalMap(float3 normal, float3 tangent, float3 bitangent, float3 sampledNormal)
{
	//Remap range from [0; 1] to [-1; 1]
	sampledNormal = sampledNormal * 2 - 1.0f;
	
	//Create TBN (Tangent Bitangent Normal) matrix
	float3x3 TBN = float3x3(tangent, bitangent, normal);
	
	//Adapt sampled normal from tangent space to world space 
	return normalize(mul(sampledNormal, TBN));
}

float NormalDistributionGGX(float3 normalWorldSpace, float3 halfway, float roughness)
{
	float alpha_Squared = roughness * roughness;
	float NdotH_Squared = max(dot(normalWorldSpace, halfway),0) * max(dot(normalWorldSpace, halfway), 0);

	float denominator = (NdotH_Squared * (alpha_Squared - 1.0) + 1.0);
	denominator = PI * denominator * denominator;


	return alpha_Squared / denominator;
}

float GeometrySchlickGGX(float NdotV, float k)
{	
	return NdotV / (NdotV * (1.0 - k) + k); 
}

float GeometrySmith(float3 normalWorldSpace, float3 view, float3 lightDir, float k)
{
	float NdotV = max(dot(normalWorldSpace, view), 0);
	float NdotL = max(dot(normalWorldSpace, lightDir), 0);

	float ggx1 = GeometrySchlickGGX(NdotV, k);
	float ggx2 = GeometrySchlickGGX(NdotL, k);

	return ggx1 * ggx2;
}

float3 FresnelSchlick(float cosTheta, float3 F0)
{
	return F0 + (1.0f - F0) * pow(1.0f - cosTheta, 5.0f);
}

[RootSignature(PBR_SIG)]
void main(
// === IN === //
	in float2 i_uv : Texcoord,
	in float3 i_normal : Normal,
	in float3 i_tangent : Tangent0,
	in float3 i_bitangent : Tangent1,
	in float4 i_currentPos : PositionT,

// === OUT === //
	out float4 pixel : SV_Target
)
{
	//Texture sampling
	float3 albedoTexel = (float3)bindlessTextures[NonUniformResourceIndex(materialData.diffuseID)].Sample(textureSampler, i_uv);
	float3 normalTexel = (float3)bindlessTextures[NonUniformResourceIndex(materialData.normalID)].Sample(textureSampler, i_uv);
	float3 ormTexel	 = (float3)bindlessTextures[NonUniformResourceIndex(materialData.ormID)].Sample(textureSampler, i_uv);

	float3 normalWorldSpace = ApplyNormalMap(i_normal, i_tangent, i_bitangent, normalTexel);

	float ambient = 0.1f;
	float3 normal = normalize(i_normal);

	float specularLight = 1.0f;
	float3 viewDirection = normalize(cameraPosition - i_currentPos.xyz);
	
	// Default dielectric reflectance
	float3 F0 = float3(0.04f, 0.04f, 0.04f);	
	// Metals use albedo as F0
	F0 = lerp(F0, albedoTexel, ormTexel.b);

	float NDF = NormalDistributionGGX(normalWorldSpace, HALFWAYVEC, ormTexel.g);
	float G = GeometrySmith(normalWorldSpace, viewDirection, ormTexel.g);
	float3 F = FresnelSchlick(max(dot(HALFWAYVEC, viewDirection), 0.0f), F0);

	// BRDF
	float3 sLambert = albedoTexel / PI;
	float3 sCookTorrance = 
		((NDF * F * G) / 
		(4.0f * max(dot(normal, viewDirection), 0.0f) * max(dot(normal, nLIGHTDIR ), 0))) 
		+ 0.0001f;
	
    pixel = float4(0.0f, 0.0f, 0.0f, 0.0f);
}