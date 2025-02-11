file(GLOB SHADERS_FILES CONFIGURE_DEPENDS "Shaders/*.hlsl")
file(GLOB SHADERS_INTERFACE_FILES CONFIGURE_DEPENDS "Shaders/*.hlsli")
 
foreach(SHADER_FILE ${SHADERS_FILES})
  get_filename_component(SHADER_NAME ${SHADER_FILE} NAME_WE)
 
  set(SHADER_ENTRYPOINT "main")
 
  if(${SHADER_NAME} MATCHES "RootSignature")
    set(SHADER_TYPE "RootSignature")
    set(SHADER_MODEL "rootsig_1_1")
    set(SHADER_ENTRYPOINT "ROOTSIG")
  elseif(${SHADER_NAME} MATCHES "VertexShader")
    set(SHADER_TYPE "Vertex")
    set(SHADER_MODEL "6.5")
  elseif(${SHADER_NAME} MATCHES "PixelShader")
    set(SHADER_TYPE "Pixel")
    set(SHADER_MODEL "6.5")
  endif()
 
  set_source_files_properties(${SHADER_FILE} PROPERTIES
      VS_SHADER_TYPE ${SHADER_TYPE}
      VS_SHADER_ENTRYPOINT ${SHADER_ENTRYPOINT}
      VS_SHADER_MODEL ${SHADER_MODEL}
  )
endforeach()

add_executable(App main.cpp ${SHADERS_FILES} ${SHADERS_INTERFACE_FILES})

set_target_properties(App PROPERTIES
  VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/app/"
)

target_compile_features(App PRIVATE cxx_std_20)
 
target_link_libraries(App PRIVATE D3D12 ImGui)

add_definitions(-DUNICODE -D_UNICODE)

source_group("Shaders" FILES ${SHADERS_FILES})
source_group("Shaders\\Interfaces" FILES ${SHADERS_INTERFACE_FILES})