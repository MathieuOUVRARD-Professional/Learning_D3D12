file(GLOB CUSTOM_IMGUI_CPP_FILES CONFIGURE_DEPENDS "ImGui/*.cpp")
file(GLOB CUSTOM_IMGUI_H_FILES CONFIGURE_DEPENDS "ImGui/*.h")

file(GLOB SHADERS_FILES CONFIGURE_DEPENDS "Shaders/*.hlsl")
file(GLOB SHADERS_INTERFACE_FILES CONFIGURE_DEPENDS "Shaders/*.hlsli")
 
foreach(SHADER_FILE ${SHADERS_FILES})
  get_filename_component(SHADER_NAME ${SHADER_FILE} NAME_WE)
 
  set(SHADER_ENTRYPOINT "main")
 
  if(${SHADER_NAME} MATCHES "RootSignature")     
    string(REPLACE "RootSignature" "" SHADER_ENTRYPOINT "${SHADER_NAME}")    
    string(TOUPPER "${SHADER_ENTRYPOINT}" SHADER_ENTRYPOINT)

    if(NOT SHADER_ENTRYPOINT)         
      set(SHADER_ENTRYPOINT "ROOTSIG")
      message(${SHADER_NAME} " is default root signature entrypoint set to ROOTSIG")
    else()
      string(APPEND SHADER_ENTRYPOINT "SIG")
      message(${SHADER_NAME} " entry point set to " ${SHADER_ENTRYPOINT})
    endif()

    set(SHADER_TYPE "RootSignature")
    set(SHADER_MODEL "rootsig_1.1")    
    set(SHADER_ENTRYPOINT ${SHADER_ENTRYPOINT})
  elseif(${SHADER_NAME} MATCHES "Vertex")
    set(SHADER_TYPE "Vertex")
    set(SHADER_MODEL "6.5")
  elseif(${SHADER_NAME} MATCHES "Pixel")
    set(SHADER_TYPE "Pixel")
    set(SHADER_MODEL "6.5")
  else()
    message(SEND_ERROR "Error shader is unknown ${SHADER_NAME}")
  endif()
 
  set_source_files_properties(${SHADER_FILE} PROPERTIES
      VS_SHADER_TYPE ${SHADER_TYPE}
      VS_SHADER_ENTRYPOINT ${SHADER_ENTRYPOINT}
      VS_SHADER_MODEL ${SHADER_MODEL}
  )
endforeach()

add_executable(App main.cpp ${SHADERS_FILES} ${SHADERS_INTERFACE_FILES} ${CUSTOM_IMGUI_H_FILES} ${CUSTOM_IMGUI_CPP_FILES})

set_target_properties(App PROPERTIES
  VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/app/"
)

source_group("ImGui\\Source Files" FILES ${CUSTOM_IMGUI_CPP_FILES})
source_group("ImGui\\Header Files" FILES ${CUSTOM_IMGUI_H_FILES})

source_group("Shaders" FILES ${SHADERS_FILES})
source_group("Shaders\\Interfaces" FILES ${SHADERS_INTERFACE_FILES})

target_compile_features(App PRIVATE cxx_std_20)

target_include_directories(App PUBLIC ".")

target_link_libraries(App PRIVATE D3D12 ImGui)

target_compile_definitions(App PRIVATE
$<$<CONFIG:DEBUG>:_DEBUG>
)
target_compile_definitions(App PRIVATE
  $<$<CONFIG:DEVELOPMENT>:_DEVELOPMENT>
)
target_compile_definitions(App PRIVATE
  $<$<CONFIG:RELEASE>:_RELEASE>
)

add_definitions(-DUNICODE -D_UNICODE)
add_definitions(-D GLM_ENABLE_EXPERIMENTAL)