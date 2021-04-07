# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("DRenderer")

file(GLOB_RECURSE MY_CPP ${CMAKE_SOURCE_DIR}/src/*.cpp)

include_directories(${CMAKE_SOURCE_DIR}/include) # include

include_directories(${CMAKE_SOURCE_DIR}/thirdparty/include) # include

link_directories(${CMAKE_SOURCE_DIR}/thirdparty/lib) # lib

add_executable (DRenderer ${MY_CPP} ${CMAKE_SOURCE_DIR}/thirdparty/glad.c)

target_link_libraries(DRenderer assimp-vc142-mt)

target_link_libraries(DRenderer glfw3)

install(TARGETS DRenderer RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/out)
