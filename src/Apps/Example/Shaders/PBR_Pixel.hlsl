#include "PBR_RootSignature.hlsl"
#include "Pipeline.hlsli"

// PBR Lighting Constants
static const float PI = 3.14159265359;

cbuffer Camera : register(b1)
{
	float3 cameraPosition;
};

cbuffer LightConstants : register(b2)
{
	Light light;
};

cbuffer MaterialData : register(b3)
{
    MaterialData materialData;
}

Texture2D<float4> bindlessTextures[] : register(t0, space0);
sampler textureSampler : register(s0);


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

float3 ComputeLighting(Light light, float3 normalWorldSpace, float3 viewDirection, float3 worldPos, float3 albedo, float roughness, float metallic)
{	
	// Default for Point & Spot lights
    float3 L = normalize(light.position - worldPos);
    float attenuation = 1.0f;
	
	// Handle Directional light (Set attenuation to 1 and use predefined direction)
    float3 lightDirection = lerp(L, -light.direction, light.type == 0);
    attenuation = lerp(attenuation, 1.0f, light.type == 0);
	
	// Handle Point Light Attenuation
    float dist = length(light.position - worldPos);
    float pointAttenuation = saturate(1.0f - (dist * dist) / (light.radius * light.radius));
    attenuation *= lerp(1.0f, pointAttenuation, light.type == 1);
	
	// Handle Spot light (Smooth attenuation)
    float spotFactor = saturate(dot(L, -light.direction));
    float spotAttenuation = smoothstep(light.outerAngle, light.innerAngle, spotFactor);
    attenuation *= lerp(1.0f, spotAttenuation, light.type == 2);
	
	// Halfway vector
    float3 halfwayVec = normalize(viewDirection + lightDirection);
	
	// Compute BRDF components
    float NdotV = max(dot(normalWorldSpace, viewDirection), 0.0);
    float NdotL = max(dot(normalWorldSpace, lightDirection), 0.0);
    float NdotH = max(dot(normalWorldSpace, halfwayVec), 0.0);
    float HdotV = max(dot(halfwayVec, viewDirection), 0.0);
	
	// Compute Fresnel Reflectance at Normal Incidence (F0)	
    float3 F0 = float3(0.04f, 0.04f, 0.04f);	// Default dielectric reflectance
    F0 = lerp(F0, albedo, metallic);			// Metals use albedo as F0
	
	// Compute Cook-Torrance BRDF Components
    float k = (roughness + 1.0f) * (roughness + 1.0f) / 8.0f;
    float NDF = NormalDistributionGGX(normalWorldSpace, halfwayVec, roughness);
    float G = GeometrySmith(normalWorldSpace, viewDirection, halfwayVec, k);
    float3 F = FresnelSchlick(max(dot(halfwayVec, viewDirection), 0.0f), F0);
	
	// Specular BRDF
    float3 numerator = (NDF * F * G);
    float denominator = 4.0f * NdotV * NdotL + 0.0001f;
    float3 specular = numerator / max(denominator, 0.0001f);
	
	// Diffuse Reflection (Lambertian)
    float3 kD = 1.0f - F; // Energy conservation
    kD *= (1.0f - metallic); // Metals have no diffuse component
    float3 Lambertian = albedo / PI;
	
	// Final color computation
    float3 diffuse = kD * Lambertian;
	
    return (diffuse + specular) * light.color * light.intensity * NdotL * attenuation;
}

[RootSignature(PBR_SIG)]
void main(
// === IN === //
	in float2 i_uv : Texcoord,
	in float3 i_normal : Normal,
	in float3 i_tangent : Tangent0,
	in float3 i_bitangent : Tangent1,
	in float4 i_currentPos : PositionT,
	in uint	  i_materialID : TEST,

// === OUT === //
	out float4 pixel : SV_Target
)
{	
	// Texture sampling
	float3 albedoTexel = materialData.baseColor * bindlessTextures[NonUniformResourceIndex(materialData.diffuseID)].Sample(textureSampler, i_uv).rgb;
	float3 normalTexel = bindlessTextures[NonUniformResourceIndex(materialData.normalID)].Sample(textureSampler, i_uv).rgb;
	float3 ormTexel	 = bindlessTextures[NonUniformResourceIndex(materialData.ormID)].Sample(textureSampler, i_uv).rgb;
    float3 emmisive = materialData.emissiveColor * bindlessTextures[NonUniformResourceIndex(materialData.emissiveID)].Sample(textureSampler, i_uv).rgb;
	float roughness = materialData.roughness *  ormTexel.g;
	float metalness = materialData.metalness *  ormTexel.b;
	
	// AlphaClipping
    float alpha = bindlessTextures[NonUniformResourceIndex(materialData.diffuseID)].Sample(textureSampler, i_uv).a;
    clip(alpha - 0.25f); // Discards the pixel if alpha <= 0
	
	//Ambient
    float3 ambient = 0.05f * albedoTexel;
	
	// Convert normal map to world space	
    float3 normal = normalize(i_normal);
	float3 normalWorldSpace = ApplyNormalMap(normal, i_tangent, i_bitangent, normalTexel);
		
	// View Direction
    float3 viewDirection = normalize(cameraPosition - i_currentPos.xyz);
	
    float3 lighting = 0.0f;
    lighting = ComputeLighting(light, normalWorldSpace, viewDirection, i_currentPos.xyz, albedoTexel, roughness, metalness);
	
    float3 finalColor = lighting + emmisive + ambient;
	
	//Gamma correction
    float gamma = 2.2;
    finalColor.rgb = pow(finalColor.rgb, float3(1.0f / gamma, 1.0f / gamma, 1.0f / gamma));
	
    pixel = float4(finalColor, materialData.opacity);
}