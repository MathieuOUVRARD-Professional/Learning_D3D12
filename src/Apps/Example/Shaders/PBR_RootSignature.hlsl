/*
*   Root Signature Layout (CPU)
*   - 0:        CameraData "cameraPos, viewProjMatrix"
*   - 1:        uint "modelData_ID"
*   - 2         Light "lightData"
*   - 3         ModelData[] "modelslData"
*   - 4         MaterialData[] "materialData"
*   - 5:        descriptor table
*
*   Root Signature Layout (GPU)
*   - b0        CameraData "cameraPos, viewProjMatrix"
*   - b1        uint "modelData_ID"
*   - b2        Light "lightData"
*   - b3        ModelData[] "modelslData"
*   - b4        MaterialData[] "materialData"
*   - t0...     Texture2D<float4> "Textures"
*   - s0        Sampler for "Textures"
*/

#define PBR_SIG \
"RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)," \
"RootConstants(num32BitConstants = 20, b0),"\
"RootConstants(num32BitConstants = 4, b1),"\
"RootConstants(num32BitConstants = 16, b2),"\
"CBV(b3),"\
"CBV(b4),"\
"DescriptorTable(" \
    "SRV(t0, numDescriptors = 1024, flags = DATA_VOLATILE | DESCRIPTORS_VOLATILE)"\
"),"\
"StaticSampler(s0)"
