/*
*   Root Signature Layout (CPU)
*   - 0:        float3 "Light Color"
*   - 1:        CameraMatrices "Camera Matrices"
*
*   Root Signature Layout (GPU)
*   - b0:       float3 "Light Color"
*   - b1:       CameraMatrices
*/


#define LIGHTSIG \
"RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)," \
"RootConstants(num32BitConstants=4, b0),"\
"RootConstants(num32BitConstants=32, b1),"
