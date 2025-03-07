
struct Matrices
{
    float4x4 viewProj;
    float4x4 model;
};

struct MaterialData
{
    uint materialID;
    uint textureCount;
    
    float3 baseColor;
    float3 emmisiveColor;
    
    float opacity;
    float roughness;
    float metalness;
};

struct Light
{
    float4 color;
    float4 position;
};