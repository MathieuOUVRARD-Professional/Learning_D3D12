/*
*   Root Signature Layout (CPU)
*   - 0:        float3 "Light Color"
*   - 1:        matrix "cameraMatrix
*   - 2:        matrix "modelMatrix"
*
*   Root Signature Layout (GPU)
*   - b0:       float3 "Light Color"
*   - b1:       matrix "cameraMatrix
*   - b2:       matrix modelMatrix
*/


#define LIGHTSIG \
"RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)," \
"RootConstants(num32BitConstants = 4, b0),"\
"RootConstants(num32BitConstants = 32, b1),"
