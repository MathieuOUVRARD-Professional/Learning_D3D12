# D3D12 Renderer plan

## TODO

- Copy texutre by rows and properly handle padding for textures rowPitch < 256
- Light casters
- Multiple lights handling
- Frame buffer
- Shadow Mapping
- Frustum culling
- Multithreading for loading scene and copying to upload buffer
- Loading screen
- Mip maps
- DirectStorage
- Packing
- Sort transparency and use multiple pipeline for blending:  
<https://github.com/KhronosGroup/glTF-Sample-Assets/tree/main/Models/AlphaBlendModeTest>
- Fix view setting cursor at the midle of the main screen and not nearest

## Done

- Create window
- Resize window
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
