/*
*   Root Signature Layout (CPU)
*   - 0:        matrix "cameraMatrix
*   - 1:        glm::vec3 "lightColor"
*
*   Root Signature Layout (GPU)
*   - b0:       matrix "cameraMatrix
*   - b2:       float3 lightColor
*/


#define LIGHTSIG \
"RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)," \
"RootConstants(num32BitConstants = 32, b0),"\
"RootConstants(num32BitConstants = 4, b1),"
