# Optionally glob, but only for CMake 3.12 or later:
file(GLOB IMGUI_FILES CONFIGURE_DEPENDS "*.*")
 
file(GLOB IMGUI_BACKENDS_CPP_FILES CONFIGURE_DEPENDS "backends/*.cpp")
file(GLOB IMGUI_BACKENDS_H_FILES CONFIGURE_DEPENDS "backends/*.h")
 
file(GLOB IMGUI_DEBUGGERS_FILES CONFIGURE_DEPENDS "debuggers/*")
 
file(GLOB IMGUI_FONTS_FILES CONFIGURE_DEPENDS "fonts/*")
 
file(GLOB IMGUI_FREETYPE_CPP_FILES CONFIGURE_DEPENDS "freetype/*.cpp")
file(GLOB IMGUI_FREETYPE_H_FILES CONFIGURE_DEPENDS "freetype/*.h")
 
add_library(ImGui
  ${IMGUI_FILES}
  ${IMGUI_BACKENDS_CPP_FILES} ${IMGUI_BACKENDS_H_FILES}
  ${IMGUI_DEBUGGERS_FILES}
  ${IMGUI_FONTS_FILES}
  ${IMGUI_FREETYPE_CPP_FILES} ${IMGUI_FREETYPE_H_FILES}
  )
# All users of this library will need at least C++11

target_compile_features(ImGui PUBLIC cxx_std_20)
 
source_group("backends\\Source Files" FILES ${IMGUI_BACKENDS_CPP_FILES})
source_group("backends\\Header Files" FILES ${IMGUI_BACKENDS_H_FILES})
 
source_group("debuggers" FILES ${IMGUI_DEBUGGERS_FILES})
 
source_group("fonts" FILES ${IMGUI_FONTS_FILES})
 
source_group("freetype\\Source Files" FILES ${IMGUI_FREETYPE_CPP_FILES})
source_group("freetype\\Header Files" FILES ${IMGUI_FREETYPE_H_FILES})
 
# We need this directory, and users of our library will need it too
target_include_directories(ImGui PUBLIC ".")
 
target_link_libraries(ImGui PRIVATE Freetype::Freetype)
target_link_libraries(D3D12 PRIVATE ImGui)
 
add_definitions(-DUNICODE -D_UNICODE)