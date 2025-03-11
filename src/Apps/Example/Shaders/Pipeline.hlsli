
struct Matrices
{
    float4x4 viewProj;
    float4x4 model;
};

struct MaterialData
{
    uint diffuseID;
    uint normalID;
    uint ormID;
    uint emissiveID;
    
    float3 baseColor;
    float3 emissiveColor;
    
    float opacity;
    float roughness;
    float metalness;
};

struct Light
{
    float4 color;
    float4 position;
};