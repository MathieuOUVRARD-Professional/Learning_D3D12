# Optionally glob, but only for CMake 3.12 or later:
file(GLOB D3D_CPP_FILES CONFIGURE_DEPENDS "D3D/*.cpp")
file(GLOB D3D_H_FILES CONFIGURE_DEPENDS "D3D/*.h")
 
file(GLOB DEBUG_CPP_FILES CONFIGURE_DEPENDS "Debug/*.cpp")
file(GLOB DEBUG_H_FILES CONFIGURE_DEPENDS "Debug/*.h")
 
file(GLOB SUPPORT_CPP_FILES CONFIGURE_DEPENDS "Support/*.cpp")
file(GLOB SUPPORT_H_FILES CONFIGURE_DEPENDS "Support/*.h")
 
file(GLOB UTIL_CPP_FILES CONFIGURE_DEPENDS "Util/*.cpp")
file(GLOB UTIL_H_FILES CONFIGURE_DEPENDS "Util/*.h")
 
add_library(D3D12
  ${D3D_CPP_FILES} ${D3D_H_FILES}
  ${DEBUG_CPP_FILES} ${DEBUG_H_FILES}
  ${SUPPORT_CPP_FILES} ${SUPPORT_H_FILES}
  ${SHADERS_FILES} ${SHADERS_INTERFACE_FILES}
  ${UTIL_CPP_FILES} ${UTIL_H_FILES}
  )
 
# All users of this library will need at least C++11
target_compile_features(D3D12 PUBLIC cxx_std_20)
 
source_group("D3D\\Source Files" FILES ${D3D_CPP_FILES})
source_group("D3D\\Header Files" FILES ${D3D_H_FILES})
 
source_group("Debug\\Source Files" FILES ${DEBUG_CPP_FILES})
source_group("Debug\\Header Files" FILES ${DEBUG_H_FILES})
 
source_group("Support\\Source Files" FILES ${SUPPORT_CPP_FILES})
source_group("Support\\Header Files" FILES ${SUPPORT_H_FILES})
 
source_group("Util\\Source Files" FILES ${UTIL_CPP_FILES})
source_group("Util\\Header Files" FILES ${UTIL_H_FILES}) 

# We need this directory, and users of our library will need it too
target_include_directories(D3D12 PUBLIC ".")
 
target_link_libraries(D3D12 PRIVATE "d3d12.lib" "dxgi.lib" "dxguid.lib" "d3dcompiler.lib" Microsoft::DirectX-Headers fmt::fmt spdlog::spdlog glm::glm)

target_compile_definitions(D3D12 PRIVATE
$<$<CONFIG:DEBUG>:_DEBUG>
)
target_compile_definitions(D3D12 PRIVATE
  $<$<CONFIG:DEVELOPMENT>:_DEVELOPMENT>
)
target_compile_definitions(D3D12 PRIVATE
  $<$<CONFIG:RELEASE>:_RELEASE>
)
 
add_definitions(-DUNICODE -D_UNICODE)
add_definitions(-D GLM_ENABLE_EXPERIMENTAL)