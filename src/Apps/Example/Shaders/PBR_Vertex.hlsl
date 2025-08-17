#include "PBR_RootSignature.hlsl"
#include "Pipeline.hlsli"

cbuffer CameraData : register(b0)
{
	CameraData cameraData;
};

cbuffer ModelID : register(b1)
{
	uint modelID;
};

cbuffer ModelData : register(b4)
{
    ModelData modelsData[405];
}

struct PBR_V_In
{
	float3 pos : Position;
	float2 uv : Texcoord;
	float3 normal : Normal;
	float3 tangent : Tangent0;
	float3 bitangent : Tangent1;
};

[RootSignature(PBR_SIG)]
PBR_V_Out main(PBR_V_In vInput)
{	
	ModelData modelData = modelsData[modelID];

	PBR_V_Out vOutput;

	vOutput.uv = vInput.uv;

    vOutput.normal = (float3) mul((float3x3) modelData.model, vInput.normal);

    vOutput.tangent = (float3) mul((float3x3) modelData.model, vInput.tangent);

    vOutput.bitangent = (float3) mul((float3x3) modelData.model, vInput.bitangent);

    vOutput.worldPos = mul(modelData.model, float4(vInput.pos, 1.0f));

    vOutput.pos = mul(cameraData.viewProj, vOutput.worldPos);

	vOutput.materialID = modelData.materialID;

	return vOutput;
}