#include "PBR_RootSignature.hlsl"
#include "Pipeline.hlsli"

// PBR Lighting Constants
static const float PI = 3.14159265359;


cbuffer CameraData : register(b0)
{
	CameraData cameraData;
};

cbuffer SceneData : register(b2)
{
    SceneData sceneData;
}

cbuffer LightData : register(b3)
{
    Light lights[100];
};

cbuffer MaterialData : register(b5)
{
    MaterialData materialsData[29];
}

Texture2D<float4> bindlessTextures[] : register(t0, space0);
sampler textureSampler : register(s0);
sampler shadowSampler : register(s1);


float3 ApplyNormalMap(float3 normal, float3 tangent, float3 bitangent, float3 sampledNormal)
{
	//Remap range from [0; 1] to [-1; 1]
	sampledNormal = sampledNormal * 2 - 1.0f;
	
	//Create TBN (Tangent Bitangent Normal) matrix
	float3x3 TBN = float3x3(tangent, bitangent, normal);
	
	//Adapt sampled normal from tangent space to world space 
	return normalize(mul(sampledNormal, TBN));
}

float ComputeShadow(float4 shadowPos, Light light, float3 normalWorldSpace)
{
	//Convert from homogeneous clip space to NDC
    float3 projCoords = shadowPos.xyz / shadowPos.w;
    projCoords.xy = projCoords.xy * float2(0.5f, -0.5f) + 0.5f;
	
	//Compare depth with shadow map
    float currentDepth = projCoords.z;
	//currentDepth = lerp(currentDepth, LinearizeDepth(currentDepth, 0.01f, light.radius) / light.radius, light.type == 1 || light.type == 2);
	
    //float spotBias = max(0.0001f * (1.0f - max(dot(normalWorldSpace, light.direction), 0.0)), 0.000005f);
    float spotBias =0.000005f;
    float bias = lerp(0.005f, spotBias, light.type == 1 || light.type == 2); // max(0.05 * (1.0 - dot(normalWorldSpace, light.direction)), 0.005);
	
    float shadow = 0.0;
	
    float2 shadowMapSize = float2(0.0f, 0.0f);
    bindlessTextures[NonUniformResourceIndex(light.shadowMapID)].GetDimensions(shadowMapSize.x, shadowMapSize.y);    
	
    float2 texelSize = 1.0 / shadowMapSize;
    int samlingRadius = 3;
	
    for (int x = -samlingRadius; x <= samlingRadius; ++x)
    {
        for (int y = -samlingRadius; y <= samlingRadius; ++y)
        {
            float pcfDepth = bindlessTextures[NonUniformResourceIndex(light.shadowMapID)].Sample(textureSampler, projCoords.xy + float2(x, y) * texelSize).r;
            //pcfDepth = lerp(pcfDepth, LinearizeDepth(pcfDepth, 0.01f, light.radius) / light.radius, light.type == 1 || light.type == 2);
			
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= pow((1 + 2 * samlingRadius), 2);    
	
	// If out of range set no shadow for directional lights
    shadow = lerp(shadow, 0.0, currentDepth > 1.0 && light.type == 0);
    
    return shadow;
}

float NormalDistributionGGX(float3 normalWorldSpace, float3 halfway, float roughness)
{
    float alpha_Squared = roughness * roughness * roughness * roughness;
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
    float pointAttenuation = saturate(1.0f - ((dist * dist) / (light.radius * light.radius)));
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
    float k = ((roughness + 1.0f) * (roughness + 1.0f)) / 8.0f;
    float NDF = NormalDistributionGGX(normalWorldSpace, halfwayVec, roughness);
    float G = GeometrySmith(normalWorldSpace, viewDirection, halfwayVec, k);
    float3 F = FresnelSchlick(max(dot(halfwayVec, viewDirection), 0.0f), F0);
	
	// Specular BRDF
    float3 numerator = (NDF * F * G);
    float denominator = 4.0f * NdotV * NdotL + 0.0001f;
    //float3 specular = numerator / max(denominator, 0.0001f);
    float3 specular = numerator / denominator;
	
	// Diffuse Reflection (Lambertian)
    float3 kD = float3(1.0f, 1.0f, 1.0f) - F;	// Energy conservation
    kD *= (1.0f - metallic);					// Metals have no diffuse component
    float3 Lambertian = albedo / PI;
	
	// Final color computation
    float3 diffuse = kD * Lambertian;
	
    return (diffuse + specular) * light.color * light.intensity * NdotL * attenuation;
}

[RootSignature(PBR_SIG)]
void main(
// === IN === //
	PBR_V_Out pInput,

// === OUT === //
	out float4 pixel : SV_Target
)
{	
    MaterialData materialData = materialsData[pInput.materialID];
	
	// Texture sampling
	float3 albedoTexel = materialData.baseColor * bindlessTextures[NonUniformResourceIndex(materialData.diffuseID)].Sample(textureSampler, pInput.uv).rgb;
	float3 normalTexel = bindlessTextures[NonUniformResourceIndex(materialData.normalID)].Sample(textureSampler, pInput.uv).rgb;
	float3 ormTexel	 = bindlessTextures[NonUniformResourceIndex(materialData.ormID)].Sample(textureSampler, pInput.uv).rgb;
    float3 emmisive = materialData.emissiveColor * bindlessTextures[NonUniformResourceIndex(materialData.emissiveID)].Sample(textureSampler, pInput.uv).rgb;
	float roughness = materialData.roughness *  ormTexel.g;
	float metalness = materialData.metalness *  ormTexel.b;
	
    float gamma = 2.2;
	// Convert albedo texture from sRGB to linearSpace
    albedoTexel.rgb = pow(albedoTexel.rgb, gamma);
	
	// AlphaClipping
    float alpha = bindlessTextures[NonUniformResourceIndex(materialData.diffuseID)].Sample(textureSampler, pInput.uv).a;
    clip(alpha - 0.25f); // Discards the pixel if alpha <= 0
	
	//Ambient
    float3 ambient = 0.01f * albedoTexel;
	
	// Convert normal map to world space	
    float3 normal = normalize(pInput.normal);
	float3 normalWorldSpace = ApplyNormalMap(normal, pInput.tangent, pInput.bitangent, normalTexel);
		
	// View Direction
    float3 viewDirection = normalize(cameraData.position - pInput.worldPos.xyz);
	
    float3 lighting = 0.0f;
    float shadowFactor = 0.0f;
    float3 finalColor = 0.0f;
    
    for (int i = 0; i < sceneData.N_LIGHTS; i++)
    {
        float4 shadowPos = mul(lights[i].viewProjMatrix, float4(pInput.worldPos.xyz, 1.0f));
        shadowFactor = ComputeShadow(shadowPos, lights[i], normalWorldSpace);
		
        lighting = ComputeLighting(lights[i], normalWorldSpace, viewDirection, pInput.worldPos.xyz, albedoTexel, roughness, metalness);
        finalColor += (1.0 - shadowFactor) * lighting;
    }
	
    finalColor += emmisive + ambient;
	
	//Gamma correction (back from  linear space to sRGB space)
    finalColor.rgb = pow(finalColor.rgb / (finalColor.rgb + float3(1.0f, 1.0f, 1.0f)), float3(1.0f / gamma, 1.0f / gamma, 1.0f / gamma));
	
    pixel = float4(finalColor, materialData.opacity);
}