
struct Matrices
{
    float4x4 viewProj;
    float4x4 model;
};

struct CameraData
{
    float4x4 viewProj;
    float3 position;
};

struct ModelData
{
    float4x4 model;
    uint materialID;
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
    float4x4 viewProjMatrix;

    float3 position;
    float type;
    
    float3 direction;
    float intensity;
    
    float3 color;
    float radius;
    
    float innerAngle;
    float outerAngle;
    float shadowMapID;    
};

struct PBR_V_Out
{
    float2 uv           : Texcoord;
    float3 normal       : Normal;
    float3 tangent      : Tangent0;
    float3 bitangent    : Tangent1;
    float4 currentPos   : PositionT;
    uint   materialID   : MaterialID;
    //nointerpolation uint materialID : TEXCOORD5;
    float4 pos          : SV_Position;    
};

struct PBR_P_in
{
    //nointerpolation int materialID : TEXCOORD5;
    float2 uv           : Texcoord;
    float3 normal       : Normal;
    float3 tangent      : Tangent0;
    float3 bitangent    : Tangent1;
    float4 currentPos   : PositionT;    
    uint   materialID   : MaterialID;
};

float LinearizeDepth(float depth, float nearPlane, float farPlane)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));
}