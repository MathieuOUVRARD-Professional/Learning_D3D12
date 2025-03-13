
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
    float padding1;
    
    float3 emissiveColor;
    float padding2;
    
    float opacity;
    float roughness;
    float metalness;
    float padding3;    
};

struct Light
{
    float3 color;
    float padding1;
    float3 position;
    float padding2;
};