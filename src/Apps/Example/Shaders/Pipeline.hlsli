
struct Matrices
{
    float4x4 viewProj;
    float4x4 model;
};

struct MaterialData
{
    float3 baseColor;
    uint diffuseID;
    
    float3 emissiveColor;
    uint normalID;
    
    uint ormID;
    uint emissiveID;
    
    float opacity;
    float roughness;
    float metalness; 
};

struct Light
{    
    float3 position;
    float type;
    
    float3 direction;
    float intensity;
    
    float3 color;
    float radius;
    
    float innerAngle;
    float outerAngle;
};