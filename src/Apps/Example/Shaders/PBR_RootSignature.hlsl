/*
*   Root Signature Layout (CPU)
*   - 0:        Matrices "matrices viewProj + model"
*   - 1:        float3 "cameraPos"
*   - 2         Light "lightData"
*   - 3         float "materialIndex"
*   - 4:        descriptor table
*
*   Root Signature Layout (GPU)
*   - b0        Matrices "matrices viewProj + model"
*   - b1        float3 "cameraPos"
*   - b2        Light "lightData"
*   - b3        Light "lightData"
*   - b4        float "materialIndex"
*   - t0...     Texture2D<float4> "Textures"
*   - s0        Sampler for "Textures"
*/

#define PBR_SIG \
"RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)," \
"RootConstants(num32BitConstants = 32, b0),"\
"RootConstants(num32BitConstants = 4, b1),"\
"RootConstants(num32BitConstants = 16, b2),"\
"CBV(b3),"\
"DescriptorTable(" \
    "SRV(t0, numDescriptors = 1024, flags=DATA_VOLATILE | DESCRIPTORS_VOLATILE)"\
"),"\
"StaticSampler(s0)"
