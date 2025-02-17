/*
*   Root Signature Layout (CPU)
*   - 0:        float3 "Color"
*   - 1:        matrix "cameraMatrix
*   - 2:        matrix "modelMatrix"
*   - 3:        descriptor table "Textures"
*
*   Root Signature Layout (GPU)
*   - b0:       float3 "Color"
*   - b1:       matrix "cameraMatrix"
*   - b2:       matrix "modelMatrix"
*   - t0...     Texture2D<float4> "Textures"
*   - s0        Sampler for "Textures"
*/


#define ROOTSIG \
"RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)," \
"RootConstants(num32BitConstants = 4, b0),"\
"RootConstants(num32BitConstants = 32, b1),"\
"DescriptorTable(" \
    "SRV(t0, numDescriptors = 1, flags=DATA_VOLATILE)"\
"),"\
"StaticSampler(s0)"
