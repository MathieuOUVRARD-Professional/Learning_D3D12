/*
*   Root Signature Layout (CPU)
*   - 0:        Matrices "matrices viewProj + model"
*   - 1:        float3 "cameraPos"
*   - 2:        MaterialData "materialData"
*   - 3:        descriptor table "Textures"
*
*   Root Signature Layout (GPU)
*   - b0        Matrices "matrices viewProj + model"
*   - b1        float3 "cameraPos"
*   - b2        MaterialData "materialData"
*   - t0...     Texture2D<float4> "Textures"
*   - s0        Sampler for "Textures"
*/

#define PBR_SIG \
"RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)," \
"RootConstants(num32BitConstants = 32, b0),"\
"RootConstants(num32BitConstants = 4, b1),"\
"RootConstants(num32BitConstants = 12, b2),"\
"DescriptorTable(" \
    "SRV(t0, numDescriptors = 2, flags=DATA_VOLATILE)"\
"),"\
"StaticSampler(s0)"
