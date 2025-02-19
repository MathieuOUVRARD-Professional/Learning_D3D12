cmake_minimum_required(VERSION 3.20)
 
set(CMAKE_TOOLCHAIN_FILE "vcpkg\\scripts\\buildsystems\\vcpkg.cmake")
 
project(
  CMakeProject
  VERSION 1.0
  DESCRIPTION "D3D12"
  LANGUAGES CXX)
 
# Only do these if this is the main project, and not if it is included through add_subdirectory
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
 
  # Set C++ level
  set(CMAKE_CXX_STANDARD 20)
 
  # Let's ensure -std=c++xx instead of -std=g++xx
  set(CMAKE_CXX_EXTENSIONS OFF)
 
  # Let's nicely support folders in IDEs
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
 
  # Docs only available if this is the main app
  # find_package(Doxygen)
  # if(Doxygen_FOUND)
  #   add_subdirectory(docs)
  # else()
  #   message(STATUS "Doxygen not found, not building docs")
  # endif()

  find_package(directx-headers CONFIG REQUIRED)
  find_package(glm CONFIG REQUIRED)
  find_package(spdlog CONFIG REQUIRED)
  find_package(fmt CONFIG REQUIRED)  
  find_package(assimp CONFIG REQUIRED)
  find_package(Freetype CONFIG REQUIRED)

  add_subdirectory(src/Runtime/D3D12)   #D3D12 Custom Lib
  add_subdirectory(src/Runtime/ImGui)   #ImGui Lib
  add_subdirectory(src/Apps/Example)    #App and related files
  
  target_link_libraries(App PRIVATE "dxguid.lib" Microsoft::DirectX-Headers)
  target_link_libraries(App PRIVATE glm::glm)
  target_link_libraries(App PRIVATE spdlog::spdlog)
  target_link_libraries(App PRIVATE fmt::fmt)
  target_link_libraries(App PRIVATE assimp::assimp)
endif()
 
 
 