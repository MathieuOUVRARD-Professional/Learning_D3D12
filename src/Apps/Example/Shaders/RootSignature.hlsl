/*
*   Root Signature Layout (CPU)
*   - 0:        Matrices "matrices viewProj + model"
*   - 1:        Light "light" color + pos
*   - 2:        float3 "Color"
*   - 3:        float3 "cameraPos"
*   - 4:        descriptor table "Textures"
*
*   Root Signature Layout (GPU)
*   - b0:       Matrices "matrices viewProj + model"
*   - b1:       Light "light"
*   - b2:       float3 "puke"
*   - b3:       float3 "cameraPos"
*   - t0...     Texture2D<float4> "Textures"
*   - s0        Sampler for "Textures"
*/


#define ROOTSIG \
"RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)," \
"RootConstants(num32BitConstants = 32, b0),"\
"RootConstants(num32BitConstants = 16, b1),"\
"RootConstants(num32BitConstants = 4, b2),"\
"RootConstants(num32BitConstants = 4, b3),"\
"DescriptorTable(" \
    "SRV(t0, numDescriptors = 2, flags=DATA_VOLATILE)"\
"),"\
"StaticSampler(s0)"
