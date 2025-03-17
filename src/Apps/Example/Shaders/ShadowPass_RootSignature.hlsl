/*
*   ShadowPass  Signature Layout (CPU)
*   - 0:        Matrices "matrices viewProj + model"
*
*   ShadowPass  Signature Layout (GPU)
*   - b0        Matrices "matrices viewProj + model"
*/

#define SHADOWPASS_SIG \
"RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)," \
"RootConstants(num32BitConstants = 32, b0),"


