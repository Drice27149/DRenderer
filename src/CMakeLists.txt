cmake_minimum_required(VERSION 3.13)
project(main)
set(CMAKE_CXX_STANDARD 17)

FILE(GLOB ALLCPP CONFIGURE_DEPENDS "*.cpp")

add_executable(main ${ALLCPP} glad.c)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)
link_directories(${PROJECT_SOURCE_DIR}/lib)
target_link_libraries(main  ${PROJECT_SOURCE_DIR}/dll/glfw3.dll)