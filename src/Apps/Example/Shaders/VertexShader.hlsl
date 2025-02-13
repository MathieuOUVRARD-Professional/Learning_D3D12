#include "RootSignature.hlsl"
#include "Pipeline.hlsli"

Correction correction : register(b1);
CameraMatrices cameraMatrices : register(b2);

[RootSignature(ROOTSIG)]
void main(
// === IN === //
	in float3 pos : Position,
	in float2 uv : Texcoord,
// === OUT === //
	out float2 o_uv : Texcoord,
	out float4 o_pos : SV_Position
)
{
	// Rules of transformation. Model -> View -> Projection
    float3 px;							// Model
    
	//px.x = (pos.x * correction.cosAngle) - (pos.y * correction.sinAngle); 
    //px.y = (pos.x * correction.sinAngle) + (pos.y * correction.cosAngle);
	
    px = pos * correction.zoom;		// View
				
    px *= correction.aspectRatio;		// Projection
	
	o_pos = mul(mul(cameraMatrices.projection, cameraMatrices.view), float4(px, 1.0f));
	o_uv = uv;
}