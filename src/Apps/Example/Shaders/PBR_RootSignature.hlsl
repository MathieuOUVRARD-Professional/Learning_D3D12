/*
*   Root Signature Layout (CPU)
*   - 0:        CameraData "cameraPos, viewProjMatrix"
*   - 1:        uint "modelData_ID"
*   - 2         SceneData "sceneData
*   - 3         LightData[] "lightsData"
*   - 4         ModelData[] "modelslData"
*   - 5         MaterialData[] "materialData"
*   - 6:        descriptor table
*
*   Root Signature Layout (GPU)
*   - b0        CameraData "cameraPos, viewProjMatrix"
*   - b1        uint "modelData_ID"
*   - b2        SceneData "sceneData
*   - b3        LightData[] "lightsData"
*   - b4        ModelData[] "modelslData"
*   - b5        MaterialData[] "materialData"
*   - t0...     Texture2D<float4> "Textures"
*   - s0        Sampler for "Textures"
*/

#define PBR_SIG \
"RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)," \
"RootConstants(num32BitConstants = 20, b0),"\
"RootConstants(num32BitConstants = 4, b1),"\
"RootConstants(num32BitConstants = 4, b2),"\
"CBV(b3),"\
"CBV(b4),"\
"CBV(b5),"\
"DescriptorTable(" \
    "SRV(t0, numDescriptors = 1024, flags = DATA_VOLATILE | DESCRIPTORS_VOLATILE)"\
"),"\
"StaticSampler(s0)"
