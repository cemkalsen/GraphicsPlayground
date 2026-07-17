cmake_minimum_required(VERSION 3.20)
project(GraphicsPlayground)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# GLFW
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glfw)

add_executable(Renderer
    src/main.cpp
    external/glad/src/glad.c
 "src/shader.h")

target_include_directories(Renderer PRIVATE external/glad/include)
target_link_libraries(Renderer PRIVATE glfw opengl32)

add_custom_command(TARGET Renderer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/src
    $<TARGET_FILE_DIR:Renderer>/src)