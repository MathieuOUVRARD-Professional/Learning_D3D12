
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

struct PBR_V_Out
{
    float2 uv           : Texcoord;
    float3 normal       : Normal;
    float3 tangent      : Tangent0;
    float3 bitangent    : Tangent1;
    float4 currentPos   : PositionT;
    nointerpolation uint materialID : TEXCOORD5;
    float4 pos          : SV_Position;    
};

struct PBR_P_in
{
    nointerpolation int materialID : TEXCOORD5;
    float2 uv           : Texcoord;
    float3 normal       : Normal;
    float3 tangent      : Tangent0;
    float3 bitangent    : Tangent1;
    float4 currentPos   : PositionT;    
};