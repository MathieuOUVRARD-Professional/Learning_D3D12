#include "PBR_RootSignature.hlsl"
#include "Pipeline.hlsli"

cbuffer MatricesConstants : register(b0)
{
	Matrices matrices;
};


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
	PBR_V_Out vOutput;

	vOutput.uv = vInput.uv;

    vOutput.normal = (float3) mul((float3x3) matrices.model, vInput.normal);

    vOutput.tangent = (float3) mul((float3x3) matrices.model, vInput.tangent);

    vOutput.bitangent = (float3) mul((float3x3) matrices.model, vInput.bitangent);

	vOutput.currentPos = mul(matrices.model, float4(vInput.pos, 1.0f));

	vOutput.pos = mul(matrices.viewProj, vOutput.currentPos);

	vOutput.materialID = 5;

	return vOutput;
}