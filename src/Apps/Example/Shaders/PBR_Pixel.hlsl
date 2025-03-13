#include "PBR_RootSignature.hlsl"
#include "Pipeline.hlsli"

// PBR Lighting Constants
static const float PI = 3.14159265359;

Texture2D<float4> bindlessTextures[] : register(t0, space0);
sampler textureSampler : register(s0);

cbuffer Camera : register(b1)
{
	float3 cameraPosition;
};

cbuffer Material : register(b2)
{
	MaterialData materialData;
};

cbuffer LightConstants : register(b3)
{
	Light light;
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
	float3 normal = normalize(i_normal);

	// Texture sampling
	float3 albedoTexel = materialData.baseColor * bindlessTextures[NonUniformResourceIndex(materialData.diffuseID)].Sample(textureSampler, i_uv).rgb;
	float3 normalTexel = bindlessTextures[NonUniformResourceIndex(materialData.normalID)].Sample(textureSampler, i_uv).rgb;
	float3 ormTexel	 = bindlessTextures[NonUniformResourceIndex(materialData.ormID)].Sample(textureSampler, i_uv).rgb;
	float roughness = materialData.roughness *  ormTexel.g;
	float metalness = materialData.metalness *  ormTexel.b;	
	
    float alpha = bindlessTextures[NonUniformResourceIndex(materialData.diffuseID)].Sample(textureSampler, i_uv).a;
    clip(alpha - 0.25f); // Discards the pixel if alpha <= 0

    float ambient =  0.1f;
	
	// Convert normal map to world space
	float3 normalWorldSpace = ApplyNormalMap(normal, i_tangent, i_bitangent, normalTexel);
	
	// View & Light vectors
	float3 viewDirection = normalize(cameraPosition - i_currentPos.xyz);
    float3 lightDirection = normalize(light.position.xyz - i_currentPos.xyz);
	float3 halfwayVec = normalize(viewDirection + lightDirection);
	
	// Compute Fresnel Reflectance at Normal Incidence (F0)	
	float3 F0 = float3(0.04f, 0.04f, 0.04f);	// Default dielectric reflectance
	F0 = lerp(F0, albedoTexel, metalness);		// Metals use albedo as F0

	// Compute Cook-Torrance BRDF Components
	float k = (roughness + 1.0f) * (roughness + 1.0f) / 8.0f;
	float NDF = NormalDistributionGGX(normalWorldSpace, halfwayVec, roughness);
	float G = GeometrySmith(normalWorldSpace, viewDirection, halfwayVec, k);
	float3 F = FresnelSchlick(max(dot(halfwayVec, viewDirection), 0.0f), F0);

	// Specular BRDF
	float3 numerator = (NDF * F * G);
	float denominator = 4.0f * max(dot(normalWorldSpace, viewDirection), 0.0f) * max(dot(normalWorldSpace, lightDirection ), 0.0f) + 0.0001f;
	float3 specular = numerator / max(denominator, 0.0001f);

	 // Diffuse Reflection (Lambertian)
	float3 kD = 1.0f - F;						// Energy conservation
	kD *= (1.0f - metalness);					// Metals have no diffuse component
	float3 Lambertian = albedoTexel / PI;

	// Final color computation
	float NdotL = max(dot(normalWorldSpace, lightDirection), 0.0f);
	float3 diffuse = kD * Lambertian;
	float3 finalColor = (diffuse + specular) * NdotL * light.color.rgb + ambient;
	
    pixel = float4(finalColor, materialData.opacity);
}