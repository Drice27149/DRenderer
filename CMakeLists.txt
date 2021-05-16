# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("DRenderer")

file(GLOB_RECURSE MY_CPP ${CMAKE_SOURCE_DIR}/src/*.cpp)

file(GLOB_RECURSE COMMON_CPP ${CMAKE_SOURCE_DIR}/thirdparty/d3dApp/*.cpp ${CMAKE_SOURCE_DIR}/thirdparty/imgui/*.cpp)

include_directories(${CMAKE_SOURCE_DIR}/include) # include

include_directories(${CMAKE_SOURCE_DIR}/include/D3D12)

include_directories(${CMAKE_SOURCE_DIR}/include/cluster)

include_directories(${CMAKE_SOURCE_DIR}/include/common)

include_directories(${CMAKE_SOURCE_DIR}/thirdparty/include) # include

include_directories(${CMAKE_SOURCE_DIR}/thirdparty/d3dApp) # include

include_directories(${CMAKE_SOURCE_DIR}/thirdparty/imgui) # include

link_directories(${CMAKE_SOURCE_DIR}/thirdparty/lib) # lib

add_executable (DRenderer WIN32 ${COMMON_CPP} ${MY_CPP} ${CMAKE_SOURCE_DIR}/thirdparty/glad.c)

target_link_libraries(DRenderer assimp-vc142-mt)

target_link_libraries(DRenderer glfw3)

target_link_libraries(DRenderer
    d3d12.lib
    dxgi.lib
    dxguid.lib
    d3dcompiler.lib
    D3D12.lib
    dxgi.lib
)

install(TARGETS DRenderer RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/out)
