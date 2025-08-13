# D3D12 Renderer plan

## TODO

- Shadow Mapping for point light
- Multiple lights handling
- Sort transparency and use multiple pipeline for blending:  
<https://github.com/KhronosGroup/glTF-Sample-Assets/tree/main/Models/AlphaBlendModeTest>
- Use HDRI sky texture
- Frustum culling
- Multithreading to display loading screen while launching scene
- Loading screen
- DirectStorage
- Packing
- Fix view setting cursor at the midle of the main screen and not nearest
- C++ side root signature
- Add RTV to ShadowMap Framebuffer to linearize it for debuging view
- Check utf16 for set name

## Done

- Create window
- Resize window
- Add debug layers
- Draw triangle
- Adding ImGui
- F key to switch fullscreen mode
- Load a texture
- Draw a textured triangle
- Handle aspect ratio
- Create framework with CMake and vcpkg to handle dependancies
- Created basic logging with spdLog
- Going 3D
- Camera controls
- Draw 2 objects with 2 different shaders
- Implement depth testing
- Add normal to vertex for basic lighting
- Add specular texture to improve lighting
- Implement custom ImGui func: color picker and perf overlay
- Adding assimp to process incoming sponza scene
- Make assimp add scene objects to an object list
- Create Mesh and ObjectList class
- Copy mesh data (vertices, indices) to GPU
- Create buffer views for each mesh of object list
- Name ressources for debug
- Create wireframe pipeline
- Add debug colors
- Draw wireframe Sponza
- Load materials from GLTF with assimp
- Use bindless for the textures of an object list
- Copy textures to GPU (now upload buffer has limited size and is sent multiple times)
- Add tangents and bitangents to vertex
- Create PBR pipeline
- Draw textured Sponza using PBR
- Depth buffer resizing
- Gamma correction
- Add Transform Guizmo
- Transform UI with cursor wrapping
- Frambuffer
- Light casters
- Use CBV to transfer Material Data
- Shadow Mapping for direct light
- Properly display image with Imgui
- Heap Allocator for bindless
- Mip mapping
- Copy texutre rows by rows and properly handle padding for textures rowPitch < 256
- Shadow Mapping for spot
- Refined Framebuffer with inheritance --> DepthBuffer & ColorBuffer
- Use unique_ptr for textures resource to avoid useless copies
- Use unique_ptr for meshes vertex and indices resources to avoid useless copies
- Created class to handle constant buffers
